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
#ifndef GZ_RENDERING_OGRE2_OGRE2GPUCOMPRESSION_HH_
#define GZ_RENDERING_OGRE2_OGRE2GPUCOMPRESSION_HH_

#include <memory>
#include <vector>

#include "gz/rendering/config.hh"
#include "gz/rendering/ogre2/Export.hh"

namespace Ogre
{
  class TextureGpu;
}

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    //
    class Ogre2GpuCompressionPrivate;

    /// \brief Converts an ogre2 camera's RGBA8 colour texture to NV12 on the
    /// GPU (compute) and reads it back asynchronously via BufferPacked::readRequest,
    /// so frame N's transfer overlaps frame N+1's render. Not part of the public,
    /// engine-agnostic API.
    class GZ_RENDERING_OGRE2_VISIBLE Ogre2GpuCompression
    {
      /// \brief Constructor.
      public: Ogre2GpuCompression();

      /// \brief Destructor. Drains in-flight copies and releases resources.
      public: ~Ogre2GpuCompression();

      /// \brief Size the NV12 UAV buffer and the staging ring for a source size.
      /// \param[in] _width Source texture width.
      /// \param[in] _height Source texture height.
      /// \param[in] _ringDepth Number of in-flight readback tickets (>=2 for pipelining).
      public: void Configure(unsigned int _width, unsigned int _height,
                  unsigned int _ringDepth = 2u);

      /// \brief Dispatch the RGBA->NV12 compute job for _src into the NV12 UAV
      /// buffer, then issue a non-blocking readback ticket via readRequest().
      /// \param[in] _src Resident RGBA8 source texture.
      public: void ConvertToNv12(Ogre::TextureGpu *_src);

      /// \brief Try to retrieve the oldest completed NV12 result as packed
      /// bytes (width*height*3/2). Non-blocking: returns false if no ticket has
      /// finished transferring.
      /// \param[out] _out Filled with width*height*3/2 bytes on success.
      /// \return True if a frame was retrieved.
      public: bool TryRetrieveNv12(std::vector<unsigned char> &_out);

      /// \brief Drain in-flight tickets and free the UAV buffer.
      public: void Reset();

      /// \brief Private data.
      private: std::unique_ptr<Ogre2GpuCompressionPrivate> dataPtr;
    };
    }
  }
}
#endif
