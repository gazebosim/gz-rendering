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
#ifndef _IGNITION_RENDERING_OPTIXLIGHT_HH_
#define _IGNITION_RENDERING_OPTIXLIGHT_HH_

#include "ignition/rendering/base/BaseLight.hh"
#include "ignition/rendering/optix/OptixNode.hh"
#include "ignition/rendering/optix/OptixIncludes.hh"
#include "ignition/rendering/optix/OptixLightTypes.hh"

namespace ignition
{
  namespace rendering
  {
    class IGNITION_VISIBLE OptixLight :
      public BaseLight<OptixNode>
    {
      protected: OptixLight();

      public: virtual ~OptixLight();

      public: virtual gazebo::common::Color GetDiffuseColor() const;

      public: virtual void SetDiffuseColor(const gazebo::common::Color &_color);

      public: virtual gazebo::common::Color GetSpecularColor() const;

      public: virtual void SetSpecularColor(const gazebo::common::Color &_color);

      public: virtual double GetAttenuationConstant() const;

      public: virtual void SetAttenuationConstant(double _weight);

      public: virtual double GetAttenuationLinear() const;

      public: virtual void SetAttenuationLinear(double _weight);

      public: virtual double GetAttenuationQuadratic() const;

      public: virtual void SetAttenuationQuadratic(double _weight);

      public: virtual double GetAttenuationRange() const;

      public: virtual void SetAttenuationRange(double _range);

      public: virtual bool GetCastShadows() const;

      public: virtual void SetCastShadows(bool _castShadows);

      protected: virtual void WritePoseToDeviceImpl();

      protected: virtual OptixCommonLightData &GetCommonData() = 0;

      protected: virtual const OptixCommonLightData &GetCommonData() const = 0;

      protected: virtual void Init();
    };

    class IGNITION_VISIBLE OptixDirectionalLight :
      public BaseDirectionalLight<OptixLight>
    {
      protected: OptixDirectionalLight();

      public: virtual ~OptixDirectionalLight();

      public: virtual math::Vector3d GetDirection() const;

      public: virtual void SetDirection(const math::Vector3d &_dir);

      public: virtual OptixDirectionalLightData GetData() const;

      public: virtual void PreRender();

      protected: virtual OptixCommonLightData &GetCommonData();

      protected: virtual const OptixCommonLightData &GetCommonData() const;

      protected: OptixDirectionalLightData data;

      private: OptixDirectionalLightPtr SharedThis();

      private: friend class OptixScene;
    };

    class IGNITION_VISIBLE OptixPointLight :
      public BasePointLight<OptixLight>
    {
      protected: OptixPointLight();

      public: virtual ~OptixPointLight();

      public: virtual OptixPointLightData GetData() const;

      public: virtual void PreRender();

      protected: virtual OptixCommonLightData &GetCommonData();

      protected: virtual const OptixCommonLightData &GetCommonData() const;

      protected: OptixPointLightData data;

      private: OptixPointLightPtr SharedThis();

      private: friend class OptixScene;
    };

    class IGNITION_VISIBLE OptixSpotLight :
      public BaseSpotLight<OptixLight>
    {
      protected: OptixSpotLight();

      public: virtual ~OptixSpotLight();

      public: virtual math::Vector3d GetDirection() const;

      public: virtual void SetDirection(const math::Vector3d &_dir);

      public: virtual math::Angle GetInnerAngle() const;

      public: virtual void SetInnerAngle(const math::Angle &_angle);

      public: virtual math::Angle GetOuterAngle() const;

      public: virtual void SetOuterAngle(const math::Angle &_angle);

      public: virtual double GetFalloff() const;

      public: virtual void SetFalloff(double _falloff);

      public: virtual OptixSpotLightData GetData() const;

      public: virtual void PreRender();

      protected: virtual OptixCommonLightData &GetCommonData();

      protected: virtual const OptixCommonLightData &GetCommonData() const;

      protected: OptixSpotLightData data;

      private: OptixSpotLightPtr SharedThis();

      private: friend class OptixScene;
    };
  }
}
#endif
