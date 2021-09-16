#include "LuxCoreEngineVisual.hh"

using namespace ignition;
using namespace rendering;

void LuxCoreEngineVisual::SetMaterial(MaterialPtr _material, bool _unique)
{
  for (auto iter = this->geometries->Begin(); iter != this->geometries->End(); ++iter)
  {
    iter->second->SetMaterial(_material, _unique);
  }

  printf("Visual %s\n", Name().c_str());
  printf("Material %s\n", _material->Name().c_str());
}

GeometryStorePtr LuxCoreEngineVisual::Geometries() const 
{
  return this->geometries;
}

bool LuxCoreEngineVisual::AttachGeometry(GeometryPtr _geometry) {}

bool LuxCoreEngineVisual::DetachGeometry(GeometryPtr _geometry) {}

void LuxCoreEngineVisual::Init()
{
  BaseVisual::Init();
  this->CreateStorage();
}

void LuxCoreEngineVisual::CreateStorage()
{
  this->geometries = LuxCoreEngineGeometryStorePtr(new LuxCoreEngineGeometryStore);
}

LuxCoreEngineVisualPtr LuxCoreEngineVisual::SharedThis()
{
  return std::dynamic_pointer_cast<LuxCoreEngineVisual>(shared_from_this());
}
