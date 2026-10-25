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
#include "Ogre2GpuReadbackTicket.hh"

#include <chrono>
#include <cstdlib>
#include <ctime>
#include <thread>

#ifdef __linux__
#include <sys/prctl.h>
#endif

#include <gz/common/Console.hh>

#include "gz/rendering/ogre2/Ogre2RenderEngine.hh"
#include "Ogre2ReadbackWaitPolicy.hh"

#ifdef _MSC_VER
  #pragma warning(push, 0)
#endif
#include <OgreRoot.h>
#include <OgreRenderSystem.h>
#include <OgreTextureGpu.h>
#include <OgreTextureGpuManager.h>
#include <OgreAsyncTextureTicket.h>
#ifdef _MSC_VER
  #pragma warning(pop)
#endif

using namespace gz;
using namespace rendering;

//////////////////////////////////////////////////
/// \brief Fetch the active OGRE render system, or nullptr if unavailable.
static Ogre::RenderSystem *ReadbackRenderSystem()
{
  auto engine = Ogre2RenderEngine::Instance();
  if (!engine)
    return nullptr;
  auto ogreRoot = engine->OgreRoot();
  if (!ogreRoot)
    return nullptr;
  return ogreRoot->getRenderSystem();
}

//////////////////////////////////////////////////
/// \brief Fetch the active OGRE texture manager, or nullptr if unavailable.
static Ogre::TextureGpuManager *ReadbackTextureManager()
{
  auto renderSystem = ReadbackRenderSystem();
  return renderSystem ? renderSystem->getTextureGpuManager() : nullptr;
}

//////////////////////////////////////////////////
/// \brief Whether a download may be waited for by polling its fence with
/// short sleeps (WaitForDownload) rather than in map(); Ogre2ReadbackWaitPolicy
/// then chooses per wait. OpenGL only: there the driver may busy-wait in
/// map() (glClientWaitSync) for the whole GPU latency, while Vulkan's
/// flushCommands() does not submit the download.
static bool PollReadback()
{
  auto engine = Ogre2RenderEngine::Instance();
  return engine && engine->GraphicsAPI() == GraphicsAPI::OPENGL;
}

//////////////////////////////////////////////////
/// \brief Whether the env var GZ_RENDERING_OGRE2_BLOCKING_READBACK_WAIT
/// forces the wait in map(). Read on every call so tests can compare both
/// waits.
static bool BlockingReadbackForced()
{
  return std::getenv("GZ_RENDERING_OGRE2_BLOCKING_READBACK_WAIT") != nullptr;
}

//////////////////////////////////////////////////
/// \brief Read the monotonic wall time and the calling thread's CPU time.
/// \param[out] _wallUs Wall time in us.
/// \param[out] _cpuUs Thread CPU time in us.
/// \return False where the thread's CPU time is not available (no POSIX
/// CLOCK_THREAD_CPUTIME_ID, e.g. Windows): the readback then waits in
/// map(), as it did before the sleeping wait existed.
static bool ReadbackClocks(double &_wallUs, double &_cpuUs)
{
#ifdef CLOCK_THREAD_CPUTIME_ID
  timespec cpu;
  if (clock_gettime(CLOCK_THREAD_CPUTIME_ID, &cpu) != 0)
    return false;
  _cpuUs = cpu.tv_sec * 1e6 + cpu.tv_nsec * 1e-3;
  _wallUs = std::chrono::duration<double, std::micro>(
      std::chrono::steady_clock::now().time_since_epoch()).count();
  return true;
#else
  (void)_wallUs;
  (void)_cpuUs;
  return false;
#endif
}

//////////////////////////////////////////////////
/// \brief Wait for the download of \p _ticket to finish without spinning,
/// so that the following map() returns at once. Same data as waiting in
/// map(): only who waits for the fence changes.
/// \param[in] _ticket Ticket with a download issued with accurate tracking.
/// \param[in] _renderSystem Render system that issued the download.
static void WaitForDownload(Ogre::AsyncTextureTicket *_ticket,
    Ogre::RenderSystem *_renderSystem)
{
  using Clock = std::chrono::steady_clock;
  if (_ticket->queryIsTransferDone())
    return;
  // queryIsTransferDone() polls the fence without flushing (glClientWaitSync
  // without GL_SYNC_FLUSH_COMMANDS_BIT): submit the commands once, or the
  // fence may never signal.
  _renderSystem->flushCommands();
#ifdef __linux__
  // Sleep 10 us at a time, and wake up on time: with the default 50 us timer
  // slack every poll would add up to 50 us to the render thread's latency.
  const int oldSlack = prctl(PR_GET_TIMERSLACK, 0, 0, 0, 0);
  prctl(PR_SET_TIMERSLACK, 1000UL, 0, 0, 0);
#endif
  const auto start = Clock::now();
  // Poll for at most 100 ms; then map() waits for the fence as it always
  // did.
  while (!_ticket->queryIsTransferDone() &&
         Clock::now() - start < std::chrono::milliseconds(100))
  {
    std::this_thread::sleep_for(std::chrono::microseconds(10));
  }
#ifdef __linux__
  if (oldSlack > 0)
  {
    prctl(PR_SET_TIMERSLACK,
          static_cast<unsigned long>(oldSlack),  // NOLINT(runtime/int)
          0, 0, 0);
  }
#endif
}

