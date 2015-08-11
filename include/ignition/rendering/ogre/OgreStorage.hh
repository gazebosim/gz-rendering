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
#ifndef _IGNITION_RENDERING_OGRESTORAGE_HH_
#define _IGNITION_RENDERING_OGRESTORAGE_HH_

#include "ignition/rendering/base/BaseStorage.hh"
#include "ignition/rendering/ogre/ogre.hh"

namespace ignition
{
  namespace rendering
  {
    template class BaseSceneStore<OgreScene>;
    template class BaseNodeStore<OgreNode>;
    template class BaseLightStore<OgreLight>;
    template class BaseSensorStore<OgreSensor>;
    template class BaseVisualStore<OgreVisual>;
    template class BaseGeometryStore<OgreGeometry>;
    template class BaseSubMeshStore<OgreSubMesh>;
    template class BaseMaterialMap<OgreMaterial>;

    typedef BaseSceneStore<OgreScene>       OgreSceneStore;
    typedef BaseNodeStore<OgreNode>         OgreNodeStore;
    typedef BaseLightStore<OgreLight>       OgreLightStore;
    typedef BaseSensorStore<OgreSensor>     OgreSensorStore;
    typedef BaseVisualStore<OgreVisual>     OgreVisualStore;
    typedef BaseGeometryStore<OgreGeometry> OgreGeometryStore;
    typedef BaseSubMeshStore<OgreSubMesh>   OgreSubMeshStore;
    typedef BaseMaterialMap<OgreMaterial>   OgreMaterialMap;

    typedef boost::shared_ptr<OgreSceneStore>    OgreSceneStorePtr;
    typedef boost::shared_ptr<OgreNodeStore>     OgreNodeStorePtr;
    typedef boost::shared_ptr<OgreLightStore>    OgreLightStorePtr;
    typedef boost::shared_ptr<OgreSensorStore>   OgreSensorStorePtr;
    typedef boost::shared_ptr<OgreVisualStore>   OgreVisualStorePtr;
    typedef boost::shared_ptr<OgreGeometryStore> OgreGeometryStorePtr;
    typedef boost::shared_ptr<OgreSubMeshStore>  OgreSubMeshStorePtr;
    typedef boost::shared_ptr<OgreMaterialMap>   OgreMaterialMapPtr;
  }
}
#endif
