#include "LuxCoreEngineLight.hh"

using namespace ignition;
using namespace rendering;

ignition::math::Color LuxCoreEngineLight::DiffuseColor() const {}

void LuxCoreEngineLight::SetDiffuseColor(const ignition::math::Color &_color) {}

ignition::math::Color LuxCoreEngineLight::SpecularColor() const {}

void LuxCoreEngineLight::SetSpecularColor(const ignition::math::Color &_color) {}

double LuxCoreEngineLight::AttenuationConstant() const {}

void LuxCoreEngineLight::SetAttenuationConstant(double _value) {}

double LuxCoreEngineLight::AttenuationLinear() const {}

void LuxCoreEngineLight::SetAttenuationLinear(double _value) {}

double LuxCoreEngineLight::AttenuationQuadratic() const {}

void LuxCoreEngineLight::SetAttenuationQuadratic(double _value) {}

double LuxCoreEngineLight::AttenuationRange() const {}

void LuxCoreEngineLight::SetAttenuationRange(double _range) {}

bool LuxCoreEngineLight::CastShadows() const {}

void LuxCoreEngineLight::SetCastShadows(bool _castShadows) {}

double LuxCoreEngineLight::Intensity() const {}

void LuxCoreEngineLight::SetIntensity(double _intensity) {}

math::Vector3d LuxCoreEngineDirectionalLight::Direction() const {}

void LuxCoreEngineDirectionalLight::SetDirection(const math::Vector3d &_dir) {}
