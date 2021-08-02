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
#ifndef IGNITION_RENDERING_OPTIX_OPTIXRENDERTYPES_HH_
#define IGNITION_RENDERING_OPTIX_OPTIXRENDERTYPES_HH_

#include <luxrays/utils/properties.h>
#include "ignition/rendering/base/BaseRenderTypes.hh"

namespace ignition
{
  namespace rendering
  {

    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
    class LuxCoreCamera;
    class LuxCoreScene;
    class LuxCoreSensor;
    class LuxCoreNode;
    class LuxCoreObject;
    class LuxCoreMaterial;
    class LuxCoreRenderTarget;

    typedef shared_ptr<LuxCoreCamera>               LuxCoreCameraPtr;
    typedef shared_ptr<LuxCoreScene>                LuxCoreScenePtr;
    typedef BaseSceneStore<LuxCoreScene>            LuxCoreSceneStore;
    typedef shared_ptr<LuxCoreSceneStore>           LuxCoreSceneStorePtr;
    typedef shared_ptr<luxrays::Properties>         LuxCorePropsPtr;    
    typedef shared_ptr<LuxCoreSensor>               LuxCoreSensorPtr;
    typedef shared_ptr<LuxCoreNode>                 LuxCoreNodePtr;
    typedef shared_ptr<LuxCoreObject>               LuxCoreObjectPtr;
    typedef shared_ptr<LuxCoreMaterial>             LuxCoreMaterialPtr;
    typedef BaseMaterialMap<LuxCoreMaterial>        LuxCoreMaterialMap;
    typedef shared_ptr<LuxCoreMaterialMap>          LuxCoreMaterialMapPtr;
    typedef BaseSensorStore<LuxCoreSensor>          LuxCoreSensorStore;
    typedef shared_ptr<LuxCoreSensorStore>          LuxCoreSensorStorePtr;
    typedef shared_ptr<LuxCoreRenderTarget>         LuxCoreRenderTargetPtr;

    }
  }
}
#endif
