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

#include "gz/rendering/GpuWaitTime.hh"

#include <algorithm>
#include <cctype>
#include <mutex>
#include <string>
#include <unordered_map>

#include <gz/utils/NeverDestroyed.hh>

namespace
{
/// \brief Time spent blocked on the GPU by the current thread
thread_local std::chrono::nanoseconds threadGpuWaitTime{0};

/// \brief Engines that reported whether they render in software.
struct SoftwareRenderers
{
  /// \brief Protects engines
  std::mutex mutex;

  /// \brief True for each engine that renders on the CPU
  std::unordered_map<const gz::rendering::RenderEngine *, bool> engines;
};

/// \brief Get the engines that reported. Never destroyed, engines are
/// singletons that can be torn down late at exit.
/// \return The reports
SoftwareRenderers &softwareRenderers()
{
  static gz::utils::NeverDestroyed<SoftwareRenderers> renderers;
  return renderers.Access();
}
}

namespace gz
{
namespace rendering
{
inline namespace GZ_RENDERING_VERSION_NAMESPACE {

//////////////////////////////////////////////////
std::chrono::nanoseconds ThreadGpuWaitTime()
{
  return threadGpuWaitTime;
}

//////////////////////////////////////////////////
void AddThreadGpuWaitTime(std::chrono::nanoseconds _time)
{
  threadGpuWaitTime += _time;
}

//////////////////////////////////////////////////
bool IsSoftwareRenderer(const RenderEngine *_engine)
{
  auto &renderers = softwareRenderers();
  std::lock_guard<std::mutex> lock(renderers.mutex);
  auto it = renderers.engines.find(_engine);
  return it != renderers.engines.end() && it->second;
}

//////////////////////////////////////////////////
void SetSoftwareRenderer(const RenderEngine *_engine, bool _software)
{
  auto &renderers = softwareRenderers();
  std::lock_guard<std::mutex> lock(renderers.mutex);
  renderers.engines[_engine] = _software;
}

//////////////////////////////////////////////////
bool IsSoftwareDeviceName(const std::string &_name)
{
  std::string name = _name;
  std::transform(name.begin(), name.end(), name.begin(),
      [](unsigned char _c) { return std::tolower(_c); });

  for (const char *software : {"llvmpipe", "lavapipe", "softpipe", "swrast",
                               "swiftshader", "software rasterizer"})
  {
    if (name.find(software) != std::string::npos)
      return true;
  }
  return false;
}

}
}
}
