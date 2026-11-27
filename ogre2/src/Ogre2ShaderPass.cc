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

#include <cstdint>
#include <cstring>
#include <exception>
#include <fstream>
#include <memory>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>

#include <gz/common/Console.hh>
#include <gz/common/Profiler.hh>

#include "gz/rendering/RenderPassSystem.hh"
#include "gz/rendering/ShaderParams.hh"
#include "gz/rendering/ogre2/Ogre2RenderEngine.hh"
#include "gz/rendering/ogre2/Ogre2ShaderPass.hh"

#ifdef _MSC_VER
  #pragma warning(push, 0)
#endif
#include <Compositor/OgreCompositorManager2.h>
#include <Compositor/OgreCompositorNodeDef.h>
#include <Compositor/Pass/PassQuad/OgreCompositorPassQuadDef.h>
#include <OgreGpuProgramParams.h>
#include <OgreHighLevelGpuProgram.h>
#include <OgreHighLevelGpuProgramManager.h>
#include <OgreHlmsDatablock.h>
#include <OgreHlmsSamplerblock.h>
#include <OgreMaterial.h>
#include <OgreMaterialManager.h>
#include <OgrePass.h>
#include <OgrePixelFormatGpu.h>
#include <OgreRenderSystem.h>
#include <OgreResourceGroupManager.h>
#include <OgreRoot.h>
#include <OgreStagingTexture.h>
#include <OgreTechnique.h>
#include <OgreTextureBox.h>
#include <OgreTextureGpu.h>
#include <OgreTextureGpuManager.h>
#include <OgreTextureUnitState.h>
#ifdef _MSC_VER
  #pragma warning(pop)
#endif

/// \brief Private data for the Ogre2ShaderPass class
class gz::rendering::Ogre2ShaderPassPrivate
{
  /// \brief The pass's material, made from the user's shaders
  public: Ogre::MaterialPtr material;

  /// \brief The shader programs
  public: Ogre::HighLevelGpuProgramPtr vertexProgram;
  public: Ogre::HighLevelGpuProgramPtr fragmentProgram;

  /// \brief The data texture, and the unit that samples it
  public: Ogre::TextureGpu *texture = nullptr;
  public: Ogre::TextureUnitState *unit = nullptr;

  /// \brief The texture's size and format as created
  public: unsigned int textureWidth = 0u;
  public: unsigned int textureHeight = 0u;
  public: PixelFormat textureFormat = PF_UNKNOWN;

  /// \brief Set when the pass gave up: its error is logged once
  public: bool failed = false;

  /// \brief Parameters already warned about
  public: std::set<std::string> warned;

  /// \brief Whether Ogre found a shader of this pass not to compile, when
  /// it first drew it.
  /// \return True if one did not compile.
  public: bool CompileError() const
  {
    return (this->vertexProgram && this->vertexProgram->hasCompileError()) ||
        (this->fragmentProgram && this->fragmentProgram->hasCompileError());
  }
};

using namespace gz;
using namespace rendering;

namespace
{
/// \brief The whole of a text file.
/// \param[in] _path Its path.
/// \return Its text.
/// \throw std::runtime_error if it cannot be read.
std::string ReadFile(const std::string &_path)
{
  std::ifstream in(_path);
  if (!in)
    throw std::runtime_error("cannot read shader [" + _path + "]");
  std::stringstream text;
  text << in.rdbuf();
  return text.str();
}

/// \brief Ogre's texel format for one of the formats SetTexture takes.
/// \param[in] _format The format.
/// \return Ogre's format.
Ogre::PixelFormatGpu OgreFormat(PixelFormat _format)
{
  switch (_format)
  {
    case PF_L8:
      return Ogre::PFG_R8_UNORM;
    case PF_R8G8B8A8:
      return Ogre::PFG_RGBA8_UNORM;
    case PF_FLOAT32_R:
      return Ogre::PFG_R32_FLOAT;
    case PF_FLOAT32_RGBA:
    default:
      return Ogre::PFG_RGBA32_FLOAT;
  }
}

/// \brief Make and compile a GLSL program.
/// \param[in] _name Its name.
/// \param[in] _type Vertex or fragment.
/// \param[in] _path The source's path, or empty for _resource.
/// \param[in] _resource A file of the engine's media, if _path is empty.
/// \return The program.
/// \throw std::runtime_error if it does not compile.
Ogre::HighLevelGpuProgramPtr MakeProgram(const std::string &_name,
    Ogre::GpuProgramType _type, const std::string &_path,
    const std::string &_resource)
{
  auto &programs = Ogre::HighLevelGpuProgramManager::getSingleton();
  Ogre::HighLevelGpuProgramPtr program = programs.createProgram(_name,
      Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, "glsl",
      _type);
  if (_path.empty())
    program->setSourceFile(_resource);
  else
    program->setSource(ReadFile(_path));
  program->setReplaceVersionMacro(true);
  program->load();
  if (program->hasCompileError() || !program->isSupported())
  {
    programs.remove(program);
    throw std::runtime_error("shader [" +
        (_path.empty() ? _resource : _path) + "] does not compile");
  }
  return program;
}
}  // namespace

