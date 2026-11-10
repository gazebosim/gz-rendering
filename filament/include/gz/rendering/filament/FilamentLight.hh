/*
 * Copyright (C) 2026 Open Source Robotics Foundation
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
#ifndef GZ_RENDERING_FILAMENT_FILAMENTLIGHT_HH_
#define GZ_RENDERING_FILAMENT_FILAMENTLIGHT_HH_

#include <filament/LightManager.h>

#include "gz/rendering/config.hh"
#include "gz/rendering/base/BaseLight.hh"
#include "gz/rendering/filament/FilamentNode.hh"
#include "gz/rendering/filament/FilamentRenderTypes.hh"
#include "gz/rendering/filament/Export.hh"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    //
    /// \brief Light component living on the node's entity, so the node
    /// transform positions/orients it.
    class GZ_RENDERING_FILAMENT_VISIBLE FilamentLight :
      public BaseLight<FilamentNode>
    {
      protected: FilamentLight();

      protected: explicit FilamentLight(filament::LightManager::Type _type);

      public: virtual ~FilamentLight();

      public: virtual math::Color DiffuseColor() const override;

      public: virtual void SetDiffuseColor(const math::Color &_color) override;

      public: virtual math::Color SpecularColor() const override;

      public: virtual void SetSpecularColor(const math::Color &_color) override;

      public: virtual double AttenuationConstant() const override;

      public: virtual void SetAttenuationConstant(double _value) override;

      public: virtual double AttenuationLinear() const override;

      public: virtual void SetAttenuationLinear(double _value) override;

      public: virtual double AttenuationQuadratic() const override;

      public: virtual void SetAttenuationQuadratic(double _value) override;

      public: virtual double AttenuationRange() const override;

      public: virtual void SetAttenuationRange(double _range) override;

      public: virtual bool CastShadows() const override;

      public: virtual void SetCastShadows(bool _castShadows) override;

      public: virtual double Intensity() const override;

      public: virtual void SetIntensity(double _intensity) override;

      public: virtual void Destroy() override;

      /// \brief Resize the shadow map of a directional light; ignored by
      /// the other types. Called by FilamentScene::SetShadowTextureSize.
      public: void SetShadowMapSize(unsigned int _size);

      protected: virtual void Init() override;

      /// \brief Push all parameters into the light component
      protected: virtual void UpdateLight();

      /// \brief Set ogre2's attenuation (point and spot lights)
      /// \param[in] _spotExponent Exponent of the spot cone ramp
      protected: void UpdateAttenuation(double _spotExponent);

      protected: filament::LightManager::Type type;

      protected: math::Color diffuse = math::Color::White;

      protected: math::Color specular = math::Color::White;

      protected: double attenConstant{1.0};

      protected: double attenLinear{0.0};

      protected: double attenQuadratic{0.0};

      protected: double attenRange{100.0};

      protected: bool castShadows{true};

      protected: double intensity{1.0};
    };

    class GZ_RENDERING_FILAMENT_VISIBLE FilamentDirectionalLight :
      public BaseDirectionalLight<FilamentLight>
    {
      protected: FilamentDirectionalLight();

      public: virtual ~FilamentDirectionalLight();

      public: virtual math::Vector3d Direction() const override;

      public: virtual void SetDirection(const math::Vector3d &_dir) override;

      protected: virtual void UpdateLight() override;

      protected: math::Vector3d direction{0, 0, -1};

      private: friend class FilamentScene;
    };

    class GZ_RENDERING_FILAMENT_VISIBLE FilamentPointLight :
      public BasePointLight<FilamentLight>
    {
      protected: FilamentPointLight();

      public: virtual ~FilamentPointLight();

      private: friend class FilamentScene;
    };

    class GZ_RENDERING_FILAMENT_VISIBLE FilamentSpotLight :
      public BaseSpotLight<FilamentLight>
    {
      protected: FilamentSpotLight();

      public: virtual ~FilamentSpotLight();

      public: virtual math::Vector3d Direction() const override;

      public: virtual void SetDirection(const math::Vector3d &_dir) override;

      public: virtual math::Angle InnerAngle() const override;

      public: virtual void SetInnerAngle(const math::Angle &_angle) override;

      public: virtual math::Angle OuterAngle() const override;

      public: virtual void SetOuterAngle(const math::Angle &_angle) override;

      public: virtual double Falloff() const override;

      public: virtual void SetFalloff(double _falloff) override;

      protected: virtual void UpdateLight() override;

      protected: math::Vector3d direction{0, 0, -1};

      protected: math::Angle innerAngle{0.5};

      protected: math::Angle outerAngle{0.8};

      protected: double falloff{1.0};

      private: friend class FilamentScene;
    };
    }
  }
}
#endif
