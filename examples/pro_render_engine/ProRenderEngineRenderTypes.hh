/*
 * Copyright (C) 2015 Open Source Robotics Foundation
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
#ifndef IGNITION_RENDERING_PRORENDERENGINE_PRORENDERENGINERENDERTYPES_HH_
#define IGNITION_RENDERING_PRORENDERENGINE_PRORENDERENGINERENDERTYPES_HH_

#include "ignition/rendering/base/BaseRenderTypes.hh"

namespace ignition {
namespace rendering {

inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
class ProRenderEngineCamera;
class ProRenderEngineScene;
class ProRenderEngineSensor;
class ProRenderEngineNode;
class ProRenderEngineObject;
class ProRenderEngineMaterial;
class ProRenderEngineRenderTarget;

typedef shared_ptr<ProRenderEngineCamera> ProRenderEngineCameraPtr;
typedef shared_ptr<ProRenderEngineScene> ProRenderEngineScenePtr;
typedef BaseSceneStore<ProRenderEngineScene> ProRenderEngineSceneStore;
typedef shared_ptr<ProRenderEngineSceneStore> ProRenderEngineSceneStorePtr;
typedef shared_ptr<ProRenderEngineSensor> ProRenderEngineSensorPtr;
typedef shared_ptr<ProRenderEngineNode> ProRenderEngineNodePtr;
typedef shared_ptr<ProRenderEngineObject> ProRenderEngineObjectPtr;
typedef shared_ptr<ProRenderEngineMaterial> ProRenderEngineMaterialPtr;
typedef BaseMaterialMap<ProRenderEngineMaterial> ProRenderEngineMaterialMap;
typedef shared_ptr<ProRenderEngineMaterialMap> ProRenderEngineMaterialMapPtr;
typedef BaseSensorStore<ProRenderEngineSensor> ProRenderEngineSensorStore;
typedef shared_ptr<ProRenderEngineSensorStore> ProRenderEngineSensorStorePtr;
typedef shared_ptr<ProRenderEngineRenderTarget> ProRenderEngineRenderTargetPtr;

} // namespace IGNITION_RENDERING_VERSION_NAMESPACE
} // namespace rendering
} // namespace ignition
#endif