//////////////////////////////////////////////////
Ogre2ShaderPass::Ogre2ShaderPass()
  : dataPtr(std::make_unique<Ogre2ShaderPassPrivate>())
{
}

//////////////////////////////////////////////////
Ogre2ShaderPass::~Ogre2ShaderPass()
{
  this->Destroy();
}

//////////////////////////////////////////////////
void Ogre2ShaderPass::Destroy()
{
  // Ogre may be gone first, at exit
  if (!Ogre::Root::getSingletonPtr())
    return;
  try
  {
    // The material, which samples the texture, first
    this->dataPtr->unit = nullptr;
    if (this->dataPtr->material)
      Ogre::MaterialManager::getSingleton().remove(this->dataPtr->material);
    this->dataPtr->material.reset();
    if (this->dataPtr->texture)
    {
      Ogre::Root::getSingleton().getRenderSystem()->getTextureGpuManager()
          ->destroyTexture(this->dataPtr->texture);
    }
    this->dataPtr->texture = nullptr;
    auto &programs = Ogre::HighLevelGpuProgramManager::getSingleton();
    for (auto *program : {&this->dataPtr->vertexProgram,
                          &this->dataPtr->fragmentProgram})
    {
      if (*program)
        programs.remove(*program);
      program->reset();
    }
  }
  catch (...)
  {
    // Nothing to do at teardown
  }
}

//////////////////////////////////////////////////
bool Ogre2ShaderPass::IsEnabled() const
{
  // The render target asks this before it chains the passes for a frame,
  // and before PreRender: a shader that did not compile in the frame before
  // takes the pass out of the next one
  return !this->dataPtr->failed && !this->dataPtr->CompileError() &&
      BaseShaderPass<Ogre2RenderPass>::IsEnabled();
}

