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
#ifndef IGNITION_RENDERING_UTILS_HH_
#define IGNITION_RENDERING_UTILS_HH_

#include <vector>

#include <ignition/math/Helpers.hh>
#include <ignition/math/AxisAlignedBox.hh>
#include <ignition/math/Vector2.hh>
#include <ignition/math/Vector3.hh>
#include <ignition/math/Pose3.hh>

#include "ignition/rendering/Camera.hh"
#include "ignition/rendering/config.hh"
#include "ignition/rendering/Export.hh"
#include "ignition/rendering/RayQuery.hh"


namespace ignition
{
  /// \brief Rendering classes and function useful in robot applications.
  namespace rendering
  {
    // Inline bracket to help doxygen filtering.
    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
    //
    /// \brief Retrieve the first point on a surface in the 3D scene hit by a
    /// ray cast from the given 2D screen coordinates.
    /// \param[in] _screenPos 2D coordinates on the screen, in pixels.
    /// \param[in] _camera User camera
    /// \param[in] _rayQuery Ray query for mouse clicks
    /// \param[in] _maxDistance maximum distance to check the collision
    /// \return 3D coordinates of a point in the 3D scene.
    IGNITION_RENDERING_VISIBLE
    math::Vector3d screenToScene(
        const math::Vector2i &_screenPos,
        const CameraPtr &_camera,
        const RayQueryPtr &_rayQuery,
        float _maxDistance = 10.0);

    /// \brief Retrieve the first point on a surface in the 3D scene hit by a
    /// ray cast from the given 2D screen coordinates.
    /// \param[in] _screenPos 2D coordinates on the screen, in pixels.
    /// \param[in] _camera User camera
    /// \param[in] _rayQuery Ray query for mouse clicks
    /// \param[inout] _rayResult Ray query result
    /// \param[in] _maxDistance maximum distance to check the collision
    /// \return 3D coordinates of a point in the 3D scene.
    IGNITION_RENDERING_VISIBLE
    math::Vector3d screenToScene(
        const math::Vector2i &_screenPos,
        const CameraPtr &_camera,
        const RayQueryPtr &_rayQuery,
        RayQueryResult &_rayResult,
        float _maxDistance = 10.0);

    /// \brief Retrieve the point on a plane at z = 0 in the 3D scene hit by a
    /// ray cast from the given 2D screen coordinates.
    /// \param[in] _screenPos 2D coordinates on the screen, in pixels.
    /// \param[in] _camera User camera
    /// \param[in] _rayQuery Ray query for mouse clicks
    /// \param[in] _offset Offset along the plane normal
    /// \return 3D coordinates of a point in the 3D scene.
    IGNITION_RENDERING_VISIBLE
    math::Vector3d screenToPlane(
      const math::Vector2i &_screenPos,
      const CameraPtr &_camera,
      const RayQueryPtr &_rayQuery,
      const float _offset = 0.0);

    /// \brief Get the screen scaling factor.
    /// \return The screen scaling factor.
    IGNITION_RENDERING_VISIBLE
    float screenScalingFactor();

    /// \brief Transform a bounding box.
    /// \param[in] _box The bounding box.
    /// \param[in] _pose Pose used to transform the bounding box.
    /// \return Vertices of the transformed bounding box in
    /// world coordinates.
    IGNITION_RENDERING_VISIBLE
    ignition::math::AxisAlignedBox transformAxisAlignedBox(
        const ignition::math::AxisAlignedBox &_box,
        const ignition::math::Pose3d &_pose);
    }
  }
}
#endif
