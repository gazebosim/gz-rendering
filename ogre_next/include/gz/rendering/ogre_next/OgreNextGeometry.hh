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
#ifndef GZ_RENDERING_OGRE_NEXT_OGRE_NEXTGEOMETRY_HH_
#define GZ_RENDERING_OGRE_NEXT_OGRE_NEXTGEOMETRY_HH_

#include "gz/rendering/base/BaseGeometry.hh"
#include "gz/rendering/ogre_next/OgreNextObject.hh"

namespace Ogre
{
  class MovableObject;
}

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    //
    /// \brief OgreNext.x implementation of the geometry class
    class GZ_RENDERING_OGRE_NEXT_VISIBLE OgreNextGeometry :
      public BaseGeometry<OgreNextObject>
    {
      /// \brief Constructor
      protected: OgreNextGeometry();

      /// \brief Destructor
      public: virtual ~OgreNextGeometry();

      // Documentation inherited.
      public: virtual bool HasParent() const override;

      // Documentation inherited.
      public: virtual VisualPtr Parent() const override;

      /// \brief Get the ogre object representing this geometry
      /// \return Pointer to an ogre movable object
      public: virtual Ogre::MovableObject *OgreObject() const = 0;

      /// \brief Set the parent of this ogre geometry
      /// \param[in] _parent Parent visual
      protected: virtual void SetParent(OgreNextVisualPtr _parent);

      /// \brief Parent visual
      protected: OgreNextVisualPtr parent;

      /// \brief Make ogre_next visual our friend so it can it can access
      /// function for setting the parent of this geometry
      private: friend class OgreNextVisual;
    };
    }
  }
}
#endif