//////////////////////////////////////////////////
void Ogre2ShaderPass::PreRender(const CameraPtr &_camera)
{
  GZ_PROFILE("Ogre2ShaderPass::PreRender");
  if (this->dataPtr->failed)
    return;
  try
  {
    // Ogre compiles a GLSL program when it is first drawn: a shader that
    // does not compile throws from that frame's render, and is marked
    if (this->dataPtr->CompileError())
      throw std::runtime_error("its shaders do not compile");
    if (this->preRenderCallback)
      this->preRenderCallback(_camera);
    if (!this->dataPtr->material)
      return;

    // The data texture: created again when its size or format changed
    if (this->textureDirty)
    {
      auto *manager = Ogre::Root::getSingleton().getRenderSystem()
          ->getTextureGpuManager();
      if (!this->dataPtr->texture ||
          this->dataPtr->textureWidth != this->textureWidth ||
          this->dataPtr->textureHeight != this->textureHeight ||
          this->dataPtr->textureFormat != this->textureFormat)
      {
        Ogre::TextureGpu *old = this->dataPtr->texture;
        static int textureCounter = 0;
        this->dataPtr->texture = manager->createTexture(
            "ShaderPassData_" + std::to_string(textureCounter++),
            Ogre::GpuPageOutStrategy::SaveToSystemRam,
            Ogre::TextureFlags::ManualTexture, Ogre::TextureTypes::Type2D);
        this->dataPtr->texture->setResolution(this->textureWidth,
                                              this->textureHeight);
        this->dataPtr->texture->setNumMipmaps(1u);
        this->dataPtr->texture->setPixelFormat(
            OgreFormat(this->textureFormat));
        this->dataPtr->texture->scheduleTransitionTo(
            Ogre::GpuResidency::Resident);
        this->dataPtr->unit->setTexture(this->dataPtr->texture);
        if (old)
          manager->destroyTexture(old);
        this->dataPtr->textureWidth = this->textureWidth;
        this->dataPtr->textureHeight = this->textureHeight;
        this->dataPtr->textureFormat = this->textureFormat;
      }
      const Ogre::PixelFormatGpu format = OgreFormat(this->textureFormat);
      Ogre::StagingTexture *staging = manager->getStagingTexture(
          this->textureWidth, this->textureHeight, 1u, 1u, format);
      staging->startMapRegion();
      Ogre::TextureBox box = staging->mapRegion(this->textureWidth,
          this->textureHeight, 1u, 1u, format);
      const std::size_t rowBytes = static_cast<std::size_t>(
          this->textureWidth) * PixelUtil::BytesPerPixel(this->textureFormat);
      for (unsigned int row = 0; row < this->textureHeight; ++row)
      {
        std::memcpy(box.at(0, row, 0),
            this->textureData.data() + row * rowBytes, rowBytes);
      }
      staging->stopMapRegion();
      staging->upload(box, this->dataPtr->texture, 0u);
      manager->removeStagingTexture(staging);
      if (!this->dataPtr->texture->isDataReady())
        this->dataPtr->texture->notifyDataIsReady();
      this->textureDirty = false;
    }

    // The fragment shader's uniforms, every frame: a parameter changed
    // through a reference kept to it does not mark the set as changed
    {
      auto params = this->dataPtr->material->getTechnique(0u)->getPass(0u)
          ->getFragmentProgramParameters();
      for (const auto &[uniform, param] : *this->fragmentParams)
      {
        if (!params->_findNamedConstantDefinition(uniform))
        {
          if (this->dataPtr->warned.insert(uniform).second)
          {
            gzwarn << "ShaderPass: the fragment shader has no uniform ["
                   << uniform << "]\n";
          }
          continue;
        }
        if (ShaderParam::PARAM_FLOAT == param.Type())
        {
          float value;
          param.Value(&value);
          params->setNamedConstant(uniform, value);
        }
        else if (ShaderParam::PARAM_INT == param.Type())
        {
          int value;
          param.Value(&value);
          params->setNamedConstant(uniform, value);
        }
        else if (ShaderParam::PARAM_FLOAT_BUFFER == param.Type())
        {
          std::shared_ptr<void> buffer;
          param.Buffer(buffer);
          params->setNamedConstant(uniform,
              reinterpret_cast<float *>(buffer.get()), param.Count(), 1u);
        }
        else if (ShaderParam::PARAM_INT_BUFFER == param.Type())
        {
          std::shared_ptr<void> buffer;
          param.Buffer(buffer);
          params->setNamedConstant(uniform,
              reinterpret_cast<int *>(buffer.get()), param.Count(), 1u);
        }
        else if (this->dataPtr->warned.insert(uniform).second)
        {
          gzwarn << "ShaderPass: [" << uniform << "] is of a type a shader "
                 << "pass does not set; use the data texture\n";
        }
      }
      this->fragmentParams->ClearDirty();
    }
  }
  catch (const std::exception &_e)
  {
    gzerr << "ShaderPass: " << _e.what() << "; the pass is disabled\n";
    this->dataPtr->failed = true;
    this->SetEnabled(false);
  }
}

