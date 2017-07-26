/*
 * Copyright (C) 2017 Open Source Robotics Foundation
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
#ifndef IGNITION_RENDERING_BASE_BASERAYQUERY_HH_
#define IGNITION_RENDERING_BASE_BASERAYQUERY_HH_

#include "ignition/rendering/RayQuery.hh"
#include "ignition/rendering/Scene.hh"

namespace ignition
{
  namespace rendering
  {
    /// \class BaseRayQuery BaseRayQuery.hh
    /// ignition/rendering/base/BaseRayQuery.hh
    /// \brief A Ray Query class used for computing ray object intersections
    template <class T>
    class IGNITION_VISIBLE BaseRayQuery :
        public virtual RayQuery,
        public T
    {
      /// \brief Constructor
      protected: BaseRayQuery();

      /// \brief Deconstructor
      public: virtual ~BaseRayQuery();

      /// \brief Set ray origin
      /// \param[in] _origin Ray Origin
      public: virtual void SetOrigin(const math::Vector3d &_origin);

      /// \brief Set ray direction
      /// \param[in] _origin Ray Origin
      public: virtual void SetDirection(const math::Vector3d &_dir);

      /// \brief Create the ray query from camera
      /// \param[in] _camera Camera to construct ray
      /// \param[in] _coord normalized device coords [-1, +1]
      public: virtual void SetFromCamera(const CameraPtr &_camera,
                const math::Vector2d &_coord);

      /// \brief Compute intersections
      /// \param[out] A vector of intersection results
      /// \return True if results are not empty
      public: virtual RayQueryResult ClosestPoint();

      /// \brief Ray origin
      protected: ignition::math::Vector3d origin;

      /// \brief Ray direction
      protected: ignition::math::Vector3d direction;
    };

    //////////////////////////////////////////////////
    template <class T>
    BaseRayQuery<T>::BaseRayQuery()
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    BaseRayQuery<T>::~BaseRayQuery()
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseRayQuery<T>::SetOrigin(const math::Vector3d &_origin)
    {
      this->origin = _origin;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseRayQuery<T>::SetDirection(const math::Vector3d &_dir)
    {
      this->direction = _dir;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseRayQuery<T>::SetFromCamera(const CameraPtr &_camera,
        const ignition::math::Vector2d &_coord)
    {
    //  math::Matrix4d projectionMatrix = _camera->ProjectionMatrix();
      math::Matrix4d projectionMatrix;
      math::Matrix4d viewMatrix = math::Matrix4d(_camera->WorldPose());
      math::Vector3d start(_coord.X(), _coord.Y(), -1.0);
      math::Vector3d end(_coord.X(), _coord.Y(), 0.0);
      math::Matrix4d viewProjInv = (projectionMatrix * viewMatrix).Inverse();
      start = viewProjInv * start;
      end = viewProjInv * end;

      this->origin =  start;
      this->direction = (end - start).Normalize();
    }

    //////////////////////////////////////////////////
    template <class T>
    RayQueryResult BaseRayQuery<T>::ClosestPoint()
    {
      // TODO implement a generic ray query here?
      RayQueryResult result;
      return result;
    }

  }
}
#endif
