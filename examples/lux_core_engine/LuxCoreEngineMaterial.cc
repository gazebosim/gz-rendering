#include "LuxCoreEngineMaterial.hh"

#include "LuxCoreEngineScene.hh"

using namespace ignition;
using namespace rendering;

LuxCoreEngineMaterial::LuxCoreEngineMaterial() {
  this->diffuseR = 0.5f;
  this->diffuseG = 0.5f;
  this->diffuseB = 0.5f;

  this->materialType = "matte";
}

LuxCoreEngineMaterial::~LuxCoreEngineMaterial() {}

void LuxCoreEngineMaterial::UpdateLuxSDL() {
  scene->SceneLux()->Parse(
      luxrays::Property("scene.materials." + Name() +
                        ".type")(this->materialType)
      << luxrays::Property("scene.materials." + Name() + ".kd")(
             this->diffuseR, this->diffuseG, this->diffuseB)
      << luxrays::Property("scene.materials." + Name() + ".emission")(
             this->emissiveR, this->emissiveG, this->emissiveB));
}

void LuxCoreEngineMaterial::SetDiffuse(const double _r, const double _g,
                                       const double _b, const double _a) {
  this->diffuseR = _r;
  this->diffuseG = _g;
  this->diffuseB = _b;

  this->UpdateLuxSDL();
}

void LuxCoreEngineMaterial::SetEmissive(const double _r, const double _g,
                                        const double _b, const double _a) {
  this->emissiveR = _r;
  this->emissiveG = _g;
  this->emissiveB = _b;

  this->UpdateLuxSDL();
}

void LuxCoreEngineMaterial::SetShaderType(enum ShaderType _type) {
  if (_type == 0) {
    this->materialType = "matte";
  }
  if (_type == 1) {
    this->materialType = "glossy2";
  }
  if (_type == 2) {
    this->materialType = "metal2";
  }
  if (_type == 3) {
    this->materialType = "mirror";
  }
  if (_type == 4) {
    this->materialType = "glass";
  }

  this->UpdateLuxSDL();
}
