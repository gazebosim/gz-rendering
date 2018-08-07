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
#ifndef IGNITION_RENDERING_OGRE2_OGRE2STORAGE_HH_
#define IGNITION_RENDERING_OGRE2_OGRE2STORAGE_HH_

#include <memory>

#include "ignition/rendering/base/BaseStorage.hh"

#include "ignition/rendering/ogre2/Ogre2Node.hh"
#include "ignition/rendering/ogre2/Ogre2Sensor.hh"
#include "ignition/rendering/ogre2/Ogre2Scene.hh"

namespace ignition
{
  namespace rendering
  {
    template class BaseSceneStore<Ogre2Scene>;
    template class BaseNodeStore<Ogre2Node>;
    template class BaseSensorStore<Ogre2Sensor>;

    typedef BaseSceneStore<Ogre2Scene>       Ogre2SceneStore;
    typedef BaseNodeStore<Ogre2Node>         Ogre2NodeStore;
    typedef BaseSensorStore<Ogre2Sensor>     Ogre2SensorStore;

    typedef std::shared_ptr<Ogre2SceneStore>    Ogre2SceneStorePtr;
    typedef std::shared_ptr<Ogre2NodeStore>     Ogre2NodeStorePtr;
    typedef std::shared_ptr<Ogre2SensorStore>   Ogre2SensorStorePtr;
  }
}
#endif
