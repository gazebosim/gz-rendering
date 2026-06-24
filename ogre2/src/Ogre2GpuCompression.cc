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
#include <deque>
#include <vector>

#include <gz/common/Console.hh>

#include "gz/rendering/ogre2/Ogre2GpuCompression.hh"
#include "gz/rendering/ogre2/Ogre2RenderEngine.hh"

#ifdef _MSC_VER
  #pragma warning(push, 0)
#endif
#include <OgreRoot.h>
#include <OgreRenderSystem.h>
#include <OgreTextureGpu.h>
#include <OgreHlmsCompute.h>
#include <OgreHlmsComputeJob.h>
#include <OgreHlmsManager.h>
#include <OgreDescriptorSetTexture.h>
#include <OgreDescriptorSetUav.h>
#include <OgreShaderParams.h>
#include <Vao/OgreVaoManager.h>
#include <Vao/OgreUavBufferPacked.h>
#include <Vao/OgreAsyncTicket.h>
#ifdef _MSC_VER
  #pragma warning(pop)
#endif

/// \brief One in-flight async readback ticket for a dispatched NV12 frame.
struct Nv12Inflight
{
  /// \brief Non-blocking ticket issued by UavBufferPacked::readRequest().
  Ogre::AsyncTicketPtr ticket;
};

class gz::rendering::Ogre2GpuCompressionPrivate
{
  /// \brief Source dimensions.
  public: unsigned int width = 0;
  public: unsigned int height = 0;

  /// \brief Ring depth (max concurrent in-flight readback tickets).
  /// Default 3 (latency+1) so steady single-camera pumping never hits the cap.
  public: unsigned int ringDepth = 3u;

  /// \brief NV12 byte count for the configured dimensions.
  /// Each NV12 byte is stored as one uint32 in the UAV (low 8 bits).
  public: size_t Nv12Bytes() const
          { return static_cast<size_t>(width) * height * 3u / 2u; }

  /// \brief The RGBA->NV12 compute job (looked up once from the JSON material).
  public: Ogre::HlmsComputeJob *nv12Job = nullptr;

  /// \brief UAV buffer the compute job writes into (uint-per-byte packing).
  /// numElements = w*h*3/2, bytesPerElement = sizeof(uint32_t).
  public: Ogre::UavBufferPacked *nv12Uav = nullptr;

  /// \brief In-flight async readback tickets, oldest first.
  public: std::deque<Nv12Inflight> inflight;
};

using namespace gz;
using namespace rendering;

//////////////////////////////////////////////////
Ogre2GpuCompression::Ogre2GpuCompression()
  : dataPtr(new Ogre2GpuCompressionPrivate)
{
}

//////////////////////////////////////////////////
Ogre2GpuCompression::~Ogre2GpuCompression()
{
  this->Reset();
}

//////////////////////////////////////////////////
void Ogre2GpuCompression::Configure(unsigned int _width, unsigned int _height,
    unsigned int _ringDepth)
{
  if (_ringDepth < 2u)
    _ringDepth = 2u;

  // NV12 4:2:0 requires even width and height; odd dimensions would silently
  // truncate the Nv12Bytes() calculation (w*h*3/2 integer division).
  // This guard MUST run BEFORE the unchanged-config early-return below:
  // otherwise an odd-sized RESIZE could early-return on a partial match and
  // leave a stale even-dimension UAV bound to the new (odd) texture.
  if ((_width & 1u) || (_height & 1u))
  {
    gzerr << "Ogre2GpuCompression::Configure: NV12 requires even width and "
          << "height, got " << _width << "x" << _height
          << ". Helper left unconfigured." << std::endl;
    // Tear down any prior even-dimension state so nothing stale stays bound.
    this->Reset();
    return;
  }

  if (this->dataPtr->width == _width &&
      this->dataPtr->height == _height &&
      this->dataPtr->ringDepth == _ringDepth &&
      this->dataPtr->nv12Uav)
  {
    return;
  }

  this->Reset();
  this->dataPtr->width = _width;
  this->dataPtr->height = _height;
  this->dataPtr->ringDepth = _ringDepth;

  // Allocate the UAV buffer: one uint32 per NV12 output byte (uint-per-byte
  // packing strategy from the Task 4 compute shader design; disjoint writes
  // from each thread are race-free). The host compacts in TryRetrieveNv12.
  auto *engine = Ogre2RenderEngine::Instance();
  Ogre::VaoManager *vaoManager =
      engine->OgreRoot()->getRenderSystem()->getVaoManager();

  const size_t numElems = this->dataPtr->Nv12Bytes();
  this->dataPtr->nv12Uav = vaoManager->createUavBuffer(
      numElems,
      static_cast<uint32_t>(sizeof(uint32_t)),
      Ogre::BB_FLAG_UAV,
      nullptr,
      false);
}

