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
#ifndef _IGNITION_RENDERING_LIGHT_HH_
#define _IGNITION_RENDERING_LIGHT_HH_

#include "gazebo/common/Color.hh"
#include "ignition/rendering/Node.hh"

namespace ignition
{
  namespace rendering
  {
    class IGNITION_VISIBLE Light :
      public virtual Node
    {
      public: virtual ~Light() { }

      public: virtual gazebo::common::Color GetDiffuseColor() const = 0;

      public: virtual void SetDiffuseColor(double _r, double _g, double _b,
                  double _a = 1.0) = 0;

      public: virtual void SetDiffuseColor(const gazebo::common::Color &_color) = 0;

      public: virtual gazebo::common::Color GetSpecularColor() const = 0;

      public: virtual void SetSpecularColor(double _r, double _g, double _b,
                  double _a = 1.0) = 0;

      public: virtual void SetSpecularColor(const gazebo::common::Color &_color) = 0;

      public: virtual double GetAttenuationConstant() const = 0;

      public: virtual void SetAttenuationConstant(double _weight) = 0;

      public: virtual double GetAttenuationLinear() const = 0;

      public: virtual void SetAttenuationLinear(double _weight) = 0;

      public: virtual double GetAttenuationQuadratic() const = 0;

      public: virtual void SetAttenuationQuadratic(double _weight) = 0;

      public: virtual double GetAttenuationRange() const = 0;

      public: virtual void SetAttenuationRange(double _range) = 0;

      public: virtual bool GetCastShadows() const = 0;

      public: virtual void SetCastShadows(bool _castShadows) = 0;
    };

    class IGNITION_VISIBLE DirectionalLight :
      public virtual Light
    {
      public: virtual ~DirectionalLight() { }

      public: virtual math::Vector3d GetDirection() const = 0;

      public: virtual void SetDirection(double _x, double _y, double _z) = 0;

      public: virtual void SetDirection(const math::Vector3d &_dir) = 0;
    };

    class IGNITION_VISIBLE PointLight :
      public virtual Light
    {
      public: virtual ~PointLight() { }
    };

    class IGNITION_VISIBLE SpotLight :
      public virtual Light
    {
      public: virtual ~SpotLight() { }

      public: virtual math::Vector3d GetDirection() const = 0;

      public: virtual void SetDirection(double _x, double _y, double _z) = 0;

      public: virtual void SetDirection(const math::Vector3d &_dir) = 0;

      public: virtual math::Angle GetInnerAngle() const = 0;

      public: virtual void SetInnerAngle(double _radians) = 0;

      public: virtual void SetInnerAngle(const math::Angle &_angle) = 0;

      public: virtual math::Angle GetOuterAngle() const = 0;

      public: virtual void SetOuterAngle(double _radians) = 0;

      public: virtual void SetOuterAngle(const math::Angle &_angle) = 0;

      public: virtual double GetFalloff() const = 0;

      public: virtual void SetFalloff(double _falloff) = 0;
    };
  }
}
#endif
