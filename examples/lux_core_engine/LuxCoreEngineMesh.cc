#include "LuxCoreEngineMesh.hh"

#include "LuxCoreEngineScene.hh"

using namespace ignition;
using namespace rendering;

LuxCoreEngineMesh::LuxCoreEngineMesh() {}

bool LuxCoreEngineMesh::HasParent() const {}

VisualPtr LuxCoreEngineMesh::Parent() const {}

void LuxCoreEngineMesh::SetMaterial(MaterialPtr _material, bool _unique)
{
  std::string objName = Name();
  std::string meshName = Name() + "-mesh";
  std::string matName = "mat_white"; 
  luxrays::Properties props;
	props.SetFromString(
		"scene.objects." + objName + ".shape = " + meshName + "\n"
		"scene.objects." + objName + ".material = " + matName + "\n");
	scene->SceneLux()->Parse(props);
}

SubMeshStorePtr LuxCoreEngineMesh::SubMeshes() const {}
