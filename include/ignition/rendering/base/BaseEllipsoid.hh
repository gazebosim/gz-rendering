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

#include "ignition/rendering/base/BaseObject.hh"
#include "ignition/rendering/Ellipsoid.hh"

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
      public: virtual void SetRadii(
        const ignition::math::Vector3d &_radii) override;

      // Documentation inherited
      public: virtual ignition::math::Vector3d Radii() override;

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
    }
  }
}
#endif
