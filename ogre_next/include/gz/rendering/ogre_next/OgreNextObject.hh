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
#ifndef GZ_RENDERING_OGRE_NEXT_OGRE_NEXTOBJECT_HH_
#define GZ_RENDERING_OGRE_NEXT_OGRE_NEXTOBJECT_HH_

#include "gz/rendering/config.hh"
#include "gz/rendering/base/BaseObject.hh"
#include "gz/rendering/ogre_next/OgreNextRenderTypes.hh"
#include "gz/rendering/ogre_next/Export.hh"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    //
    /// \brief OgreNext.x implementation of the Object class
    class GZ_RENDERING_OGRE_NEXT_VISIBLE OgreNextObject :
      public BaseObject
    {
      /// \brief Constructor
      protected: OgreNextObject();

      /// \brief Destructor
      public: virtual ~OgreNextObject();

      // Documentation inherited
      public: virtual ScenePtr Scene() const override;

      /// \brief Pointer to the ogre scene
      protected: OgreNextScenePtr scene;

      /// \brief Make ogre scene our friend so it is able to create objects
      private: friend class OgreNextScene;
    };
    }
  }
}
#endif
