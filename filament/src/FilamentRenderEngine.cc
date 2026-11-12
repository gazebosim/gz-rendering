/*
 * Copyright (C) 2026 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <algorithm>
#include <charconv>
#include <cstring>
#include <fstream>
#include <iterator>
#include <memory>
#include <set>
#include <thread>
#include <vector>

#include <EGL/egl.h>
#include <backend/PixelBufferDescriptor.h>
#include <filamat/MaterialBuilder.h>
#include <filamat/Package.h>
#include <filament-matp/Config.h>
#include <filament-matp/MaterialParser.h>
#include <filament/Engine.h>
#include <filament/Material.h>
#include <filament/Renderer.h>
#include <filament/SwapChain.h>
#include <filament/Texture.h>

#include <gz/common/Console.hh>
#include <gz/common/Filesystem.hh>
#include <gz/common/Image.hh>
#include <gz/common/StringUtils.hh>
#include <gz/common/Util.hh>
#include <gz/plugin/Register.hh>

#include "gz/rendering/GraphicsAPI.hh"
#include "gz/rendering/InstallationDirectories.hh"
#include "gz/rendering/filament/FilamentRenderEngine.hh"
#include "gz/rendering/filament/FilamentScene.hh"
#include "FilamentUtils.hh"

using namespace gz;
using namespace rendering;

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    class FilamentRenderEnginePrivate
    {
      public: filament::Engine *engine{nullptr};
      public: filament::Renderer *renderer{nullptr};
      /// \brief 1x1 headless swap chain for BeginFrame/EndFrame, lazy
      public: filament::SwapChain *headless{nullptr};
      public: bool inFrame{false};
      public: FilamentSceneStorePtr scenes;
      public: std::map<std::string, filament::Material *> materials;
      public: std::map<std::string, filament::Texture *> textures;
      /// \brief Textures with a texel alpha below 1, see TextureHasAlpha
      public: std::set<const filament::Texture *> alphaTextures;
      /// \brief OPENGL or VULKAN
      public: rendering::GraphicsAPI backend{rendering::GraphicsAPI::OPENGL};
      public: std::string resourcePath;
      /// \brief Caller's EGL display / context (load params "egl_display",
      /// "egl_context"): filament's GL context then shares their objects
      public: EGLDisplay eglDisplay{EGL_NO_DISPLAY};
      public: EGLContext eglContext{EGL_NO_CONTEXT};
      /// \brief Load param "useCurrentGLContext"
      public: bool callerGLContext{false};
      public: std::unique_ptr<filament::backend::Platform> platform;
      /// \brief Thread in filament's job system, see AdoptThread()
      public: std::thread::id jobThread;
    };
    }
  }
}

//////////////////////////////////////////////////
FilamentRenderEngine::~FilamentRenderEngine()
{
}

//////////////////////////////////////////////////
std::string FilamentRenderEnginePlugin::Name() const
{
  return FilamentRenderEngine::Instance()->Name();
}

//////////////////////////////////////////////////
RenderEngine *FilamentRenderEnginePlugin::Engine() const
{
  return FilamentRenderEngine::Instance();
}

//////////////////////////////////////////////////
FilamentRenderEngine::FilamentRenderEngine()
  : dataPtr(new FilamentRenderEnginePrivate)
{
}

//////////////////////////////////////////////////
void FilamentRenderEngine::Destroy()
{
  BaseRenderEngine::Destroy();

  if (this->dataPtr->scenes)
  {
    this->dataPtr->scenes->RemoveAll();
    this->dataPtr->scenes.reset();
  }

  if (this->dataPtr->engine)
  {
    for (auto &it : this->dataPtr->textures)
      this->dataPtr->engine->destroy(it.second);
    this->dataPtr->textures.clear();
    this->dataPtr->alphaTextures.clear();
    for (auto &it : this->dataPtr->materials)
      this->dataPtr->engine->destroy(it.second);
    this->dataPtr->materials.clear();
    if (this->dataPtr->headless)
      this->dataPtr->engine->destroy(this->dataPtr->headless);
    this->dataPtr->headless = nullptr;
    if (this->dataPtr->renderer)
      this->dataPtr->engine->destroy(this->dataPtr->renderer);
    this->dataPtr->renderer = nullptr;
    // filament shuts down from a thread in its job system
    if (this->dataPtr->jobThread == std::thread::id() ||
        this->dataPtr->jobThread == std::this_thread::get_id())
    {
      this->AdoptThread();
      filament::Engine::destroy(&this->dataPtr->engine);
    }
    else
    {
      gzwarn << "Filament engine not destroyed: it renders on another thread"
             << std::endl;
      // leaked with the engine, which still uses it
      this->dataPtr->platform.release();
    }
    this->dataPtr->engine = nullptr;
    // a new engine (Load/Init again) is adopted afresh
    this->dataPtr->jobThread = std::thread::id();
  }
  this->dataPtr->platform.reset();
}

//////////////////////////////////////////////////
bool FilamentRenderEngine::IsEnabled() const
{
  return this->initialized;
}

//////////////////////////////////////////////////
std::string FilamentRenderEngine::Name() const
{
  return "filament";
}

//////////////////////////////////////////////////
rendering::GraphicsAPI FilamentRenderEngine::GraphicsAPI() const
{
  return this->dataPtr->backend;
}

//////////////////////////////////////////////////
filament::Engine *FilamentRenderEngine::FilamentEngine() const
{
  return this->dataPtr->engine;
}

//////////////////////////////////////////////////
filament::Renderer *FilamentRenderEngine::Renderer() const
{
  return this->dataPtr->renderer;
}

//////////////////////////////////////////////////
void FilamentRenderEngine::BeginFrame()
{
  if (this->dataPtr->inFrame)
    return;
  this->AdoptThread();
  if (!this->dataPtr->headless)
  {
    this->dataPtr->headless =
        this->dataPtr->engine->createSwapChain(1, 1, 0);
  }
  // the frame skipper may ask to skip; rendering anyway is allowed and the
  // first render() then does the deferred frame setup
  this->dataPtr->renderer->beginFrame(this->dataPtr->headless);
  this->dataPtr->inFrame = true;
}

//////////////////////////////////////////////////
void FilamentRenderEngine::EndFrame()
{
  if (!this->dataPtr->inFrame)
    return;
  this->dataPtr->renderer->endFrame();
  this->dataPtr->inFrame = false;
}

//////////////////////////////////////////////////
namespace
{
// Filament has no public API to hand its job system's slot for outside
// threads from one thread to another (Engine::create adopts the creating
// thread, every thread that renders must be adopted), and it does not
// install utils/JobSystem.h, which declares utils::JobSystem::adopt() and
// emancipate(). Call them by their Itanium C++ ABI names (GCC and Clang;
// `this` is the first argument), exported by libutils.a.
// TODO(anyone): call engine.getJobSystem().adopt() / emancipate() once filament
// installs the header.
extern "C" void _ZN5utils9JobSystem5adoptEv(::utils::JobSystem *_jobSystem);
extern "C" void _ZN5utils9JobSystem10emancipateEv(
    ::utils::JobSystem *_jobSystem);

/// \brief utils::JobSystem::adopt(): the calling thread takes _engine's
/// job system slot for outside threads
void AdoptJobSystemThread(filament::Engine &_engine)
{
  _ZN5utils9JobSystem5adoptEv(&_engine.getJobSystem());
}

/// \brief utils::JobSystem::emancipate(): the calling thread frees that
/// slot
void EmancipateJobSystemThread(filament::Engine &_engine)
{
  _ZN5utils9JobSystem10emancipateEv(&_engine.getJobSystem());
}
}

//////////////////////////////////////////////////
void FilamentRenderEngine::AdoptThread()
{
  const std::thread::id self = std::this_thread::get_id();
  if (this->dataPtr->jobThread == self)
    return;
  if (this->dataPtr->jobThread != std::thread::id())
  {
    gzerr << "Filament renders from one thread at a time: its job system "
          << "has a single slot for threads it did not create" << std::endl;
  }
  AdoptJobSystemThread(*this->dataPtr->engine);
  this->dataPtr->jobThread = self;
}

//////////////////////////////////////////////////
bool FilamentRenderEngine::InFrame() const
{
  return this->dataPtr->inFrame;
}

namespace
{
  /// \brief The whole file, empty if it can not be read
  std::vector<uint8_t> ReadFile(const std::string &_path)
  {
    std::ifstream in(_path, std::ios::binary);
    return std::vector<uint8_t>(std::istreambuf_iterator<char>(in),
        std::istreambuf_iterator<char>());
  }

  /// \brief Whether _dds starts with a DDS magic and header
  bool IsDds(const std::vector<uint8_t> &_dds)
  {
    return _dds.size() >= 128u &&
        std::equal(_dds.begin(), _dds.begin() + 4, "DDS ");
  }

  /// \brief Little endian 32 bit field at byte _offset of a DDS file
  uint32_t DdsU32(const std::vector<uint8_t> &_dds, size_t _offset)
  {
    return static_cast<uint32_t>(_dds[_offset] | (_dds[_offset + 1] << 8u) |
        (_dds[_offset + 2] << 16u) | (_dds[_offset + 3] << 24u));
  }
}

//////////////////////////////////////////////////
filament::Material *FilamentRenderEngine::Material(const std::string &_name)
{
  auto it = this->dataPtr->materials.find(_name);
  if (it != this->dataPtr->materials.end())
    return it->second;

  // A path to a filament material source (Material::SetFragmentShader with
  // a .mat file): compiled at runtime, cached under its path. Failures are
  // cached too (null), so that they are reported once.
  if (common::EndsWith(_name, ".mat"))
  {
    filament::Material *mat = this->CompileMaterial(_name);
    this->dataPtr->materials[_name] = mat;
    return mat;
  }

  std::string path = common::joinPaths(this->dataPtr->resourcePath,
      "materials", _name + ".filamat");
  const std::vector<uint8_t> data = ReadFile(path);
  filament::Material *mat = nullptr;
  if (data.empty())
  {
    gzerr << "Unable to open filament material package [" << path << "]"
          << std::endl;
  }
  else
  {
    mat = filament::Material::Builder()
        .package(data.data(), data.size())
        .build(*this->dataPtr->engine);
    if (!mat)
    {
      gzerr << "Failed to build filament material [" << _name << "]"
            << std::endl;
    }
  }
  this->dataPtr->materials[_name] = mat;
  return mat;
}

//////////////////////////////////////////////////
namespace
{
/// \brief matc's options for the running backend, all platforms (the GL
/// backend may run on an OpenGL ES context)
class RuntimeMatConfig : public matp::Config
{
  public: explicit RuntimeMatConfig(filament::backend::Backend _backend)
  {
    this->mPlatform = Platform::ALL;
    // like the build-time matc flags (--optimize-size); the performance
    // level's GLSL can trip Mesa's compiler (struct constructor members)
    this->mOptimizationLevel = Optimization::SIZE;
    this->mTargetApi = _backend == filament::backend::Backend::VULKAN ?
        TargetApi::VULKAN : TargetApi::OPENGL;
  }
  public: Output *getOutput() const noexcept override { return nullptr; }
  public: Input *getInput() const noexcept override { return nullptr; }
  public: std::string toString() const noexcept override { return ""; }
  public: std::string toPIISafeString() const noexcept override
  { return ""; }
};
}

//////////////////////////////////////////////////
filament::Material *FilamentRenderEngine::CompileMaterial(
    const std::string &_path)
{
  std::ifstream in(_path, std::ios::binary | std::ios::ate);
  if (!in)
  {
    gzerr << "Unable to open filament material source [" << _path << "]"
          << std::endl;
    return nullptr;
  }
  // the parser takes ownership of the text
  ssize_t size = static_cast<ssize_t>(in.tellg());
  std::unique_ptr<char[]> chars(new char[size]);
  in.seekg(0);
  in.read(chars.get(), size);
  std::unique_ptr<const char[]> buffer(chars.release());

  // TODO(anyone): compiles on the render thread (~1 s per material, for the
  // running backend only), cached per path for the engine's lifetime; an
  // on-disk .filamat cache if startup time matters. No #include support.
  auto *engine = this->dataPtr->engine;
  this->AdoptThread();
  RuntimeMatConfig config(engine->getBackend());
  filamat::MaterialBuilder::init();
  filament::Material *mat = nullptr;
  {
    filamat::MaterialBuilder builder;
    matp::MaterialParser parser;
    ::utils::Status status = parser.parse(builder, config, size, buffer);
    if (!status.isOk())
    {
      gzerr << "Failed to parse filament material [" << _path << "]: "
            << status.getMessage() << std::endl;
    }
    else
    {
      // the engine's job system, adopted above
      filamat::Package package = builder.build(engine->getJobSystem());
      if (!package.isValid())
      {
        gzerr << "Failed to compile filament material [" << _path << "]"
              << std::endl;
      }
      else
      {
        mat = filament::Material::Builder()
            .package(package.getData(), package.getSize())
            .build(*engine);
      }
    }
  }
  filamat::MaterialBuilder::shutdown();
  if (mat)
    gzmsg << "Compiled filament material [" << _path << "]" << std::endl;
  return mat;
}

//////////////////////////////////////////////////
/// \brief Top mip of an uncompressed 24/32-bit RGB(A) DDS (like waves'
/// wave_normals.dds), which common::Image can not read. False otherwise.
static bool LoadUncompressedDds(const std::string &_path, common::Image &_img)
{
  if (!common::EndsWith(common::lowercase(_path), ".dds"))
    return false;
  const std::vector<uint8_t> dds = ReadFile(_path);
  auto u32 = [&](size_t _o) { return DdsU32(dds, _o); };
  if (!IsDds(dds) || !(u32(80) & 0x40u) || u32(112) != 0u)
    return false;
  const uint32_t h = u32(12), w = u32(16), bits = u32(88);
  const uint32_t bpp = bits / 8u;
  if ((bpp != 3u && bpp != 4u) || dds.size() < 128u + size_t(w) * h * bpp)
    return false;
  // channel byte offsets from the masks (little endian)
  auto shift = [&](uint32_t _mask)
  {
    uint32_t b = 0;
    while (_mask && !(_mask & 0xffu))
    {
      _mask >>= 8u;
      ++b;
    }
    return b;
  };
  const uint32_t r = shift(u32(92)), g = shift(u32(96)), b = shift(u32(100));
  std::vector<unsigned char> rgb(size_t(w) * h * 3u);
  for (size_t i = 0; i < size_t(w) * h; ++i)
  {
    const uint8_t *px = dds.data() + 128u + i * bpp;
    rgb[i * 3] = px[r];
    rgb[i * 3 + 1] = px[g];
    rgb[i * 3 + 2] = px[b];
  }
  _img.SetFromData(rgb.data(), w, h, common::Image::RGB_INT8);
  return true;
}

//////////////////////////////////////////////////
filament::Texture *FilamentRenderEngine::Texture(const std::string &_path,
    std::shared_ptr<const common::Image> _data, bool _srgb)
{
  const std::string key = _srgb ? _path : _path + "\n(linear)";
  auto it = this->dataPtr->textures.find(key);
  if (it != this->dataPtr->textures.end())
    return it->second;

  // Empty path: 1x1 white texture, bound when a material has no texture so
  // every declared sampler is set.
  common::Image img;
  if (_path.empty())
  {
    const unsigned char white[4] = {255, 255, 255, 255};
    img.SetFromData(white, 1, 1, common::Image::RGBA_INT8);
  }
  else if (!_data && !LoadUncompressedDds(_path, img))
  {
    img.Load(_path);
  }
  // common::Image is not safely copyable: read in-memory data in place
  const common::Image &src = _data ? *_data : img;
  if (!src.Valid())
  {
    gzerr << "Unable to load texture image [" << _path << "]" << std::endl;
    return nullptr;
  }

  const unsigned int w = src.Width();
  const unsigned int h = src.Height();
  std::vector<unsigned char> rgba = src.RGBAData();
  const size_t texels = static_cast<size_t>(w) * h;
  if (rgba.size() == texels * 8u)
  {
    // 16 bits per channel (native endian, e.g. 16-bit PNG roughness maps):
    // keep the high byte, the textures are 8-bit. ogre2's FreeImage codec
    // uploads them as RGBA16_UNORM, which has no sRGB variant: it samples
    // them linearly even in sRGB slots (Harmonic World's 16-bit tree
    // billboards and wall albedo), so do the same.
    _srgb = false;
    std::vector<unsigned char> rgba8(texels * 4u);
    for (size_t i = 0; i < rgba8.size(); ++i)
    {
      uint16_t v;
      std::memcpy(&v, rgba.data() + i * 2u, sizeof(v));
      rgba8[i] = static_cast<unsigned char>(v >> 8u);
    }
    rgba.swap(rgba8);
  }
  if (rgba.size() != texels * 4u)
  {
    gzerr << "Unexpected texture data size for [" << _path << "]"
          << std::endl;
    return nullptr;
  }

  // filament textures have their origin at the bottom-left; image rows are
  // stored top-down. Flip so UVs can be used as authored.
  auto *buf = new unsigned char[rgba.size()];
  const size_t rowBytes = static_cast<size_t>(w) * 4u;
  for (unsigned int y = 0; y < h; ++y)
  {
    std::memcpy(buf + static_cast<size_t>(y) * rowBytes,
                rgba.data() + static_cast<size_t>(h - 1 - y) * rowBytes,
                rowBytes);
  }

  // full mip chain, like ogre2 (it auto-generates mipmaps for albedo
  // maps): minified textures average instead of aliasing
  auto &engine = *this->dataPtr->engine;
  const auto format = _srgb ? filament::Texture::InternalFormat::SRGB8_A8 :
      filament::Texture::InternalFormat::RGBA8;
  const bool mips = filament::Texture::isTextureFormatMipmappable(engine,
      format);
  uint8_t levels = 1u;
  while (mips && (std::max(w, h) >> levels) > 0u)
    ++levels;
  filament::Texture *tex = filament::Texture::Builder()
      .width(w).height(h).levels(levels)
      .sampler(filament::Texture::Sampler::SAMPLER_2D)
      .format(format)
      .usage(filament::Texture::Usage::DEFAULT |
             (levels > 1u ? filament::Texture::Usage::GEN_MIPMAPPABLE :
                            filament::Texture::Usage::NONE))
      .build(engine);
  filament::Texture::PixelBufferDescriptor desc(buf, rgba.size(),
      filament::Texture::Format::RGBA, filament::Texture::Type::UBYTE,
      &filament_utils::FreeBuffer<unsigned char>);
  tex->setImage(engine, 0, std::move(desc));
  if (levels > 1u)
    tex->generateMipmaps(engine);
  this->dataPtr->textures[key] = tex;
  for (size_t i = 3; i < rgba.size(); i += 4u)
  {
    if (rgba[i] < 255u)
    {
      this->dataPtr->alphaTextures.insert(tex);
      break;
    }
  }
  return tex;
}

//////////////////////////////////////////////////
bool FilamentRenderEngine::TextureHasAlpha(
    const filament::Texture *_texture) const
{
  return this->dataPtr->alphaTextures.count(_texture) > 0u;
}

namespace
{
  /// \brief Decode a BC1 (DXT1) or BC3 (DXT5) surface to RGBA8
  void DecodeBc(const uint8_t *_src, unsigned int _w, unsigned int _h,
      bool _bc3, uint8_t *_dst)
  {
    const unsigned int bw = (_w + 3u) / 4u;
    const unsigned int bh = (_h + 3u) / 4u;
    for (unsigned int by = 0; by < bh; ++by)
    {
      for (unsigned int bx = 0; bx < bw; ++bx)
      {
        const uint8_t *b = _src + (by * bw + bx) * (_bc3 ? 16u : 8u);
        uint8_t alpha[16];
        std::fill(alpha, alpha + 16, 255);
        if (_bc3)
        {
          int a[8] = {b[0], b[1]};
          for (int i = 2; i < 8; ++i)
          {
            a[i] = a[0] > a[1] ? ((8 - i) * a[0] + (i - 1) * a[1]) / 7 :
                i < 6 ? ((6 - i) * a[0] + (i - 1) * a[1]) / 5 :
                i == 6 ? 0 : 255;
          }
          uint64_t bits = 0;
          for (int k = 0; k < 6; ++k)
            bits |= static_cast<uint64_t>(b[2 + k]) << (8 * k);
          for (int p = 0; p < 16; ++p)
            alpha[p] = static_cast<uint8_t>(a[(bits >> (3 * p)) & 7u]);
          b += 8;
        }
        const unsigned int c[2] = {
            static_cast<unsigned int>(b[0] | (b[1] << 8u)),
            static_cast<unsigned int>(b[2] | (b[3] << 8u))};
        int pal[4][4];
        for (int i = 0; i < 2; ++i)
        {
          pal[i][0] = ((c[i] >> 11) & 31) * 255 / 31;
          pal[i][1] = ((c[i] >> 5) & 63) * 255 / 63;
          pal[i][2] = (c[i] & 31) * 255 / 31;
          pal[i][3] = 255;
        }
        const bool four = _bc3 || c[0] > c[1];
        for (int k = 0; k < 3; ++k)
        {
          pal[2][k] = four ? (2 * pal[0][k] + pal[1][k]) / 3 :
              (pal[0][k] + pal[1][k]) / 2;
          pal[3][k] = four ? (pal[0][k] + 2 * pal[1][k]) / 3 : 0;
        }
        pal[2][3] = 255;
        pal[3][3] = four ? 255 : 0;
        const uint32_t idx = b[4] | (b[5] << 8u) | (b[6] << 16u) |
            (static_cast<uint32_t>(b[7]) << 24u);
        for (unsigned int p = 0; p < 16; ++p)
        {
          const unsigned int x = bx * 4u + p % 4u;
          const unsigned int y = by * 4u + p / 4u;
          if (x >= _w || y >= _h)
            continue;
          const int *col = pal[(idx >> (2u * p)) & 3u];
          uint8_t *d = _dst + (static_cast<size_t>(y) * _w + x) * 4u;
          d[0] = static_cast<uint8_t>(col[0]);
          d[1] = static_cast<uint8_t>(col[1]);
          d[2] = static_cast<uint8_t>(col[2]);
          d[3] = _bc3 ? alpha[p] : static_cast<uint8_t>(col[3]);
        }
      }
    }
  }
}

//////////////////////////////////////////////////
bool FilamentRenderEngine::CubeFaces(const std::string &_path,
    unsigned int &_size, std::vector<uint8_t> &_faces)
{
  std::string path = _path;
  if (!common::exists(path))
  {
    path = common::joinPaths(this->dataPtr->resourcePath, "textures",
        common::basename(_path));
  }
  const std::vector<uint8_t> dds = ReadFile(path);
  auto u32 = [&](size_t _o) { return DdsU32(dds, _o); };
  const bool isDds = IsDds(dds);
  const std::string fourcc = isDds ?
      std::string(dds.begin() + 84, dds.begin() + 88) : "";
  const bool bc3 = fourcc == "DXT5";
  const uint32_t w = isDds ? u32(16) : 0u;
  const uint32_t mips = isDds ? std::max(1u, u32(28)) : 0u;
  // block-compressed bytes of one face, mip chain included
  size_t faceBytes = 0u;
  for (uint32_t l = 0; l < mips; ++l)
  {
    const size_t s = std::max(1u, ((w >> l) + 3u) / 4u);
    faceBytes += s * s * (bc3 ? 16u : 8u);
  }
  if (!isDds || (fourcc != "DXT1" && !bc3) || !(u32(112) & 0x200u) ||
      u32(12) != w || w == 0u || dds.size() < 128u + 6u * faceBytes)
  {
    gzerr << "Unable to load cube map [" << _path << "]: only DXT1/DXT5 "
          << "DDS cube maps are supported" << std::endl;
    return false;
  }
  _size = w;
  const size_t faceSize = static_cast<size_t>(_size) * _size * 4u;
  std::vector<uint8_t> ogre(6u * faceSize);
  for (int f = 0; f < 6; ++f)
  {
    DecodeBc(dds.data() + 128u + f * faceBytes, _size, _size, bc3,
        ogre.data() + f * faceSize);
  }
  // Ogre samples its (left-handed) cube maps at (x, y, -z). Filament
  // samples at (x, y, z): mirror the faces through the z = 0 plane.
  // +-X faces flip horizontally, +-Y faces vertically, +Z and -Z swap
  // and flip horizontally.
  _faces.resize(6u * faceSize);
  const int src[6] = {0, 1, 2, 3, 5, 4};
  for (int f = 0; f < 6; ++f)
  {
    const bool flipV = f == 2 || f == 3;
    for (unsigned int y = 0; y < _size; ++y)
    {
      for (unsigned int x = 0; x < _size; ++x)
      {
        const unsigned int sx = flipV ? x : _size - 1u - x;
        const unsigned int sy = flipV ? _size - 1u - y : y;
        std::memcpy(_faces.data() + f * faceSize + (y * _size + x) * 4u,
            ogre.data() + src[f] * faceSize + (sy * _size + sx) * 4u, 4u);
      }
    }
  }
  return true;
}

//////////////////////////////////////////////////
filament::Texture *FilamentRenderEngine::CubeTexture(const std::string &_path,
    bool _srgb)
{
  const std::string key = (_srgb ? "\n(cube srgb)" : "\n(cube)") + _path;
  auto it = this->dataPtr->textures.find(key);
  if (it != this->dataPtr->textures.end())
    return it->second;

  unsigned int size = 1u;
  std::vector<uint8_t> faces(6u * 4u, 0u);
  if (!_path.empty() && !this->CubeFaces(_path, size, faces))
    return nullptr;

  auto &engine = *this->dataPtr->engine;
  uint8_t levels = 1u;
  while ((size >> levels) > 0u)
    ++levels;
  filament::Texture *tex = filament::Texture::Builder()
      .width(size).height(size).levels(levels)
      .sampler(filament::Texture::Sampler::SAMPLER_CUBEMAP)
      .format(_srgb ? filament::Texture::InternalFormat::SRGB8_A8 :
          filament::Texture::InternalFormat::RGBA8)
      .usage(filament::Texture::Usage::DEFAULT |
             filament::Texture::Usage::GEN_MIPMAPPABLE)
      .build(engine);
  auto *buf = new uint8_t[faces.size()];
  std::memcpy(buf, faces.data(), faces.size());
  tex->setImage(engine, 0, 0, 0, 0, size, size, 6,
      filament::Texture::PixelBufferDescriptor(buf, faces.size(),
      filament::Texture::Format::RGBA, filament::Texture::Type::UBYTE,
      &filament_utils::FreeBuffer<uint8_t>));
  if (levels > 1u)
    tex->generateMipmaps(engine);
  this->dataPtr->textures[key] = tex;
  return tex;
}

namespace gz::rendering::filament_utils
{
/// \brief Defined in FilamentSharedEglPlatform.cc (built without RTTI)
filament::backend::Platform *CreateSharedEglPlatform(void *_eglDisplay);
}

//////////////////////////////////////////////////
bool FilamentRenderEngine::SharesCallerGLContext() const
{
  return this->dataPtr->platform != nullptr;
}

//////////////////////////////////////////////////
bool FilamentRenderEngine::UsesCallerGLContext() const
{
  return this->dataPtr->callerGLContext;
}

//////////////////////////////////////////////////
namespace
{
/// \brief Pointer passed as a decimal integer in engine parameter _key
/// \param[out] _ptr The pointer, null if the parameter is not set
/// \return False if the value is not a decimal integer
bool PointerParam(const std::map<std::string, std::string> &_params,
    const std::string &_key, void *&_ptr)
{
  _ptr = nullptr;
  auto it = _params.find(_key);
  if (it == _params.end())
    return true;
  const std::string &str = it->second;
  uintptr_t value = 0;
  const auto [end, ec] =
      std::from_chars(str.data(), str.data() + str.size(), value);
  if (ec != std::errc() || end != str.data() + str.size())
  {
    gzerr << "Invalid engine parameter [" << _key << "]: [" << str
          << "], expected a pointer as a decimal integer" << std::endl;
    return false;
  }
  _ptr = reinterpret_cast<void *>(value);
  return true;
}
}

//////////////////////////////////////////////////
bool FilamentRenderEngine::LoadImpl(
    const std::map<std::string, std::string> &_params)
{
  constexpr auto kVulkan = rendering::GraphicsAPI::VULKAN;
  constexpr auto kOpenGL = rendering::GraphicsAPI::OPENGL;
  auto it = _params.find("backend");
  if (it != _params.end())
  {
    this->dataPtr->backend =
        GraphicsAPIUtils::Set(it->second) == kVulkan ? kVulkan : kOpenGL;
  }
  // Compatibility with the ogre2 style switches used by gz-sim / tests
  it = _params.find("vulkan");
  if (it != _params.end() && it->second == "1")
    this->dataPtr->backend = kVulkan;
  it = _params.find("opengl");
  if (it != _params.end() && it->second == "1")
    this->dataPtr->backend = kOpenGL;
  // e.g. gz-gui's MinimalScene, when Qt renders with EGL
  if (!PointerParam(_params, "egl_display", this->dataPtr->eglDisplay) ||
      !PointerParam(_params, "egl_context", this->dataPtr->eglContext))
  {
    return false;
  }
  // gz-gui asks for the GL context current on its render thread (ogre2
  // renders in it): share it when it is an EGL one
  it = _params.find("useCurrentGLContext");
  this->dataPtr->callerGLContext = it != _params.end() && it->second == "1";
  if (this->dataPtr->eglContext == EGL_NO_CONTEXT &&
      this->dataPtr->callerGLContext)
  {
    this->dataPtr->eglDisplay = eglGetCurrentDisplay();
    this->dataPtr->eglContext = eglGetCurrentContext();
  }
  return true;
}

//////////////////////////////////////////////////
namespace
{
/// \brief Directory of the compiled materials and textures:
/// GZ_RENDERING_FILAMENT_RESOURCE_PATH if set, else <resources>/filament/media
/// like ogre2 (<resources>: GZ_RENDERING_RESOURCE_PATH or the install's),
/// else the build tree's, where they are compiled.
std::string MediaPath()
{
  std::string path;
  if (common::env("GZ_RENDERING_FILAMENT_RESOURCE_PATH", path) &&
      !path.empty())
  {
    return path;
  }
  if (!common::env("GZ_RENDERING_RESOURCE_PATH", path) || path.empty())
    path = getResourcePath();
  // install path
  const std::string media = common::joinPaths(path, "filament", "media");
  if (common::exists(media))
    return media;
  // build path
  return FILAMENT_BUILD_MEDIA_PATH;
}

/// \brief Every sensor readback ends in flushAndWait(), i.e. glFinish() on
/// the driver thread, which NVIDIA's GL driver spins on (sched_yield) by
/// default. __GL_YIELD=USLEEP makes it usleep between polls instead: the
/// GPU work takes a few 100 us, the CPU goes to physics meanwhile (6-9%
/// less CPU in gz-sim with RGB or depth sensors on NVIDIA). Only for the
/// OpenGL backend, read when the driver loads (so it also applies to other
/// GL users in the process loaded later); a value the user set wins. Other
/// drivers ignore the variable.
void SetGlYield()
{
  setenv("__GL_YIELD", "USLEEP", 0);
}
}

//////////////////////////////////////////////////
bool FilamentRenderEngine::InitImpl()
{
  this->dataPtr->resourcePath = MediaPath();
  if (!common::isDirectory(
      common::joinPaths(this->dataPtr->resourcePath, "materials")))
  {
    gzerr << "Filament materials not found in ["
          << this->dataPtr->resourcePath << "]. Set "
          << "GZ_RENDERING_RESOURCE_PATH to the gz-rendering resource "
          << "directory." << std::endl;
  }

  // OpenGL by default: sensors are readback bound and filament's Vulkan
  // readPixels allocates a staging buffer per call (~4 ms per readback on
  // NVIDIA, vs ~0.3 ms on OpenGL). Vulkan stays available on request.
  filament::Engine::Backend backend = filament::Engine::Backend::OPENGL;
  if (this->dataPtr->backend == rendering::GraphicsAPI::VULKAN)
    backend = filament::Engine::Backend::VULKAN;

  // The default handle arena (4 MB on GL) fills up with a few thousand
  // visuals (gz-sim's 3k_shapes world): Filament then prints a PanicLog and
  // falls back to slower heap allocations.
  filament::Engine::Config config;
  config.driverHandleArenaSizeMB = 32;
  filament::Engine::Builder builder;
  builder.backend(backend).config(&config);
  if (backend == filament::Engine::Backend::OPENGL &&
      this->dataPtr->eglDisplay != EGL_NO_DISPLAY &&
      this->dataPtr->eglContext != EGL_NO_CONTEXT)
  {
    this->dataPtr->platform.reset(
        filament_utils::CreateSharedEglPlatform(this->dataPtr->eglDisplay));
    builder.platform(this->dataPtr->platform.get())
        .sharedContext(this->dataPtr->eglContext);
  }
  if (backend == filament::Engine::Backend::OPENGL)
    SetGlYield();
  this->dataPtr->engine = builder.build();
  if (!this->dataPtr->engine && this->dataPtr->platform)
  {
    gzwarn << "Filament could not share the caller's EGL context, "
           << "using its own" << std::endl;
    this->dataPtr->platform.reset();
    this->dataPtr->engine = filament::Engine::Builder().backend(backend)
        .config(&config).build();
  }
  if (!this->dataPtr->engine && backend == filament::Engine::Backend::VULKAN)
  {
    // e.g. no Vulkan loader or ICD on this machine
    gzwarn << "Filament Vulkan backend unavailable, falling back to OpenGL"
           << std::endl;
    this->dataPtr->backend = rendering::GraphicsAPI::OPENGL;
    SetGlYield();
    this->dataPtr->engine = filament::Engine::Builder()
        .backend(filament::Engine::Backend::OPENGL)
        .config(&config).build();
  }
  if (!this->dataPtr->engine)
  {
    gzerr << "Failed to create filament engine (backend: "
          << common::lowercase(GraphicsAPIUtils::Str(this->dataPtr->backend))
          << ")" << std::endl;
    return false;
  }
  // The engine adopts its creator into its job system, which has one slot
  // for such threads. Free it: the first thread that renders takes it (gz-gui
  // creates the engine on Qt's scene graph thread, renders on its own).
  EmancipateJobSystemThread(*this->dataPtr->engine);

  this->dataPtr->renderer = this->dataPtr->engine->createRenderer();
  this->dataPtr->scenes = FilamentSceneStorePtr(new FilamentSceneStore);

  gzmsg << "Filament render engine initialized (backend: "
        << common::lowercase(GraphicsAPIUtils::Str(this->dataPtr->backend))
        << (this->dataPtr->platform ? ", shares caller's EGL context" : "")
        << ", resources: "
        << this->dataPtr->resourcePath << ")" << std::endl;
  return true;
}

//////////////////////////////////////////////////
SceneStorePtr FilamentRenderEngine::Scenes() const
{
  return this->dataPtr->scenes;
}

//////////////////////////////////////////////////
ScenePtr FilamentRenderEngine::CreateSceneImpl(unsigned int _id,
    const std::string &_name)
{
  auto scene = FilamentScenePtr(new FilamentScene(_id, _name));
  this->dataPtr->scenes->Add(scene);
  return scene;
}

// Register this plugin
GZ_ADD_PLUGIN(rendering::FilamentRenderEnginePlugin,
              rendering::RenderEnginePlugin)
GZ_ADD_PLUGIN_ALIAS(rendering::FilamentRenderEnginePlugin,
                    "gz::rendering::filament::Plugin")
