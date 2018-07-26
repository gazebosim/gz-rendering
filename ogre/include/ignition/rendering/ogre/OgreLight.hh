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
#ifndef IGNITION_RENDERING_OGRE_OGRELIGHT_HH_
#define IGNITION_RENDERING_OGRE_OGRELIGHT_HH_

#include "ignition/rendering/base/BaseLight.hh"
#include "ignition/rendering/ogre/OgreNode.hh"
#include "ignition/rendering/ogre/OgreIncludes.hh"

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
    class IGNITION_RENDERING_OGRE_VISIBLE OgreLight :
      public BaseLight<OgreNode>
    {
      protected: OgreLight();

      public: virtual ~OgreLight();

      public: virtual math::Color DiffuseColor() const;

      public: virtual void SetDiffuseColor(const math::Color &_color);

      public: virtual math::Color SpecularColor() const;

      public: virtual void SetSpecularColor(const math::Color &_color);

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

    class IGNITION_RENDERING_OGRE_VISIBLE OgreDirectionalLight :
      public BaseDirectionalLight<OgreLight>
    {
      protected: OgreDirectionalLight();

      public: virtual ~OgreDirectionalLight();

      public: virtual math::Vector3d Direction() const;

      public: virtual void SetDirection(const math::Vector3d &_dir);

      private: friend class OgreScene;
    };

    class IGNITION_RENDERING_OGRE_VISIBLE OgrePointLight :
      public BasePointLight<OgreLight>
    {
      protected: OgrePointLight();

      public: virtual ~OgrePointLight();

      private: friend class OgreScene;
    };

    class IGNITION_RENDERING_OGRE_VISIBLE OgreSpotLight :
      public BaseSpotLight<OgreLight>
    {
      protected: OgreSpotLight();

      public: virtual ~OgreSpotLight();

      public: virtual math::Vector3d Direction() const;

      public: virtual void SetDirection(const math::Vector3d &_dir);

      public: virtual math::Angle InnerAngle() const;

      public: virtual void SetInnerAngle(const math::Angle &_angle);

      public: virtual math::Angle OuterAngle() const;

      public: virtual void SetOuterAngle(const math::Angle &_angle);

      public: virtual double Falloff() const;

      public: virtual void SetFalloff(double _falloff);

      private: friend class OgreScene;
    };
    }
  }
}
#endif
