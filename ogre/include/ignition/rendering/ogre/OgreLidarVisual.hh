/*
 * Copyright (C) 2019 Open Source Robotics Foundation
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

#ifndef IGNITION_RENDERING_OGRE_OGRELIDARVISUAL_HH_
#define IGNITION_RENDERING_OGRE_OGRELIDARVISUAL_HH_

#include <memory>
#include "ignition/rendering/base/BaseLidarVisual.hh"
#include "ignition/rendering/ogre/OgreGeometry.hh"
#include "ignition/rendering/ogre/OgreIncludes.hh"

namespace ignition
{
  namespace rendering
  {
    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
    //
    // Forward declaration
    class OgreLidarVisualPrivate;

    /// \brief Ogre implementation of a LidarVisual geometry.
    class IGNITION_RENDERING_OGRE_VISIBLE OgreLidarVisual
      : public BaseLidarVisual<OgreGeometry>
    {
      /// \brief Constructor
      protected: OgreLidarVisual();

      /// \brief Destructor
      public: virtual ~OgreLidarVisual();

      // Documentation inherited.
      public: virtual void Init() override;

      // Documentation inherited.
      public: virtual void PreRender() override;

      // Documentation inherited.
      public: virtual void Destroy() override;

      // Documentation inherited.
      public: virtual Ogre::MovableObject *OgreObject() const override;

      // Documentation inherited.
      public: virtual MaterialPtr Material() const override;

      // Documentation inherited.
      public: virtual void SetMaterial(
                           MaterialPtr _material, bool _unique) override;

      // Documentation inherited
      public: virtual void SetPoint(unsigned int _index,
                           const ignition::math::Vector3d &_value) override;

      // Documentation inherited
      public: virtual void AddPoint(const ignition::math::Vector3d &_pt,
                           const ignition::math::Color &_color) override;

      // Documentation inherited
      public: virtual void ClearPoints() override;

      // Documentation inherited
      public: virtual void SetType(const LidarVisualType _lidarVisualType) override;

      // Documentation inherited
      public: virtual LidarVisualType Type() const override;

      /// \brief Create the LidarVisual geometry in ogre
      private: void Create();

      /// \brief LidarVisual should only be created by scene.
      private: friend class OgreScene;

      /// \brief Private data class
      private: std::unique_ptr<OgreLidarVisualPrivate> dataPtr;
    };
    }
  }
}
#endif
