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
#ifndef GZ_RENDERING_OGRE_NEXT_OGRE_NEXTRAYQUERY_HH_
#define GZ_RENDERING_OGRE_NEXT_OGRE_NEXTRAYQUERY_HH_

#include <memory>

#include "gz/rendering/base/BaseRayQuery.hh"
#include "gz/rendering/ogre_next/OgreNextObject.hh"
#include "gz/rendering/ogre_next/OgreNextRenderTypes.hh"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    //
    // forward declaration
    class OgreNextRayQueryPrivate;

    /// \class OgreNextRayQuery OgreNextRayQuery.hh
    /// gz/rendering/ogre_next/OgreNextRayQuery.hh
    /// \brief A Ray Query class used for computing ray object intersections
    class GZ_RENDERING_OGRE_NEXT_VISIBLE OgreNextRayQuery :
        public BaseRayQuery<OgreNextObject>
    {
      /// \brief Constructor
      protected: OgreNextRayQuery();

      /// \brief Destructor
      public: virtual ~OgreNextRayQuery() override;

      // Documentation inherited
      public: virtual void SetFromCamera(const CameraPtr &_camera,
                const math::Vector2d &_coord) override;

      // Documentation inherited
      public: void SetFromCamera( const WideAngleCameraPtr &_camera,
                                  uint32_t _faceIdx,
                                  const math::Vector2d &_coord) override;

      // Documentation inherited
      public: void SetPreferGpu(bool _preferGpu) override;

      // Documentation inherited
      public: bool UsesGpu() const override;

      // Documentation inherited
      public: virtual RayQueryResult ClosestPoint(
            bool _forceSceneUpdate = true) override;

      /// \brief Get closest point by selection buffer.
      /// This is executed on the GPU.
      private: RayQueryResult ClosestPointBySelectionBuffer();

      /// \brief Get closest point by ray triangle intersection test.
      /// This is executed on the CPU.
      private: RayQueryResult ClosestPointByIntersection(
            bool _forceSceneUpdate);

      /// \brief Private data pointer
      private: std::unique_ptr<OgreNextRayQueryPrivate> dataPtr;

      /// \brief Pointer to friend scene class for creating ray query
      private: friend class OgreNextScene;
    };
    }
  }
}
#endif
