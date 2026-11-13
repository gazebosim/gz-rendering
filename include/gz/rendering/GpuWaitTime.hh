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
#ifndef GZ_RENDERING_GPUWAITTIME_HH_
#define GZ_RENDERING_GPUWAITTIME_HH_

#include <chrono>
#include <string>

#include <gz/rendering/config.hh>

#include "gz/rendering/Export.hh"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    //
    class RenderEngine;

    /// \brief Get the total time the calling thread has spent blocked on the
    /// GPU so far.
    ///
    /// Render engines add to this counter around each point where the CPU
    /// has to stop and wait for the GPU, such as reading back a rendered image
    /// or waiting for a frame to finish. For a readback that includes the time
    /// the GPU takes to transfer the image out of its memory. Copying the image
    /// on the CPU after that is not waiting on the GPU, and ogre2 leaves it
    /// out. The ogre engine counts it, since Ogre 1 transfers and copies the
    /// image in one call.
    ///
    /// It is a per thread monotonic counter: sample it before and after a
    /// piece of work to learn how much of the wall time of that work was spent
    /// waiting on the GPU.
    ///
    /// What this measures is the GPU's effect on the calling thread, not how
    /// busy the GPU was. GPU work that finishes while the CPU is still
    /// submitting commands costs the thread nothing and is not counted, which
    /// is what makes the value useful to tell whether a thread is limited by
    /// the CPU or by the GPU.
    ///
    /// A counter that stays at zero while rendering means that the render
    /// engine in use does not report its GPU waits.
    /// \return Accumulated wait time of the calling thread
    GZ_RENDERING_VISIBLE
    std::chrono::nanoseconds ThreadGpuWaitTime();

    /// \brief Add time to the GPU wait counter of the calling thread. Meant
    /// for render engines, see ScopedGpuWait.
    /// \param[in] _time Time that the calling thread was blocked on the GPU
    GZ_RENDERING_VISIBLE
    void AddThreadGpuWaitTime(std::chrono::nanoseconds _time);

    /// \brief Check if a render engine runs on a software rasterizer, such
    /// as llvmpipe, instead of on a GPU. Its GPU waits are then CPU time.
    /// \param[in] _engine Render engine to check
    /// \return True if the rendering work runs on the CPU. False if it runs
    /// on a GPU, or if the engine did not report it.
    GZ_RENDERING_VISIBLE
    bool IsSoftwareRenderer(const RenderEngine *_engine);

    /// \brief Report whether a render engine runs on a software rasterizer.
    /// Meant for render engines, once their graphics device is known.
    /// \param[in] _engine Render engine that reports
    /// \param[in] _software True if it renders on the CPU
    GZ_RENDERING_VISIBLE
    void SetSoftwareRenderer(const RenderEngine *_engine, bool _software);

    /// \brief Check if the name of a graphics device is that of a known
    /// software rasterizer.
    /// \param[in] _name Name of the device, as reported by the graphics API
    /// \return True if the device is a software rasterizer
    GZ_RENDERING_VISIBLE
    bool IsSoftwareDeviceName(const std::string &_name);

    /// \brief Counts the lifetime of this object as time that the calling
    /// thread spent blocked on the GPU. Render engines place it around their
    /// blocking GPU synchronization points.
    class ScopedGpuWait
    {
      /// \brief Constructor. Starts timing.
      public: ScopedGpuWait()
          : start(std::chrono::steady_clock::now())
      {
      }

      /// \brief Destructor. Adds the elapsed time to the calling thread.
      public: ~ScopedGpuWait()
      {
        AddThreadGpuWaitTime(std::chrono::duration_cast<
            std::chrono::nanoseconds>(
              std::chrono::steady_clock::now() - this->start));
      }

      /// \brief Not copyable
      public: ScopedGpuWait(const ScopedGpuWait &) = delete;

      /// \brief Not assignable
      public: ScopedGpuWait &operator=(const ScopedGpuWait &) = delete;

      /// \brief Time at which the wait started
      private: std::chrono::steady_clock::time_point start;
    };
    }
  }
}
#endif
