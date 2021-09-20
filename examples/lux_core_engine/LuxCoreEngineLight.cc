#include "LuxCoreEngineLight.hh"

#include "LuxCoreEngineScene.hh"

using namespace ignition;
using namespace rendering;

ignition::math::Color LuxCoreEngineLight::DiffuseColor() const {}

void LuxCoreEngineLight::SetDiffuseColor(double _r, double _g, double _b, double _a)
{
  this->gainR = _r;
  this->gainG = _g;
  this->gainB = _b;

  scene->SceneLux()->Parse(
	    luxrays::Property("scene.lights." + Name() + ".type")(this->lightType) <<
	    luxrays::Property("scene.lights." + Name() + ".gain")(this->gainR, this->gainG, this->gainB) <<
      luxrays::Property("scene.lights." + Name() + ".dir")(this->directionX, this->directionY, this->directionZ));
}

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

LuxCoreEngineDirectionalLight::LuxCoreEngineDirectionalLight(std::string lightType)
{
  this->lightType = lightType;
}

math::Vector3d LuxCoreEngineDirectionalLight::Direction() const {}

void LuxCoreEngineDirectionalLight::SetDirection(double _x, double _y, double _z)
{
  this->directionX = _x;
  this->directionY = _y;
  this->directionZ = -_z;

  scene->SceneLux()->Parse(
	    luxrays::Property("scene.lights." + Name() + ".type")(this->lightType) <<
	    luxrays::Property("scene.lights." + Name() + ".gain")(this->gainR, this->gainG, this->gainB) <<
      luxrays::Property("scene.lights." + Name() + ".dir")(this->directionX, this->directionY, this->directionZ));
}

void LuxCoreEngineDirectionalLight::SetDirection(const math::Vector3d &_dir) {}
