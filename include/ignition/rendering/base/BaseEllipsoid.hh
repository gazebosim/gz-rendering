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

#ifndef IGNITION_RENDERING_BASEELLIPSOID_HH_
#define IGNITION_RENDERING_BASEELLIPSOID_HH_

#include <vector>

#include <ignition/math/Vector3.hh>
#include <ignition/math/Vector2.hh>

#include "ignition/rendering/Ellipsoid.hh"
#include "ignition/rendering/base/BaseObject.hh"

namespace ignition
{
  namespace rendering
  {
    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {

    /// \brief Base implementation of a Ellipsoid Visual
    template <class T>
    class BaseEllipsoid :
      public virtual Ellipsoid,
      public virtual T
    {
      // Documentation inherited
      protected: BaseEllipsoid();

      // Documentation inherited
      public: virtual ~BaseEllipsoid();

      // Documentation inherited
      public: virtual void PreRender() override;

      // Documentation inherited.
      public: virtual void Destroy() override;

      // Documentation inherited
      public: virtual void SetRadii(
        const ignition::math::Vector3d &_radii) override;

      // Documentation inherited
      public: virtual ignition::math::Vector3d Radii() override;

      /// \brief return the list of triangles, _indexes and _uv coordinates
      /// \param[inout] _positions list with the triangles
      /// \param[inout] _indexes indexes of the position list
      /// \param[inout] _uv UV coordinates
      protected: void EllipsoidMesh(
        std::vector<ignition::math::Vector3d> &_positions,
        std::vector<int> &_indexes,
        std::vector<ignition::math::Vector2d> &_uv);

      /// \brief Radius of the ellipsoid
      protected: ignition::math::Vector3d radii =
        ignition::math::Vector3d(1, 1, 1);

      /// \brief Flag to indicate grid properties have changed
      protected: bool ellipsoidDirty = false;
    };

    /////////////////////////////////////////////////
    // BaseEllipsoid
    /////////////////////////////////////////////////
    template <class T>
    BaseEllipsoid<T>::BaseEllipsoid()
    {
    }

    /////////////////////////////////////////////////
    template <class T>
    BaseEllipsoid<T>::~BaseEllipsoid()
    {
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseEllipsoid<T>::PreRender()
    {
      T::PreRender();
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseEllipsoid<T>::Destroy()
    {
      T::Destroy();
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseEllipsoid<T>::SetRadii(const ignition::math::Vector3d &_radii)
    {
      this->radii = _radii;
      this->ellipsoidDirty = true;
    }

    /////////////////////////////////////////////////
    template <class T>
    ignition::math::Vector3d BaseEllipsoid<T>::Radii()
    {
      return this->radii;
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseEllipsoid<T>::EllipsoidMesh(
      std::vector<ignition::math::Vector3d> &_positions,
      std::vector<int> &_indexes,
      std::vector<ignition::math::Vector2d> &/*_uv*/)
    {
      double umin = -M_PI / 2.0;
      double umax = M_PI / 2.0;
      double vmin = 0.0;
      double vmax = 2.0 * M_PI;
      float n = 32;
      float m = 32;
      float theta, phi;
      float d_phi = (umax - umin) / (n - 1.0);
      float d_theta = (vmax - vmin) / (m - 1.0);

      float c_theta, s_theta, c_phi, s_phi;

      for (unsigned int i = 0, theta = vmin; i < m; ++i, theta += d_theta)
      {
        c_theta = cos(theta);
        s_theta = sin(theta);

        for (unsigned int j = 0, phi = umin; j < n; ++j, phi += d_phi)
        {
          c_phi = cos(phi);
          s_phi = sin(phi);

          // Compute vertex
          _positions.emplace_back(ignition::math::Vector3d(
            this->radii.X() * c_phi * c_theta,
            this->radii.Y() * c_phi * s_theta,
            this->radii.Z() * s_phi));

          if (i > 0)
          {
            unsigned int verticesCount = _positions.size();
            for (
              unsigned int firstIndex = verticesCount - 2 * (n + 1);
              firstIndex + n + 2 < verticesCount;
              firstIndex++)
            {
              _indexes.emplace_back(firstIndex + n + 1);
              _indexes.emplace_back(firstIndex + 1);
              _indexes.emplace_back(firstIndex + 0);

              _indexes.emplace_back(firstIndex + n + 2);
              _indexes.emplace_back(firstIndex + 1);
              _indexes.emplace_back(firstIndex + n + 1);
            }
          }
        }
      }
    }
    }
  }
}
#endif
