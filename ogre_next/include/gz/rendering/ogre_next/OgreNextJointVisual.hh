/*
 * Copyright (C) 2021 Open Source Robotics Foundation
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
#ifndef GZ_RENDERING_OGRE_NEXT_OGRE_NEXTJOINTVISUAL_HH_
#define GZ_RENDERING_OGRE_NEXT_OGRE_NEXTJOINTVISUAL_HH_

#include "gz/rendering/base/BaseJointVisual.hh"
#include "gz/rendering/ogre_next/OgreNextVisual.hh"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    //
    class GZ_RENDERING_OGRE_NEXT_VISIBLE OgreNextJointVisual :
      public BaseJointVisual<OgreNextVisual>
    {
      /// \brief Constructor
      protected: OgreNextJointVisual();

      /// \brief Destructor
      public: virtual ~OgreNextJointVisual();

      /// \brief Only scene can instantiate this class
      private: friend class OgreNextScene;
    };
    }
  }
}
#endif
