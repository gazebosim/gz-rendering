#include "LuxCoreEngineGeometry.hh"

#include "LuxCoreEngineScene.hh"

using namespace ignition;
using namespace rendering;

LuxCoreEngineGeometry::LuxCoreEngineGeometry()
{
  memset(this->transformationMatrix, 0, 16 * sizeof(float));

  this->transformationMatrix[0] = 1.0f;
  this->transformationMatrix[5] = 1.0f;
  this->transformationMatrix[10] = 1.0f;
  this->transformationMatrix[15] = 1.0f;
}

void LuxCoreEngineGeometry::SetLocalPosition(double _x, double _y, double _z)
{
  this->transformationMatrix[12] = _x;
  this->transformationMatrix[13] = _y;
  this->transformationMatrix[14] = _z;

  scene->SceneLux()->UpdateObjectTransformation(Name(), this->transformationMatrix);
}

void LuxCoreEngineGeometry::SetLocalScale(double _x, double _y, double _z)
{
  this->transformationMatrix[0] = _x;
  this->transformationMatrix[5] = _y;
  this->transformationMatrix[10] = _z;

  scene->SceneLux()->UpdateObjectTransformation(Name(), this->transformationMatrix);
}
