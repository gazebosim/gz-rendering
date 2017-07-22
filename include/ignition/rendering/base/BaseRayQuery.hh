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

namespace ignition
{
  namespace rendering
  {
    /// \class BaseRayQuery BaseRayQuery.hh ignition/rendering/base/BaseRayQuery.hh
    /// \brief A Ray Query class used for computing ray object intersections
    class IGNITION_VISIBLE BaseRayQuery
        : public virtual RayQuery
    {
      /// \brief Constructor
      public: BaseRayQuery(const math::Vector3d &_origin,
                const math::Vector3d &_dir);

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
      public: virtual void SetFromCamera(CameraPtr _camera,
                const math::Vector2d &_coord);

      /// \brief Compute intersections
      /// \param[] Scene where the ray query will be executed
      /// \param[out] A vector of intersection results
      /// \return True if results are not empty
      public: virtual bool Intersect(const ScenePtr &_scene,
                std::vector<RayQueryResult> &_result);

      /// \brief Ray origin
      protected: ignition::math::Vector3d origin;

      /// \brief Ray direction
      protected: ignition::math::Vector3d direction;
    };
  }
}
#endif
