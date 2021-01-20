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

#ifndef IGNITION_RENDERING_OGRE2_OGREELLIPSOID_HH_
#define IGNITION_RENDERING_OGRE2_OGREELLIPSOID_HH_

#include <memory>
#include <vector>

#include "ignition/rendering/base/BaseEllipsoid.hh"
#include "ignition/rendering/ogre2/Ogre2Geometry.hh"
#include "ignition/rendering/ogre2/Ogre2Includes.hh"

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
      class Ogre2EllipsoidPrivate;

      /// \brief Ogre 2.x implementation of a Ellipsoid Visual.
      class IGNITION_RENDERING_OGRE2_VISIBLE Ogre2Ellipsoid
        : public BaseEllipsoid<Ogre2Geometry>
      {
        /// \brief Constructor
        protected: Ogre2Ellipsoid();

        /// \brief Destructor
        public: virtual ~Ogre2Ellipsoid();

        // Documentation inherited.
        public: virtual void Init() override;

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
        protected: virtual void SetMaterialImpl(Ogre2MaterialPtr _material);

        /// \brief Create the Ellipsoid Visual in ogre
        private: void Create();

        /// \brief Ellipsoid should only be created by scene.
        private: friend class Ogre2Scene;

        private: void calculateRing(
          size_t segments, float radius, float y, float dy,
          std::vector<ignition::math::Vector3d> & positions);

        /// \brief Private data class
        private: std::unique_ptr<Ogre2EllipsoidPrivate> dataPtr;
      };
    }
  }
}
#endif
