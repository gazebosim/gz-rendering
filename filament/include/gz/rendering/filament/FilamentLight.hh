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
      /// \brief Constructor
      protected: FilamentLight();

      /// \brief Constructor
      /// \param[in] _type Type of the filament light
      protected: explicit FilamentLight(filament::LightManager::Type _type);

      /// \brief Destructor
      public: virtual ~FilamentLight() = default;

      // Documentation inherited
      public: virtual math::Color DiffuseColor() const override;

      // Documentation inherited
      public: virtual void SetDiffuseColor(const math::Color &_color) override;

      // Documentation inherited
      public: virtual math::Color SpecularColor() const override;

      // Documentation inherited
      public: virtual void SetSpecularColor(const math::Color &_color) override;

      // Documentation inherited
      public: virtual double AttenuationConstant() const override;

      // Documentation inherited
      public: virtual void SetAttenuationConstant(double _value) override;

      // Documentation inherited
      public: virtual double AttenuationLinear() const override;

      // Documentation inherited
      public: virtual void SetAttenuationLinear(double _value) override;

      // Documentation inherited
      public: virtual double AttenuationQuadratic() const override;

      // Documentation inherited
      public: virtual void SetAttenuationQuadratic(double _value) override;

      // Documentation inherited
      public: virtual double AttenuationRange() const override;

      // Documentation inherited
      public: virtual void SetAttenuationRange(double _range) override;

      // Documentation inherited
      public: virtual bool CastShadows() const override;

      // Documentation inherited
      public: virtual void SetCastShadows(bool _castShadows) override;

      // Documentation inherited
      public: virtual double Intensity() const override;

      // Documentation inherited
      public: virtual void SetIntensity(double _intensity) override;

      // Documentation inherited
      public: virtual void Destroy() override;

      /// \brief Resize the shadow map of a directional light; ignored by
      /// the other types. Called by FilamentScene::SetShadowTextureSize.
      /// \param[in] _size Side of the shadow map, in texels
      public: void SetShadowMapSize(unsigned int _size);

      /// \brief Build the light component on the node's entity, add it to the
      /// filament scene and push the parameters into it. Directional lights
      /// get 3 shadow cascades, like ogre2's PSSM.
      protected: virtual void Init() override;

      /// \brief Push all parameters into the light component.
      protected: virtual void UpdateLight();

      /// \brief Push the attenuation of point and spot lights into the light
      /// component, like ogre2's. It needs a filament build with polynomial
      /// attenuation (GZ_FILAMENT_POLYNOMIAL_ATTENUATION); without it this
      /// does nothing and filament's inverse-square falloff, windowed to the
      /// range, applies.
      /// \param[in] _spotExponent Exponent of the spot cone ramp
      protected: void UpdateAttenuation(double _spotExponent);

      /// \brief Type of the filament light
      protected: filament::LightManager::Type type;

      /// \brief Diffuse color, the color of the filament light
      protected: math::Color diffuse = math::Color::White;

      /// \brief Specular color. Filament lights have a single color, so it
      /// only scales the specular lobe relative to the diffuse color, in
      /// filament builds with polynomial attenuation.
      protected: math::Color specular = math::Color::White;

      /// \brief Constant attenuation factor. Only stored: it does not
      /// affect the lighting, as in ogre2.
      protected: double attenConstant{1.0};

      /// \brief Linear attenuation factor, used with polynomial attenuation
      /// (see UpdateAttenuation)
      protected: double attenLinear{0.0};

      /// \brief Quadratic attenuation factor, used with polynomial
      /// attenuation (see UpdateAttenuation)
      protected: double attenQuadratic{0.0};

      /// \brief Attenuation range in meters, the falloff radius of point
      /// and spot lights
      protected: double attenRange{100.0};

      /// \brief Whether the light casts shadows
      protected: bool castShadows{true};

      /// \brief Light intensity. The power pushed to the light is pi times
      /// this value, in lux for directional lights and candela otherwise.
      protected: double intensity{1.0};
    };

    /// \brief Filament implementation of the directional light class
    class GZ_RENDERING_FILAMENT_VISIBLE FilamentDirectionalLight :
      public BaseDirectionalLight<FilamentLight>
    {
      /// \brief Constructor
      protected: FilamentDirectionalLight();

      /// \brief Destructor
      public: virtual ~FilamentDirectionalLight() = default;

      // Documentation inherited
      public: virtual math::Vector3d Direction() const override;

      // Documentation inherited
      public: virtual void SetDirection(const math::Vector3d &_dir) override;

      /// \brief Push all parameters and the direction into the light
      /// component.
      protected: virtual void UpdateLight() override;

      /// \brief Direction the light shines along, in the node frame
      protected: math::Vector3d direction{0, 0, -1};

      private: friend class FilamentScene;
    };

    /// \brief Filament implementation of the point light class
    class GZ_RENDERING_FILAMENT_VISIBLE FilamentPointLight :
      public BasePointLight<FilamentLight>
    {
      /// \brief Constructor
      protected: FilamentPointLight();

      /// \brief Destructor
      public: virtual ~FilamentPointLight() = default;

      private: friend class FilamentScene;
    };

    /// \brief Filament implementation of the spot light class
    class GZ_RENDERING_FILAMENT_VISIBLE FilamentSpotLight :
      public BaseSpotLight<FilamentLight>
    {
      /// \brief Constructor
      protected: FilamentSpotLight();

      /// \brief Destructor
      public: virtual ~FilamentSpotLight() = default;

      // Documentation inherited
      public: virtual math::Vector3d Direction() const override;

      // Documentation inherited
      public: virtual void SetDirection(const math::Vector3d &_dir) override;

      // Documentation inherited
      public: virtual math::Angle InnerAngle() const override;

      // Documentation inherited
      public: virtual void SetInnerAngle(const math::Angle &_angle) override;

      // Documentation inherited
      public: virtual math::Angle OuterAngle() const override;

      // Documentation inherited
      public: virtual void SetOuterAngle(const math::Angle &_angle) override;

      // Documentation inherited
      public: virtual double Falloff() const override;

      // Documentation inherited
      public: virtual void SetFalloff(double _falloff) override;

      /// \brief Push all parameters, the direction and the cone angles into
      /// the light component.
      protected: virtual void UpdateLight() override;

      /// \brief Direction the light shines along, in the node frame
      protected: math::Vector3d direction{0, 0, -1};

      /// \brief Full angle of the inner cone. Filament takes half of it.
      protected: math::Angle innerAngle{0.5};

      /// \brief Full angle of the outer cone. Filament takes half of it.
      protected: math::Angle outerAngle{0.8};

      /// \brief Falloff of the spot cone, the exponent of its ramp (used
      /// with polynomial attenuation, see UpdateAttenuation)
      protected: double falloff{1.0};

      private: friend class FilamentScene;
    };
    }
  }
}
#endif
