/*
 * Copyright (C) 2020 Open Source Robotics Foundation
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

#ifndef GZ_RENDERING_OGRE_NEXT_OGRELIDARVISUAL_HH_
#define GZ_RENDERING_OGRE_NEXT_OGRELIDARVISUAL_HH_

#include <memory>
#include <vector>
#include "gz/rendering/base/BaseLidarVisual.hh"
#include "gz/rendering/ogre_next/OgreNextVisual.hh"
#include "gz/rendering/ogre_next/OgreNextScene.hh"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    //
    // Forward declaration
    class OgreNextLidarVisualPrivate;

    /// \brief Ogre 2.x implementation of a Lidar Visual.
    class GZ_RENDERING_OGRE_NEXT_VISIBLE OgreNextLidarVisual
      : public BaseLidarVisual<OgreNextVisual>
    {
      /// \brief Constructor
      protected: OgreNextLidarVisual();

      /// \brief Destructor
      public: virtual ~OgreNextLidarVisual();

      // Documentation inherited.
      public: virtual void Init() override;

      // Documentation inherited.
      public: virtual void PreRender() override;

      // Documentation inherited.
      public: virtual void Destroy() override;

      // Documentation inherited
      public: virtual void Update() override;

      // Documentation inherited
      public: virtual void SetPoints(
              const std::vector<double> &_points) override;

      // Documentation inherited
      public: virtual void ClearPoints() override;

      // Documentation inherited
      public: virtual unsigned int PointCount() const override;

      // Documentation inherited
      public: virtual std::vector<double> Points() const override;

      /// \brief Create the Lidar Visual in ogre
      private: void Create();

      /// \brief Clear data stored by dynamiclines
      private: void ClearVisualData();

      // Documentation inherited
      public: virtual void SetVisible(bool _visible) override;

      /// \brief Lidar Visual should only be created by scene.
      private: friend class OgreNextScene;

      /// \brief Private data class
      private: std::unique_ptr<OgreNextLidarVisualPrivate> dataPtr;
    };
    }
  }
}
#endif
