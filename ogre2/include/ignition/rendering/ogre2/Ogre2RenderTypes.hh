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
#ifndef IGNITION_RENDERING_OGRE2_OGRE2RENDERTYPES_HH_
#define IGNITION_RENDERING_OGRE2_OGRE2RENDERTYPES_HH_

#include "ignition/rendering/config.hh"
#include "ignition/rendering/base/BaseRenderTypes.hh"

namespace ignition
{
  namespace rendering
  {
    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
    //
    class Ogre2Node;
    class Ogre2Object;
    class Ogre2RenderEngine;
    class Ogre2Scene;
    class Ogre2Sensor;

    typedef BaseSceneStore<Ogre2Scene>            Ogre2SceneStore;
    typedef BaseNodeStore<Ogre2Node>              Ogre2NodeStore;
    typedef shared_ptr<Ogre2Node>                 Ogre2NodePtr;
    typedef shared_ptr<Ogre2Object>               Ogre2ObjectPtr;
    typedef shared_ptr<Ogre2RenderEngine>         Ogre2RenderEnginePtr;
    typedef shared_ptr<Ogre2Scene>                Ogre2ScenePtr;

    typedef shared_ptr<Ogre2SceneStore>           Ogre2SceneStorePtr;
    typedef shared_ptr<Ogre2NodeStore>            Ogre2NodeStorePtr;
    }
  }
}
#endif
