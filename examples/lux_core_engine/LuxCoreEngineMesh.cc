#include "LuxCoreEngineMesh.hh"

#include "LuxCoreEngineScene.hh"

using namespace ignition;
using namespace rendering;

LuxCoreEngineMesh::LuxCoreEngineMesh() {}

bool LuxCoreEngineMesh::HasParent() const {}

VisualPtr LuxCoreEngineMesh::Parent() const {}

void LuxCoreEngineMesh::SetMaterial(MaterialPtr _material, bool _unique)
{
  scene->SceneLux()->Parse(
	    luxrays::Property("scene.objects." + Name() + ".shape")(Name() + "-mesh") <<
	    luxrays::Property("scene.objects." + Name()  + ".material")(_material->Name()));
}

SubMeshStorePtr LuxCoreEngineMesh::SubMeshes() const {}
