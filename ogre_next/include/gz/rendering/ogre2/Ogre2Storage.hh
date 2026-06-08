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

#ifndef GZ_RENDERING_OGRE2STORAGE_HH_
#define GZ_RENDERING_OGRE2STORAGE_HH_

#include "gz/rendering/ogre_next/OgreNextStorage.hh"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {

    using Ogre2GeometryStore = OgreNextGeometryStore;
    using Ogre2LightStore = OgreNextLightStore;
    using Ogre2NodeStore = OgreNextNodeStore;
    using Ogre2SceneStore = OgreNextSceneStore;
    using Ogre2SensorStore = OgreNextSensorStore;
    using Ogre2SubMeshStore = OgreNextSubMeshStore;
    using Ogre2VisualStore = OgreNextVisualStore;

    using Ogre2GeometryStorePtr = OgreNextGeometryStorePtr;
    using Ogre2LightStorePtr = OgreNextLightStorePtr;
    using Ogre2NodeStorePtr = OgreNextNodeStorePtr;
    using Ogre2SceneStorePtr = OgreNextSceneStorePtr;
    using Ogre2SensorStorePtr = OgreNextSensorStorePtr;
    using Ogre2SubMeshStorePtr = OgreNextSubMeshStorePtr;
    using Ogre2VisualStorePtr = OgreNextVisualStorePtr;
    }
  }
}
#endif
