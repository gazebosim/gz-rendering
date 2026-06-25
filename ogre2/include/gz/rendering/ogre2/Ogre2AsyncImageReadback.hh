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
#ifndef GZ_RENDERING_OGRE2_OGRE2ASYNCIMAGEREADBACK_HH_
#define GZ_RENDERING_OGRE2_OGRE2ASYNCIMAGEREADBACK_HH_

#include <deque>
#include <vector>

#include "gz/rendering/config.hh"
#include "gz/rendering/ogre2/Export.hh"

namespace Ogre
{
  class TextureGpu;
  class AsyncTextureTicket;
}

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    //
    /// \brief Multi-buffered ASYNC RGBA8 GPU->CPU readback built on OGRE's
    /// AsyncTextureTicket. There is NO compression here: the point is to
    /// isolate the async / deferred-map / multi-buffering benefit from the
    /// byte-volume reduction of NV12.
    ///
    /// Each frame RequestDownload() issues a NON-BLOCKING texture download into
    /// a staging ticket; TryRetrieve() maps the oldest finished ticket and
    /// copies its tightly-packed RGBA8 bytes. A ring of \c _ringDepth tickets
    /// lets several downloads be in flight, so the GPU->CPU DMA of frame N
    /// overlaps the render/submit of frame N+1 instead of stalling on a GPU
    /// fence inline like the synchronous Ogre2RenderTarget::Copy()
    /// (Ogre::Image2::copyContentsToMemory).
    ///
    /// Same byte volume as the raw path (w*h*4): the ONLY variable versus the
    /// blocking Copy is async-ness.
    class GZ_RENDERING_OGRE2_VISIBLE Ogre2AsyncImageReadback
    {
      /// \brief Constructor.
      public: Ogre2AsyncImageReadback();

      /// \brief Destructor. Drains in-flight copies and releases tickets.
      public: ~Ogre2AsyncImageReadback();

      /// \brief (Re)configure for the given dimensions and ring depth. Tickets
      /// are created lazily on first RequestDownload (when the source pixel
      /// format is known). Changing dimensions resets the ring.
      /// \param[in] _width  Image width in pixels.
      /// \param[in] _height Image height in pixels.
      /// \param[in] _ringDepth Max in-flight downloads (default 3).
      public: void Configure(unsigned int _width, unsigned int _height,
                  unsigned int _ringDepth = 3u);

      /// \brief Issue a non-blocking download of _src's mip 0 into a staging
      /// ticket. If the ring is full the oldest in-flight ticket is
      /// force-drained (a brief stall) and recycled, so a rendered frame is
      /// never silently lost.
      /// \param[in] _src Resident colour texture to read back.
      public: void RequestDownload(Ogre::TextureGpu *_src);

      /// \brief If the oldest in-flight download has finished, map it, copy its
      /// tightly-packed RGBA8 bytes into _out, recycle the ticket and return
      /// true. Otherwise return false WITHOUT blocking.
      /// \param[out] _out Receives w*h*4 RGBA8 bytes on success.
      /// \return True if a finished frame was retrieved.
      public: bool TryRetrieve(std::vector<unsigned char> &_out);

      /// \brief Drain and destroy all tickets and release resources.
      public: void Reset();

      /// \brief Get an idle ticket: reuse an idle one, create one (up to
      /// ringDepth), or evict + recycle the oldest in-flight ticket.
      private: Ogre::AsyncTextureTicket *AcquireTicket(Ogre::TextureGpu *_src);

      /// \brief Image width in pixels.
      private: unsigned int width = 0u;

      /// \brief Image height in pixels.
      private: unsigned int height = 0u;

      /// \brief Max in-flight downloads.
      private: unsigned int ringDepth = 3u;

      /// \brief Number of tickets created so far (<= ringDepth).
      private: unsigned int created = 0u;

      /// \brief Downloads issued but not yet retrieved (oldest at front).
      private: std::deque<Ogre::AsyncTextureTicket *> inflight;

      /// \brief Idle tickets available for reuse.
      private: std::vector<Ogre::AsyncTextureTicket *> idle;
    };
    }
  }
}
#endif
