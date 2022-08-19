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
#ifndef IGNITION_RENDERING_BASE_BASERENDERTYPES_HH_
#define IGNITION_RENDERING_BASE_BASERENDERTYPES_HH_

#include <memory>

#include "ignition/rendering/RenderTypes.hh"

namespace ignition
{
  namespace rendering
  {
    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
    //
    class BaseNodeCompositeStore;
    typedef shared_ptr<BaseNodeCompositeStore>   BaseNodeCompositeStorePtr;

    class BaseObject;
    typedef std::shared_ptr<BaseObject> BaseObjectPtr;

    template <class T> class BaseSceneStore;
    template <class T> class BaseNodeStore;
    template <class T> class BaseLightStore;
    template <class T> class BaseSensorStore;
    template <class T> class BaseVisualStore;
    template <class T> class BaseGeometryStore;
    template <class T> class BaseSubMeshStore;
    template <class T> class BaseMaterialMap;
    }
  }
}
#endif