//////////////////////////////////////////////////
void Ogre2GpuCompression::ConvertToNv12(Ogre::TextureGpu *_src)
{
  if (!_src || !this->dataPtr->nv12Uav)
    return;

  // Bound memory WITHOUT silently dropping the newly rendered frame: if the
  // ring is already full, evict the OLDEST in-flight ticket (never the newest)
  // before issuing this frame's dispatch+readback below. This keeps memory
  // capped at ringDepth while guaranteeing the current frame is always
  // converted; the consumer's TryRetrieveNv12 drain loop then delivers every
  // ticket that does complete.
  while (this->dataPtr->inflight.size() >= this->dataPtr->ringDepth)
  {
    Nv12Inflight &old = this->dataPtr->inflight.front();
    if (old.ticket)
    {
      // Force-complete the oldest DMA (map() blocks until done) then release,
      // mirroring the Reset() drain discipline so no in-flight transfer is
      // freed mid-fence. This is a rare back-pressure path, not steady state.
      old.ticket->map();
      old.ticket->unmap();
      old.ticket.setNull();
    }
    this->dataPtr->inflight.pop_front();
  }

  const unsigned int w = this->dataPtr->width;
  const unsigned int h = this->dataPtr->height;

  auto *engine = Ogre2RenderEngine::Instance();
  Ogre::Root *root = engine->OgreRoot();
  Ogre::HlmsManager *hlmsManager = root->getHlmsManager();
  Ogre::HlmsCompute *hlmsCompute = hlmsManager->getComputeHlms();

  // Look up the compute job once and cache it.
  if (!this->dataPtr->nv12Job)
    this->dataPtr->nv12Job = hlmsCompute->findComputeJob("Compress/Rgba2Nv12");

  Ogre::HlmsComputeJob *job = this->dataPtr->nv12Job;
  if (!job)
  {
    gzerr << "Ogre2GpuCompression: compute job 'Compress/Rgba2Nv12' not found."
          << std::endl;
    return;
  }

  // --- Bind input texture (slot 0) ---
  // Request a UNORM (non-sRGB) alias view of the source texture.
  // The camera's render target is PFG_RGBA8_UNORM_SRGB, and the colorimetry
  // contract (transferCharacteristics=13, sRGB preserved) means the GPU compute
  // must read the raw sRGB-ENCODED stored bytes without gamma decoding — the
  // same values returned by the blocking Capture() / copyContentsToMemory path.
  // Setting pixelFormat=PFG_RGBA8_UNORM here creates an aliased view so that
  // texelFetch sees the raw stored bytes on both OpenGL (where texelFetch on
  // sampler2D with sRGB target linearizes) and Vulkan (where the image view
  // format determines sRGB decode). Without this, GPU NV12 would diverge by
  // ~50 LSBs from the CPU reference for typical scene colours.
  Ogre::DescriptorSetTexture2::TextureSlot texSlot(
      Ogre::DescriptorSetTexture2::TextureSlot::makeEmpty());
  texSlot.texture = _src;
  texSlot.pixelFormat = Ogre::PFG_RGBA8_UNORM;
  job->setTexture(0u, texSlot);

  // --- Bind output UAV buffer (slot 0) ---
  Ogre::DescriptorSetUav::BufferSlot uavSlot(
      Ogre::DescriptorSetUav::BufferSlot::makeEmpty());
  uavSlot.buffer = this->dataPtr->nv12Uav;
  uavSlot.access = Ogre::ResourceAccess::Write;
  job->_setUavBuffer(0u, uavSlot);

  // --- Set imgWidth / imgHeight params ---
  // The JSON material pre-declares both params in "default" so findParameter
  // will succeed.  We update their values and mark dirty.
  Ogre::ShaderParams &params = job->getShaderParams("default");
  {
    Ogre::ShaderParams::Param *pW = params.findParameter("imgWidth");
    if (pW)
    {
      pW->setManualValue(static_cast<uint32_t>(w));
    }
    else
    {
      Ogre::ShaderParams::Param newP;
      newP.name = "imgWidth";
      newP.isAutomatic = false;
      newP.isEx = false;
      newP.isDirty = true;
      newP.setManualValue(static_cast<uint32_t>(w));
      params.mParams.push_back(newP);
    }
    Ogre::ShaderParams::Param *pH = params.findParameter("imgHeight");
    if (pH)
    {
      pH->setManualValue(static_cast<uint32_t>(h));
    }
    else
    {
      Ogre::ShaderParams::Param newP;
      newP.name = "imgHeight";
      newP.isAutomatic = false;
      newP.isEx = false;
      newP.isDirty = true;
      newP.setManualValue(static_cast<uint32_t>(h));
      params.mParams.push_back(newP);
    }
  }
  params.setDirty();

  // --- Dispatch ---
  // NOTE: Do NOT call setNumThreadGroupsBasedOn here.
  // The JSON material already sets thread_groups_based_on_texture with
  // divisor [2,2,1] (slot 0).  Calling it again from code would double-set it
  // and potentially clobber the JSON-driven slot reference.
  hlmsCompute->dispatch(job, nullptr, nullptr);

  // --- Issue a non-blocking readback via readRequest() ---
  // readRequest(elementStart, elementCount) issues a GPU->CPU DMA and returns
  // an AsyncTicketPtr immediately (non-blocking).  We poll queryIsTransferDone
  // in TryRetrieveNv12 (drained every frame) without ever stalling the render
  // thread. Up to ringDepth tickets may be in flight to absorb multi-frame DMA
  // latency; the oldest is evicted above if that cap is reached.
  const size_t numElems = this->dataPtr->Nv12Bytes();
  Nv12Inflight rec;
  rec.ticket = this->dataPtr->nv12Uav->readRequest(0u, numElems);
  this->dataPtr->inflight.push_back(std::move(rec));
}

