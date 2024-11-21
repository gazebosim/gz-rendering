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
#ifndef GZ_RENDERING_FRUSTUMVISUAL_HH_
#define GZ_RENDERING_FRUSTUMVISUAL_HH_

#include <gz/math/Angle.hh>
#include <gz/math/AxisAlignedBox.hh>
#include <gz/math/Plane.hh>
#include <gz/math/Pose3.hh>
#include <gz/utils/ImplPtr.hh>
#include "gz/rendering/config.hh"
#include "gz/rendering/Visual.hh"
#include "gz/rendering/Object.hh"
#include "gz/rendering/RenderTypes.hh"
#include "gz/rendering/Marker.hh"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    //
    /// \brief Planes that define the boundaries of the frustum.
    enum GZ_RENDERING_VISIBLE FrustumVisualPlane
    {
      /// \brief Near plane
      FRUSTUM_PLANE_NEAR   = 0,

      /// \brief Far plane
      FRUSTUM_PLANE_FAR    = 1,

      /// \brief Left plane
      FRUSTUM_PLANE_LEFT   = 2,

      /// \brief Right plane
      FRUSTUM_PLANE_RIGHT  = 3,

      /// \brief Top plane
      FRUSTUM_PLANE_TOP    = 4,

      /// \brief Bottom plane
      FRUSTUM_PLANE_BOTTOM = 5
    };

    /// \brief Mathematical representation of a frustum and related functions.
    /// This is also known as a view frustum.
    class GZ_RENDERING_VISIBLE FrustumVisual : public virtual Visual
    {
      /// \brief Default constructor. With the following default values:
      ///
      /// * near: 0.0
      /// * far: 1.0
      /// * fov: 0.78539 radians (45 degrees)
      /// * aspect ratio: 1.0
      /// * pose: Pose3d::Zero
      protected: FrustumVisual();

      /// \brief Destructor
      public: virtual ~FrustumVisual();

      /// \brief Constructor
      /// \param[in] _near Near plane distance. This is the distance from
      /// the frustum's vertex to the closest plane
      /// \param[in] _far Far plane distance. This is the distance from the
      /// frustum's vertex to the farthest plane.
      /// \param[in] _fov Field of view. The field of view is the
      /// angle between the frustum's vertex and the edges of the near or far
      /// plane. This value represents the horizontal angle.
      /// \param[in] _aspectRatio The aspect ratio, which is the width divided
      /// by height of the near or far planes.
      /// \param[in] _pose Pose of the frustum, which is the vertex (top of
      /// the pyramid).
      /*protected: FrustumVisual(double _near,
                      double _far,
                      const math::Angle &_fov,
                      double _aspectRatio,
                      const gz::math::Pose3d &_pose = gz::math::Pose3d::Zero);*/

      public: virtual void Update() = 0;

      /// \brief Get the near distance. This is the distance from the
      /// frustum's vertex to the closest plane.
      /// \return Near distance.
      /// \sa SetNear
      public: virtual double Near() const = 0;

      /// \brief Set the near distance. This is the distance from the
      /// frustum's vertex to the closest plane.
      /// \param[in] _near Near distance.
      /// \sa Near
      public: virtual void SetNear(double _near) = 0;

      /// \brief Get the far distance. This is the distance from the
      /// frustum's vertex to the farthest plane.
      /// \return Far distance.
      /// \sa SetFar
      public: virtual double Far() const = 0;

      /// \brief Set the far distance. This is the distance from the
      /// frustum's vertex to the farthest plane.
      /// \param[in] _far Far distance.
      /// \sa Far
      public: virtual void SetFar(double _far) = 0;

      /// \brief Get the horizontal field of view. The field of view is the
      /// angle between the frustum's vertex and the edges of the near or far
      /// plane. This value represents the horizontal angle.
      /// \return The field of view.
      /// \sa SetFOV
      public: virtual gz::math::Angle FOV() const = 0;

      /// \brief Set the horizontal field of view. The field of view is the
      /// angle between the frustum's vertex and the edges of the near or far
      /// plane. This value represents the horizontal angle.
      /// \param[in] _fov The field of view.
      /// \sa FOV
      public: virtual void SetFOV(const gz::math::Angle &_fov) = 0;

      /// \brief Get the aspect ratio, which is the width divided by height
      /// of the near or far planes.
      /// \return The frustum's aspect ratio.
      /// \sa SetAspectRatio
      public: virtual double AspectRatio() const = 0;

      /// \brief Set the aspect ratio, which is the width divided by height
      /// of the near or far planes.
      /// \param[in] _aspectRatio The frustum's aspect ratio.
      /// \sa AspectRatio
      public: virtual void SetAspectRatio(double _aspectRatio) = 0;

      /// \brief Get a plane of the frustum.
      /// \param[in] _plane The plane to return.
      /// \return Plane of the frustum.
      public: virtual gz::math::Planed Plane(const FrustumVisualPlane _plane) const = 0;

      /// \brief Check if a box lies inside the pyramid frustum.
      /// \param[in] _b Box to check.
      /// \return True if the box is inside the pyramid frustum.
      //public: virtual bool Contains(const gz::math::AxisAlignedBox &_b) const = 0; //TO-DO

      /// \brief Check if a point lies inside the pyramid frustum.
      /// \param[in] _p Point to check.
      /// \return True if the point is inside the pyramid frustum.
      //public: virtual bool Contains(const gz::math::Vector3d &_p) const = 0; //TO-DO

      /// \brief Get the pose of the frustum
      /// \return Pose of the frustum
      /// \sa SetPose
      public: virtual gz::math::Pose3d Pose() const = 0;

      /// \brief Set the pose of the frustum
      /// \param[in] _pose Pose of the frustum, top vertex.
      /// \sa Pose
      public: virtual void SetPose(const gz::math::Pose3d &_pose) = 0;

      /// \brief Compute the planes of the frustum. This is called whenever
      /// a property of the frustum is changed.
      private: void ComputePlanes();

      /// \brief Private data pointer
      //GZ_UTILS_IMPL_PTR(dataPtr)
    };
    }
  }
}
#endif
