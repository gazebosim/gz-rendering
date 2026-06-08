/*
 * Copyright (C) 2018 Open Source Robotics Foundation
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
#ifndef GZ_RENDERING_OGRE_NEXT_OGRE_NEXTSTORAGE_HH_
#define GZ_RENDERING_OGRE_NEXT_OGRE_NEXTSTORAGE_HH_

#include <memory>

#include "gz/rendering/base/BaseStorage.hh"

#include "gz/rendering/ogre_next/OgreNextLight.hh"
#include "gz/rendering/ogre_next/OgreNextMesh.hh"
#include "gz/rendering/ogre_next/OgreNextGeometry.hh"
#include "gz/rendering/ogre_next/OgreNextNode.hh"
#include "gz/rendering/ogre_next/OgreNextSensor.hh"
#include "gz/rendering/ogre_next/OgreNextScene.hh"
#include "gz/rendering/ogre_next/OgreNextVisual.hh"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {

// armhf failed to build with this code. It can not be removed for the rest
// of arches to keep ABI but should be removed in major versions unreleased
// see https://github.com/gazebosim/gz-rendering/pull/457
#ifndef __ARM_PCS_VFP
    template class BaseGeometryStore<OgreNextGeometry>;
    template class BaseLightStore<OgreNextLight>;
    template class BaseNodeStore<OgreNextNode>;
    template class BaseSceneStore<OgreNextScene>;
    template class BaseSensorStore<OgreNextSensor>;
    template class BaseSubMeshStore<OgreNextSubMesh>;
    template class BaseVisualStore<OgreNextVisual>;
#endif

    typedef BaseGeometryStore<OgreNextGeometry>    OgreNextGeometryStore;
    typedef BaseLightStore<OgreNextLight>          OgreNextLightStore;
    typedef BaseNodeStore<OgreNextNode>            OgreNextNodeStore;
    typedef BaseSceneStore<OgreNextScene>          OgreNextSceneStore;
    typedef BaseSensorStore<OgreNextSensor>        OgreNextSensorStore;
    typedef BaseSubMeshStore<OgreNextSubMesh>      OgreNextSubMeshStore;
    typedef BaseVisualStore<OgreNextVisual>        OgreNextVisualStore;

    typedef std::shared_ptr<OgreNextGeometryStore> OgreNextGeometryStorePtr;
    typedef std::shared_ptr<OgreNextLightStore>    OgreNextLightStorePtr;
    typedef std::shared_ptr<OgreNextNodeStore>     OgreNextNodeStorePtr;
    typedef std::shared_ptr<OgreNextSceneStore>    OgreNextSceneStorePtr;
    typedef std::shared_ptr<OgreNextSensorStore>   OgreNextSensorStorePtr;
    typedef std::shared_ptr<OgreNextSubMeshStore>  OgreNextSubMeshStorePtr;
    typedef std::shared_ptr<OgreNextVisualStore>   OgreNextVisualStorePtr;
    }
  }
}
#endif
