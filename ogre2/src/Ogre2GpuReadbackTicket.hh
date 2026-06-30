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
#ifndef GZ_RENDERING_OGRE2_OGRE2GPUREADBACKTICKET_HH_
#define GZ_RENDERING_OGRE2_OGRE2GPUREADBACKTICKET_HH_

#include "gz/rendering/config.hh"

#ifdef _MSC_VER
  #pragma warning(push, 0)
#endif
#include <OgrePixelFormatGpu.h>
#include <OgreTextureBox.h>
#ifdef _MSC_VER
  #pragma warning(pop)
#endif

namespace Ogre
{
  class AsyncTextureTicket;
  class TextureGpu;
}

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    //
    /// \brief Reusable GPU->CPU texture readback backed by a single, persistent
    /// Ogre::AsyncTextureTicket. Unlike Ogre::Image2::convertFromTexture (which
    /// allocates a fresh staging buffer + ticket and an extra copy every call),
    /// this keeps one ticket and maps its staging memory directly, so callers
    /// copy once straight into their own persistent buffer.
    /// Render-thread use only; NOT thread-safe.
    class Ogre2GpuReadbackTicket
    {
      /// \brief Destructor; destroys the persistent ticket if still alive.
      public: ~Ogre2GpuReadbackTicket();

      /// \brief Issue a blocking GPU->CPU download of \p _texture (mip 0,
      /// slice 0) into the persistent ticket and map it for reading. The
      /// ticket is lazily created and recreated whenever the texture's
      /// width/height/pixel-format changes. The caller MUST read the data out
      /// honoring TextureBox::bytesPerRow and then call Unmap() before the next
      /// frame's DownloadAndMap().
      /// \param[in] _texture Source GPU texture.
      /// \return Mapped TextureBox; box.data is nullptr on failure.
      public: Ogre::TextureBox DownloadAndMap(Ogre::TextureGpu *_texture);

      /// \brief Unmap the ticket previously mapped by DownloadAndMap().
      /// Safe to call when nothing is mapped.
      public: void Unmap();

      /// \brief Destroy the persistent ticket now. Call from the owner's
      /// Destroy() while the render engine (and its texture manager) is still
      /// alive, ahead of engine teardown.
      public: void Destroy();

      /// \brief The persistent ticket, or nullptr before first use / after
      /// Destroy().
      private: Ogre::AsyncTextureTicket *ticket{nullptr};

      /// \brief Cached ticket geometry; a change forces recreation.
      private: unsigned int width{0u};

      /// \brief Cached ticket height; see width.
      private: unsigned int height{0u};

      /// \brief Cached ticket pixel format; see width.
      private: Ogre::PixelFormatGpu format{Ogre::PFG_UNKNOWN};

      /// \brief True while the ticket is currently mapped.
      private: bool mapped{false};
    };

    /// \brief Whether to use the legacy Ogre::Image2::convertFromTexture
    /// readback path instead of the persistent-ticket path. True if the env
    /// var GZ_RENDERING_OGRE2_LEGACY_READBACK is set. Evaluated once.
    /// \todo Deprecate this temporary fallback env var in Gazebo N /
    /// gz-rendering12, then remove it in Gazebo O / gz-rendering13. If the new
    /// readback path works well, remove it in Gazebo N / gz-rendering12.
    /// \return true to use the legacy path.
    bool Ogre2UseLegacyReadback();
    }
  }
}
#endif
