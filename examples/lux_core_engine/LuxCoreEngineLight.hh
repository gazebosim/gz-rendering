/*
 * Copyright (C) 2022 Open Source Robotics Foundation
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
#ifndef IGNITION_RENDERING_LUXCOREENGINE_LUXCOREENGINELIGHT_HH_
#define IGNITION_RENDERING_LUXCOREENGINE_LUXCOREENGINELIGHT_HH_

#include <luxcore/luxcore.h>

#include "ignition/rendering/base/BaseLight.hh"

#include "LuxCoreEngineNode.hh"
#include "LuxCoreEngineRenderTypes.hh"

namespace ignition {
namespace rendering {
inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {

class LuxCoreEngineLight : public BaseLight<LuxCoreEngineNode> {

  public: virtual ignition::math::Color DiffuseColor() const;

  public: virtual void SetDiffuseColor(double _r, double _g, double _b,
                                       double _a = 1.0);

  public: virtual void SetDiffuseColor(const ignition::math::Color &_color);

  public: virtual ignition::math::Color SpecularColor() const;

  public: virtual void SetSpecularColor(const ignition::math::Color &_color);

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

  public: virtual double Intensity() const;

  public: virtual void SetIntensity(double _intensity);

  public: virtual void UpdateLuxSDL() = 0;

  protected: std::string lightType;

  protected: float gainR, gainG, gainB;
};

class LuxCoreEngineDirectionalLight
    : public BaseDirectionalLight<LuxCoreEngineLight> {

  public:LuxCoreEngineDirectionalLight(std::string lightType);

  public: virtual math::Vector3d Direction() const;

  public: virtual void SetDirection(double _x, double _y, double _z);

  public: virtual void SetDirection(const math::Vector3d &_dir);

  public: virtual void UpdateLuxSDL();

  protected: float directionX, directionY, directionZ;
};

class LuxCoreEnginePointLight : public BasePointLight<LuxCoreEngineLight> {

  public: LuxCoreEnginePointLight(std::string lightType);

  public: virtual void SetLocalPosition(double _x, double _y, double _z);

  public: virtual void UpdateLuxSDL();

  protected: float localPositionX, localPositionY, localPositionZ;
};

} // namespace IGNITION_RENDERING_VERSION_NAMESPACE
} // namespace rendering
} // namespace ignition

#endif
