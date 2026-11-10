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

namespace
{
/// \brief Time spent blocked on the GPU by the current thread
thread_local std::chrono::nanoseconds threadGpuWaitTime{0};
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

}
}
}