//////////////////////////////////////////////////
bool Ogre2GpuCompression::TryRetrieveNv12(std::vector<unsigned char> &_out)
{
  if (this->dataPtr->inflight.empty())
    return false;

  Nv12Inflight &rec = this->dataPtr->inflight.front();

  if (!rec.ticket)
    return false;

  // Non-blocking poll: only proceed if the GPU transfer is done.
  if (!rec.ticket->queryIsTransferDone())
    return false;

  const size_t numBytes = this->dataPtr->Nv12Bytes();
  _out.resize(numBytes);

  // Map the ticket's CPU-side staging data.
  // The UAV stores one uint32 per NV12 output byte (low 8 bits hold the
  // byte value; upper bits zero from the compute shader write).
  const void *mapped = rec.ticket->map();
  const uint32_t *words = static_cast<const uint32_t *>(mapped);
  for (size_t i = 0; i < numBytes; ++i)
    _out[i] = static_cast<unsigned char>(words[i] & 0xFFu);
  rec.ticket->unmap();

  // Release the ticket before popping (SharedPtr ref-count drop).
  rec.ticket.setNull();
  this->dataPtr->inflight.pop_front();
  return true;
}

//////////////////////////////////////////////////
void Ogre2GpuCompression::Reset()
{
  // Block-drain every in-flight ticket so its DMA fence has signalled before
  // we free the UAV buffer.  map() blocks until the transfer is done;
  // unmap() releases the CPU-side mapping.  This blocking is acceptable here
  // because Reset() is teardown / reconfigure, NOT the render hot-path.
  for (Nv12Inflight &rec : this->dataPtr->inflight)
  {
    if (rec.ticket)
    {
      rec.ticket->map();
      rec.ticket->unmap();
      rec.ticket.setNull();
    }
  }
  this->dataPtr->inflight.clear();

  if (this->dataPtr->nv12Uav)
  {
    auto *engine = Ogre2RenderEngine::Instance();
    Ogre::VaoManager *vaoManager =
        engine->OgreRoot()->getRenderSystem()->getVaoManager();
    vaoManager->destroyUavBuffer(this->dataPtr->nv12Uav);
    this->dataPtr->nv12Uav = nullptr;
  }

  this->dataPtr->nv12Job = nullptr;
  this->dataPtr->width = 0;
  this->dataPtr->height = 0;
}
