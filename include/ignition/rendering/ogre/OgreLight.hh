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
#ifndef _IGNITION_RENDERING_OGRELIGHT_HH_
#define _IGNITION_RENDERING_OGRELIGHT_HH_

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
    class IGNITION_VISIBLE OgreLight :
      public BaseLight<OgreNode>
    {
      protected: OgreLight();

      public: virtual ~OgreLight();

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

      public: virtual Ogre::Light *GetOgreLight() const;

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

    class IGNITION_VISIBLE OgreDirectionalLight :
      public BaseDirectionalLight<OgreLight>
    {
      protected: OgreDirectionalLight();

      public: virtual ~OgreDirectionalLight();

      public: virtual math::Vector3d GetDirection() const;

      public: virtual void SetDirection(const math::Vector3d &_dir);

      private: friend class OgreScene;
    };

    class IGNITION_VISIBLE OgrePointLight :
      public BasePointLight<OgreLight>
    {
      protected: OgrePointLight();

      public: virtual ~OgrePointLight();

      private: friend class OgreScene;
    };

    class IGNITION_VISIBLE OgreSpotLight :
      public BaseSpotLight<OgreLight>
    {
      protected: OgreSpotLight();

      public: virtual ~OgreSpotLight();

      public: virtual math::Vector3d GetDirection() const;

      public: virtual void SetDirection(const math::Vector3d &_dir);

      public: virtual math::Angle GetInnerAngle() const;

      public: virtual void SetInnerAngle(const math::Angle &_angle);

      public: virtual math::Angle GetOuterAngle() const;

      public: virtual void SetOuterAngle(const math::Angle &_angle);

      public: virtual double GetFalloff() const;

      public: virtual void SetFalloff(double _falloff);

      private: friend class OgreScene;
    };
  }
}
#endif
