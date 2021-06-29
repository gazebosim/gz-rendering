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
#ifndef IGNITION_RENDERING_JOINTVISUAL_HH_
#define IGNITION_RENDERING_JOINTVISUAL_HH_

#include <ignition/math/Inertial.hh>
#include "ignition/rendering/config.hh"
#include "ignition/rendering/Object.hh"
#include "ignition/rendering/RenderTypes.hh"
#include "ignition/rendering/Visual.hh"

namespace ignition
{
  namespace rendering
  {
    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
    //
    /// \brief Enum for JointVisual types
    enum IGNITION_RENDERING_VISIBLE JointVisualType
    {
      /// \brief No type
      JVT_NONE           = 0,

      /// \brief Revolute joint type
      JVT_REVOLUTE       = 1,

      /// \brief Revolute2 joint type
      JVT_REVOLUTE2      = 2,

      /// \brief Prismatic joint type
      JVT_PRISMATIC      = 3,

      /// \brief Universal joint type
      JVT_UNIVERSAL      = 4,

      /// \brief Ball joint type
      JVT_BALL           = 5,

      /// \brief Screw joint type
      JVT_SCREW          = 6,

      /// \brief Gearbox joint type
      JVT_GEARBOX        = 7,

      /// \brief Fixed joint type
      JVT_FIXED          = 8
    };

    /// \class JointVisual JointVisual.hh
    /// ignition/rendering/JointVisual.hh
    /// \brief Represents a joint visual
    class IGNITION_RENDERING_VISIBLE JointVisual :
      public virtual Visual
    {
      /// \brief Destructor
      public: virtual ~JointVisual() {}

      /// \brief Create an axis and attach it to the joint visual.
      /// \param[in] _axis Axis vector
      /// \param[in] _xyzExpressedIn Frame in which the axis vector is
      /// expressed.
      /// \param[in] _type Type of axis.
      /// \returns Newly created arrow visual.
      public: ArrowVisualPtr CreateAxis(const ignition::math::Vector3d &_axis,
          const std::string &_xyzExpressedIn, const JointVisualType _type);

      /// \brief Update an axis' arrow visual.
      /// \param[in] _arrowVisual Arrow visual to be updated.
      /// \param[in] _axis Axis vector.
      /// \param[in] _xyzExpressedIn Frame in which the axis vector is
      /// expressed.
      /// \param[in] _type Type of axis.
      public: void UpdateAxis(ArrowVisualPtr _arrowVisual,
                              const ignition::math::Vector3d &_axis,
                              const std::string &_xyzExpressedIn,
                              const JointVisualType _type);

      /// \brief Get the JointVisual which is attached to the parent link.
      /// \return Parent axis visual.
      public: virtual VisualPtr ParentAxisVisual() const = 0;

      /// \brief Get the arrow visual which represents the axis attached to the
      /// child.
      /// \return Arrow visual.
      public: ArrowVisualPtr GetArrowVisual() const;
    };
    }
  }
}
#endif
