/*
 * Copyright (C) 2022 Open Source Robotics Foundation
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
#ifndef IGNITION_RENDERING_LUXCOREENGINE_LUXCOREENGINERENDERTARGET_HH_
#define IGNITION_RENDERING_LUXCOREENGINE_LUXCOREENGINERENDERTARGET_HH_

#include "ignition/rendering/base/BaseRenderTarget.hh"

#include "LuxCoreEngineObject.hh"
#include "LuxCoreEngineRenderTypes.hh"

namespace ignition {
namespace rendering {
inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {

class LuxCoreEngineRenderTarget
    : public virtual BaseRenderTarget<LuxCoreEngineObject> {

  protected: LuxCoreEngineRenderTarget();
  
  public: virtual ~LuxCoreEngineRenderTarget();
  
  public: virtual void Copy(Image &_image) const;
  
  public: void *HostDataBuffer();
  
  public: void ResizeHostDataBuffer(unsigned int size);
  
  protected: unsigned int MemorySize() const;
  
  protected: virtual void RebuildImpl();
  
  protected: void *hostDataBuffer;
  
  private: friend class LuxCoreEngineCamera;
};

} // namespace IGNITION_RENDERING_VERSION_NAMESPACE
} // namespace rendering
} // namespace ignition
#endif