//////////////////////////////////////////////////
void Ogre2ShaderPass::CreateRenderPass()
{
  if (!this->ogreCompositorNodeDefName.empty() || this->dataPtr->failed)
    return;

  static int counter = 0;
  const std::string suffix = std::to_string(counter++);
  try
  {
    if (Ogre2RenderEngine::Instance()->GraphicsAPI() != GraphicsAPI::OPENGL)
      throw std::runtime_error("its shaders are GLSL, for OpenGL only");
    if (this->fragmentPath.empty())
      throw std::runtime_error("no fragment shader");

    this->dataPtr->vertexProgram = MakeProgram("ShaderPassVS_" + suffix,
        Ogre::GPT_VERTEX_PROGRAM, this->vertexPath, "shader_pass_vs.glsl");
    this->dataPtr->fragmentProgram = MakeProgram("ShaderPassFS_" + suffix,
        Ogre::GPT_FRAGMENT_PROGRAM, this->fragmentPath, "");

    const std::string materialName = "ShaderPass_" + suffix;
    this->dataPtr->material = Ogre::MaterialManager::getSingleton().create(
        materialName, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    Ogre::Pass *pass =
        this->dataPtr->material->getTechnique(0u)->getPass(0u);
    Ogre::HlmsMacroblock macroblock;
    macroblock.mDepthCheck = false;
    macroblock.mDepthWrite = false;
    macroblock.mCullMode = Ogre::CULL_NONE;
    pass->setMacroblock(macroblock);
    pass->setVertexProgram(this->dataPtr->vertexProgram->getName());
    pass->setFragmentProgram(this->dataPtr->fragmentProgram->getName());
    auto vsParams = pass->getVertexProgramParameters();
    if (vsParams->_findNamedConstantDefinition("worldViewProj"))
    {
      vsParams->setNamedAutoConstant("worldViewProj",
          Ogre::GpuProgramParameters::ACT_WORLDVIEWPROJ_MATRIX);
    }

    Ogre::HlmsSamplerblock samplerblock;
    samplerblock.setAddressingMode(Ogre::TAM_CLAMP);
    samplerblock.setFiltering(Ogre::TFO_BILINEAR);
    // Unit 0: the image so far, set by the compositor
    pass->createTextureUnitState()->setSamplerblock(samplerblock);
    // Unit 1: the data texture, created on the first frame
    this->dataPtr->unit = pass->createTextureUnitState();
    this->dataPtr->unit->setSamplerblock(samplerblock);
    auto fsParams = pass->getFragmentProgramParameters();
    if (fsParams->_findNamedConstantDefinition("inputTexture"))
      fsParams->setNamedConstant("inputTexture", 0);
    if (fsParams->_findNamedConstantDefinition("dataTexture"))
      fsParams->setNamedConstant("dataTexture", 1);
    if (this->textureData.empty())
    {
      const std::uint8_t zero[4] = {0u, 0u, 0u, 0u};
      this->SetTexture(zero, 1u, 1u, PF_R8G8B8A8);
    }
    this->dataPtr->material->load();

    // The node the render target chains, as the Gaussian noise pass's:
    //
    // compositor_node ShaderPassNode_<suffix>
    // {
    //   in 0 rt_input
    //   in 1 rt_output
    //   target rt_output
    //   {
    //     pass render_quad
    //     {
    //       material ShaderPass_<suffix>
    //       input 0 rt_input
    //     }
    //   }
    //   out 0 rt_output
    //   out 1 rt_input
    // }
    Ogre::CompositorManager2 *compositorManager =
        Ogre2RenderEngine::Instance()->OgreRoot()->getCompositorManager2();
    const std::string nodeName = "ShaderPassNode_" + suffix;
    Ogre::CompositorNodeDef *nodeDef =
        compositorManager->addNodeDefinition(nodeName);
    nodeDef->addTextureSourceName("rt_input", 0,
        Ogre::TextureDefinitionBase::TEXTURE_INPUT);
    nodeDef->addTextureSourceName("rt_output", 1,
        Ogre::TextureDefinitionBase::TEXTURE_INPUT);
    nodeDef->setNumTargetPass(1);
    Ogre::CompositorTargetDef *targetDef =
        nodeDef->addTargetPass("rt_output");
    targetDef->setNumPasses(1);
    auto *quad = static_cast<Ogre::CompositorPassQuadDef *>(
        targetDef->addPass(Ogre::PASS_QUAD));
    quad->mMaterialName = materialName;
    quad->addQuadTextureSource(0, "rt_input");
    nodeDef->mapOutputChannel(0, "rt_output");
    nodeDef->mapOutputChannel(1, "rt_input");

    // Last: a throw above leaves the pass out of the chain
    this->ogreCompositorNodeDefName = nodeName;
  }
  catch (const std::exception &_e)
  {
    gzerr << "ShaderPass: " << _e.what() << "; the pass is disabled\n";
    this->Destroy();
    this->dataPtr->failed = true;
    this->SetEnabled(false);
  }
}

GZ_RENDERING_REGISTER_RENDER_PASS(Ogre2ShaderPass, ShaderPass)
