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

#ifndef IGNITION_RENDERING_OGRE_OGREELLIPSOID_HH_
#define IGNITION_RENDERING_OGRE_OGREELLIPSOID_HH_

#include <memory>
#include "ignition/rendering/base/BaseEllipsoid.hh"
#include "ignition/rendering/ogre/OgreGeometry.hh"
#include "ignition/rendering/ogre/OgreIncludes.hh"

namespace Ogre
{
  class MovableObject;
}

namespace ignition
{
  namespace rendering
  {
    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
      // Forward declaration
      class OgreEllipsoidPrivate;

      /// \brief Ogre 1.x implementation of a Ellipsoid Visual.
      class IGNITION_RENDERING_OGRE_VISIBLE OgreEllipsoid
        : public BaseEllipsoid<OgreGeometry>
      {
        /// \brief Constructor
        protected: OgreEllipsoid();

        /// \brief Destructor
        public: virtual ~OgreEllipsoid();

        // Documentation inherited.
        public: virtual void Init() override;

        // Documentation inherited.
        public: virtual void Destroy() override;

        // Documentation inherited.
        public: virtual Ogre::MovableObject *OgreObject() const;

        // Documentation inherited.
        public: virtual void PreRender() override;

        // Documentation inherited.
        public: virtual MaterialPtr Material() const;

        // Documentation inherited.
        public: virtual void SetMaterial(MaterialPtr _material, bool _unique);

        /// \brief Set material to grid geometry.
        /// \param[in] _material Ogre material.
        protected: virtual void SetMaterialImpl(OgreMaterialPtr _material);

        /// \brief Create the Ellipsoid Visual in ogre
        private: void Create();

        /// \brief Ellipsoid should only be created by scene.
        private: friend class OgreScene;

        /// \brief Private data class
        private: std::unique_ptr<OgreEllipsoidPrivate> dataPtr;
      };
    }
  }
}
#endif