//////////////////////////////////////////////////
/// \brief Wait for the download of \p _ticket and map it. The calling
/// thread's Ogre2ReadbackWaitPolicy chooses whether to sleep
/// (WaitForDownload) or to spin in map(), and learns each wait's idle time
/// so the thread's slack excludes it. Every OpenGL wait is measured, also
/// the ones GZ_RENDERING_OGRE2_BLOCKING_READBACK_WAIT forces into map().
/// \param[in] _ticket Ticket with a download issued with accurate tracking.
/// \return The mapped box.
static Ogre::TextureBox WaitAndMap(Ogre::AsyncTextureTicket *_ticket)
{
  // One per thread: all the readbacks of a render thread share its slack.
  static thread_local Ogre2ReadbackWaitPolicy policy;
  double wallUs = 0;
  double cpuUs = 0;
  const bool openGl = PollReadback();
  const bool measured = openGl && ReadbackClocks(wallUs, cpuUs);
  if (openGl && !measured)
    policy.Restart();
  if (measured && policy.Sleep(wallUs, cpuUs) && !BlockingReadbackForced())
    WaitForDownload(_ticket, ReadbackRenderSystem());
  Ogre::TextureBox box = _ticket->map(0u);
  double wallEndUs = 0;
  double cpuEndUs = 0;
  if (measured)
  {
    if (ReadbackClocks(wallEndUs, cpuEndUs))
      policy.WaitIdle((wallEndUs - wallUs) - (cpuEndUs - cpuUs));
    else
      policy.Restart();
  }
  return box;
}

//////////////////////////////////////////////////
Ogre2GpuReadbackTicket::~Ogre2GpuReadbackTicket()
{
  this->Destroy();
}

//////////////////////////////////////////////////
void Ogre2GpuReadbackTicket::Destroy()
{
  if (!this->ticket)
    return;

  if (this->mapped)
  {
    this->ticket->unmap();
    this->mapped = false;
  }

  auto textureMgr = ReadbackTextureManager();
  if (textureMgr)
    textureMgr->destroyAsyncTextureTicket(this->ticket);
  else
    gzwarn << "Ogre2GpuReadbackTicket::Destroy() called after engine "
              "teardown; ticket staging buffer leaked." << std::endl;

  this->ticket = nullptr;
  this->downloaded = false;
  this->width = 0u;
  this->height = 0u;
  this->format = Ogre::PFG_UNKNOWN;
}

//////////////////////////////////////////////////
Ogre::TextureBox Ogre2GpuReadbackTicket::DownloadAndMap(
    Ogre::TextureGpu *_texture)
{
  if (!this->Download(_texture))
    return Ogre::TextureBox();  // data == nullptr
  return this->MapDownloaded();
}

//////////////////////////////////////////////////
bool Ogre2GpuReadbackTicket::Download(Ogre::TextureGpu *_texture)
{
  if (!_texture)
    return false;

  auto textureMgr = ReadbackTextureManager();
  if (!textureMgr)
    return false;

  const unsigned int w = _texture->getWidth();
  const unsigned int h = _texture->getHeight();
  const Ogre::PixelFormatGpu fmt = _texture->getPixelFormat();

  // (Re)create the ticket on first use or when the geometry/format changed.
  if (!this->ticket || this->width != w || this->height != h ||
      this->format != fmt)
  {
    if (this->ticket)
    {
      if (this->mapped)
      {
        this->ticket->unmap();
        this->mapped = false;
      }
      textureMgr->destroyAsyncTextureTicket(this->ticket);
      this->ticket = nullptr;
    }
    this->ticket = textureMgr->createAsyncTextureTicket(
        w, h, 1u, Ogre::TextureTypes::Type2D, fmt);
    this->width = w;
    this->height = h;
    this->format = fmt;
  }

  if (!this->ticket)
    return false;

  if (this->mapped)
  {
    gzwarn << "Ogre2GpuReadbackTicket::Download() called while still "
              "mapped; unmapping previous frame first." << std::endl;
    this->ticket->unmap();
    this->mapped = false;
  }

  // accurateTracking=true: the download gets its own fence.
  this->ticket->download(_texture, 0u, true);
  this->downloaded = true;
  return true;
}

//////////////////////////////////////////////////
Ogre::TextureBox Ogre2GpuReadbackTicket::MapDownloaded()
{
  if (!this->ticket || !this->downloaded)
    return Ogre::TextureBox();  // data == nullptr
  Ogre::TextureBox box = WaitAndMap(this->ticket);
  this->downloaded = false;
  this->mapped = true;
  return box;
}

//////////////////////////////////////////////////
void Ogre2GpuReadbackTicket::Unmap()
{
  if (this->ticket && this->mapped)
  {
    this->ticket->unmap();
    this->mapped = false;
  }
}

//////////////////////////////////////////////////
bool gz::rendering::Ogre2UseLegacyReadback()
{
  static const bool legacy =
      (std::getenv("GZ_RENDERING_OGRE2_LEGACY_READBACK") != nullptr);
  return legacy;
}
