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
#ifndef IGNITION_RENDERING_CUSTOMRENDERENGINE_CUSTOMRENDERENGINERENDERTYPES_HH_
#define IGNITION_RENDERING_CUSTOMRENDERENGINE_CUSTOMRENDERENGINERENDERTYPES_HH_

#include "ignition/rendering/base/BaseRenderTypes.hh"

namespace ignition
{
  namespace rendering
  {

    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
    class CustomRenderEngineCamera;
    class CustomRenderEngineScene;
    class CustomRenderEngineSensor;
    class CustomRenderEngineNode;
    class CustomRenderEngineObject;
    class CustomRenderEngineMaterial;
    class CustomRenderEngineRenderTarget;

    typedef shared_ptr<CustomRenderEngineCamera>               CustomRenderEngineCameraPtr;
    typedef shared_ptr<CustomRenderEngineScene>                CustomRenderEngineScenePtr;
    typedef BaseSceneStore<CustomRenderEngineScene>            CustomRenderEngineSceneStore;
    typedef shared_ptr<CustomRenderEngineSceneStore>           CustomRenderEngineSceneStorePtr;
    typedef shared_ptr<CustomRenderEngineSensor>               CustomRenderEngineSensorPtr;
    typedef shared_ptr<CustomRenderEngineNode>                 CustomRenderEngineNodePtr;
    typedef shared_ptr<CustomRenderEngineObject>               CustomRenderEngineObjectPtr;
    typedef shared_ptr<CustomRenderEngineMaterial>             CustomRenderEngineMaterialPtr;
    typedef BaseMaterialMap<CustomRenderEngineMaterial>        CustomRenderEngineMaterialMap;
    typedef shared_ptr<CustomRenderEngineMaterialMap>          CustomRenderEngineMaterialMapPtr;
    typedef BaseSensorStore<CustomRenderEngineSensor>          CustomRenderEngineSensorStore;
    typedef shared_ptr<CustomRenderEngineSensorStore>          CustomRenderEngineSensorStorePtr;
    typedef shared_ptr<CustomRenderEngineRenderTarget>         CustomRenderEngineRenderTargetPtr;

    }
  }
}
#endif
