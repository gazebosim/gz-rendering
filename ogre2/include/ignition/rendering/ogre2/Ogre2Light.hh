/*
 * Copyright (C) 2018 Open Source Robotics Foundation
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
#ifndef IGNITION_RENDERING_OGRE2_OGRE2LIGHT_HH_
#define IGNITION_RENDERING_OGRE2_OGRE2LIGHT_HH_

#include "ignition/rendering/base/BaseLight.hh"
#include "ignition/rendering/ogre2/Ogre2Node.hh"
#include "ignition/rendering/ogre2/Ogre2Includes.hh"

namespace Ogre
{
  class Light;
}

namespace ignition
{
  namespace rendering
  {
    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
    //
    class IGNITION_RENDERING_OGRE2_VISIBLE Ogre2Light :
      public BaseLight<Ogre2Node>
    {
      protected: Ogre2Light();

      public: virtual ~Ogre2Light();

      public: virtual math::Color DiffuseColor() const;

      // Documentation Inherited
      public: virtual void SetDiffuseColor(const math::Color &_color) override;

      public: virtual math::Color SpecularColor() const;

      // Documentation Inherited
      public: virtual void SetSpecularColor(const math::Color &_color) override;

      public: virtual double AttenuationConstant() const;

      public: virtual void SetAttenuationConstant(double _value);

      public: virtual double AttenuationLinear() const;

      public: virtual void SetAttenuationLinear(double _value);

      public: virtual double AttenuationQuadratic() const;

      public: virtual void SetAttenuationQuadratic(double _value);

      public: virtual double AttenuationRange() const;

      public: virtual void SetAttenuationRange(double _range);

      public: virtual bool CastShadows() const;

      public: virtual void SetCastShadows(bool _castShadows);

      public: virtual Ogre::Light *Light() const;

      public: virtual void Destroy();

      protected: virtual void Init();

      private: void CreateLight();

      private: void UpdateAttenuation();

      protected: double attenConstant;

      protected: double attenLinear;

      protected: double attenQuadratic;

      protected: double attenRange;

      protected: Ogre::Light *ogreLight;

      protected: Ogre::Light::LightTypes ogreLightType;
    };

    class IGNITION_RENDERING_OGRE2_VISIBLE Ogre2DirectionalLight :
      public BaseDirectionalLight<Ogre2Light>
    {
      protected: Ogre2DirectionalLight();

      public: virtual ~Ogre2DirectionalLight();

      public: virtual math::Vector3d Direction() const;

      // Documentation Inherited
      public: virtual void SetDirection(const math::Vector3d &_dir) override;

      private: friend class Ogre2Scene;
    };

    class IGNITION_RENDERING_OGRE2_VISIBLE Ogre2PointLight :
      public BasePointLight<Ogre2Light>
    {
      protected: Ogre2PointLight();

      public: virtual ~Ogre2PointLight();

      private: friend class Ogre2Scene;
    };

    class IGNITION_RENDERING_OGRE2_VISIBLE Ogre2SpotLight :
      public BaseSpotLight<Ogre2Light>
    {
      protected: Ogre2SpotLight();

      public: virtual ~Ogre2SpotLight();

      public: virtual math::Vector3d Direction() const;

      // Documentation Inherited
      public: virtual void SetDirection(const math::Vector3d &_dir) override;

      public: virtual math::Angle InnerAngle() const;

      // Documentation Inherited
      public: virtual void SetInnerAngle(const math::Angle &_angle) override;

      public: virtual math::Angle OuterAngle() const;

      // Documentation Inherited
      public: virtual void SetOuterAngle(const math::Angle &_angle) override;

      public: virtual double Falloff() const;

      public: virtual void SetFalloff(double _falloff);

      private: friend class Ogre2Scene;
    };
    }
  }
}
#endif
