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
#ifndef IGNITION_RENDERING_LUXCOREENGINE_LUXCOREENGINEOBJECT_HH_
#define IGNITION_RENDERING_LUXCOREENGINE_LUXCOREENGINEOBJECT_HH_

#include "ignition/rendering/base/BaseObject.hh"
#include "ignition/rendering/base/BaseScene.hh"

#include "LuxCoreEngineRenderTypes.hh"

namespace ignition {
namespace rendering {
inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {

class LuxCoreEngineObject : public BaseObject {

  protected: LuxCoreEngineObject();
  
  public: virtual ~LuxCoreEngineObject();
  
  public: virtual ScenePtr Scene() const;
  
  protected: LuxCoreEngineScenePtr scene;
  
  private: friend class LuxCoreEngineScene;
};

} // namespace IGNITION_RENDERING_VERSION_NAMESPACE
} // namespace rendering
} // namespace ignition

#endif
