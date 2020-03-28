/*
 * Copyright (C) 2015 Open Source Robotics Foundation
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
#ifndef IGNITION_RENDERING_OGRE_OGREVISUAL_HH_
#define IGNITION_RENDERING_OGRE_OGREVISUAL_HH_

#include <vector>

#include "ignition/rendering/base/BaseVisual.hh"
#include "ignition/rendering/ogre/OgreNode.hh"
#include "ignition/rendering/ogre/OgreRenderTypes.hh"

namespace ignition
{
  namespace rendering
  {
    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
    //
    class IGNITION_RENDERING_OGRE_VISIBLE OgreVisual :
      public BaseVisual<OgreNode>
    {
      protected: OgreVisual();

      public: virtual ~OgreVisual();

      // Documentation inherited.
      public: virtual void SetVisible(bool _visible) override;

      // Documentation inherited.
      public: virtual void SetVisibilityFlags(uint32_t _flags) override;

      // Documentation inherited.
      public: virtual ignition::math::AxisAlignedBox LocalBoundingBox()
              const override;

      // Documentation inherited.
      public: virtual ignition::math::AxisAlignedBox BoundingBox()
              const override;

      /// \brief Recursively loop through this visual's children
      /// to obtain bounding box.
      /// \param[in,out] _box The bounding box.
      /// \param[in] _local A flag indicating if the local bounding box is to
      /// be calculated.
      /// \param[in] _pose The top level visual pose
      protected: virtual void BoundsHelper(
                     ignition::math::AxisAlignedBox &_box, bool _local,
                     const ignition::math::Pose3d &_pose) const;

      /// \brief Wrapper function for BoundsHelper to reduce redundant
      /// world pose access
      /// \param[in,out] _box The bounding box.
      /// \param[in] _local A flag indicating if the local bounding box is to
      /// be calculated.
      protected: virtual void BoundsHelper(
                     ignition::math::AxisAlignedBox &_box, bool _local) const;

      /// \brief Transform a bounding box to the world space.
      /// \param[in] _bbox Input bounding box.
      /// \param[in] _worldPose Pose used to transform the bounding box.
      /// \param[out] _vertices Vertices of the transformed bounding box in
      /// world coordinates.
      protected: virtual void Transform(
                     const ignition::math::AxisAlignedBox &_bbox,
                     const ignition::math::Pose3d &_worldPose,
                     std::vector<ignition::math::Vector3d> &_vertices) const;

      /// \brief Get the minimum and maximum values of a list of vertices.
      /// \param[in] _vertices A list of input vertices.
      /// \param[out] _min Minimum x, y, z values.
      /// \param[out] _max Maximum x, y, z values.
      protected: virtual void MinMax(
                     const std::vector<ignition::math::Vector3d> &_vertices,
                     ignition::math::Vector3d &_min,
                     ignition::math::Vector3d &_max) const;

      protected: virtual GeometryStorePtr Geometries() const override;

      // Documentation inherited.
      protected: virtual bool AttachGeometry(GeometryPtr _geometry) override;

      // Documentation inherited.
      protected: virtual bool DetachGeometry(GeometryPtr _geometry) override;

      // Documentation inherited.
      protected: virtual void Init() override;

      protected: OgreGeometryStorePtr geometries;

      private: OgreVisualPtr SharedThis();

      private: friend class OgreScene;
    };
    }
  }
}
#endif
