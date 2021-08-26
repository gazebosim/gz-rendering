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
#ifndef IGNITION_RENDERING_LENSTRACEENGINE_LENSTRACEENGINERENDERTYPES_HH_
#define IGNITION_RENDERING_LENSTRACEENGINE_LENSTRACEENGINERENDERTYPES_HH_

#include "ignition/rendering/base/BaseRenderTypes.hh"

namespace ignition
{
  namespace rendering
  {

    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
    class LensTraceEngineCamera;
    class LensTraceEngineScene;
    class LensTraceEngineSensor;
    class LensTraceEngineNode;
    class LensTraceEngineObject;
    class LensTraceEngineMaterial;
    class LensTraceEngineRenderTarget;

    typedef shared_ptr<LensTraceEngineCamera>               LensTraceEngineCameraPtr;
    typedef shared_ptr<LensTraceEngineScene>                LensTraceEngineScenePtr;
    typedef BaseSceneStore<LensTraceEngineScene>            LensTraceEngineSceneStore;
    typedef shared_ptr<LensTraceEngineSceneStore>           LensTraceEngineSceneStorePtr;
    typedef shared_ptr<LensTraceEngineSensor>               LensTraceEngineSensorPtr;
    typedef shared_ptr<LensTraceEngineNode>                 LensTraceEngineNodePtr;
    typedef shared_ptr<LensTraceEngineObject>               LensTraceEngineObjectPtr;
    typedef shared_ptr<LensTraceEngineMaterial>             LensTraceEngineMaterialPtr;
    typedef BaseMaterialMap<LensTraceEngineMaterial>        LensTraceEngineMaterialMap;
    typedef shared_ptr<LensTraceEngineMaterialMap>          LensTraceEngineMaterialMapPtr;
    typedef BaseSensorStore<LensTraceEngineSensor>          LensTraceEngineSensorStore;
    typedef shared_ptr<LensTraceEngineSensorStore>          LensTraceEngineSensorStorePtr;
    typedef shared_ptr<LensTraceEngineRenderTarget>         LensTraceEngineRenderTargetPtr;

    }
  }
}
#endif
