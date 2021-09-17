#include "LuxCoreEngineMaterial.hh"

#include "LuxCoreEngineScene.hh"

using namespace ignition;
using namespace rendering;

LuxCoreEngineMaterial::LuxCoreEngineMaterial() {}

LuxCoreEngineMaterial::~LuxCoreEngineMaterial() {}

void LuxCoreEngineMaterial::SetDiffuse(const double _r, const double _g,
                  const double _b, const double _a)
{
  scene->SceneLux()->Parse(
	    luxrays::Property() <<
	    luxrays::Property("scene.materials." + Name() + ".kd")(_r, _g, _b));
}

