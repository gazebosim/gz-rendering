#include "LuxCoreEngineMeshFactory.hh"

#include "LuxCoreEngineScene.hh"

using namespace ignition;
using namespace rendering;

LuxCoreEngineMeshFactory::LuxCoreEngineMeshFactory(LuxCoreEngineScenePtr _scene) :
  scene(_scene) {}

LuxCoreEngineMeshPtr LuxCoreEngineMeshFactory::Create(const MeshDescriptor &_desc, const std::string &_name) 
{
  LuxCoreEngineMeshPtr mesh(new LuxCoreEngineMesh());
  
  if (_desc.meshName == "unit_box")
  {
    std::string meshName = _name + "-mesh";

    struct Vertex {
      float x, y, z;

      Vertex(float x, float y, float z) : x(x), y(y), z(z) {}
    };

	  Vertex *p = (Vertex *)luxcore::Scene::AllocVerticesBuffer(24);

    float minX = -0.5f, minY = -0.5f, minZ = -0.5f;
    float maxX = 0.5f, maxY = 0.5f, maxZ = 0.5f;
    
	  // Bottom face
	  p[0] = Vertex(minX, minY, minZ);
	  p[1] = Vertex(minX, maxY, minZ);
	  p[2] = Vertex(maxX, maxY, minZ);
	  p[3] = Vertex(maxX, minY, minZ);
	  // Top face
	  p[4] = Vertex(minX, minY, maxZ);
	  p[5] = Vertex(maxX, minY, maxZ);
	  p[6] = Vertex(maxX, maxY, maxZ);
	  p[7] = Vertex(minX, maxY, maxZ);
	  // Side left
	  p[8] = Vertex(minX, minY, minZ);
	  p[9] = Vertex(minX, minY, maxZ);
	  p[10] = Vertex(minX, maxY, maxZ);
	  p[11] = Vertex(minX, maxY, minZ);
	  // Side right
	  p[12] = Vertex(maxX, minY, minZ);
	  p[13] = Vertex(maxX, maxY, minZ);
	  p[14] = Vertex(maxX, maxY, maxZ);
	  p[15] = Vertex(maxX, minY, maxZ);
	  // Side back
	  p[16] = Vertex(minX, minY, minZ);
	  p[17] = Vertex(maxX, minY, minZ);
	  p[18] = Vertex(maxX, minY, maxZ);
	  p[19] = Vertex(minX, minY, maxZ);
	  // Side front
	  p[20] = Vertex(minX, maxY, minZ);
	  p[21] = Vertex(minX, maxY, maxZ);
	  p[22] = Vertex(maxX, maxY, maxZ);
	  p[23] = Vertex(maxX, maxY, minZ);

    struct VertexTriangle {
      unsigned int v1, v2, v3;

      VertexTriangle(unsigned int v1, unsigned int v2, unsigned int v3) : v1(v1), v2(v2), v3(v3) {}
    };

	  VertexTriangle *vi = (VertexTriangle *)luxcore::Scene::AllocTrianglesBuffer(12);
	  
    vi[0] = VertexTriangle(0, 1, 2);
	  vi[1] = VertexTriangle(2, 3, 0);
	  // Top face
	  vi[2] = VertexTriangle(4, 5, 6);
	  vi[3] = VertexTriangle(6, 7, 4);
	  // Side left
	  vi[4] = VertexTriangle(8, 9, 10);
	  vi[5] = VertexTriangle(10, 11, 8);
	  // Side right
	  vi[6] = VertexTriangle(12, 13, 14);
	  vi[7] = VertexTriangle(14, 15, 12);
	  // Side back
	  vi[8] = VertexTriangle(16, 17, 18);
	  vi[9] = VertexTriangle(18, 19, 16);
	  // Side back
	  vi[10] = VertexTriangle(20, 21, 22);
	  vi[11] = VertexTriangle(22, 23, 20);

    scene->SceneLux()->DefineMesh(meshName, 24, 12, (float *)p, (unsigned int *)vi, NULL, NULL, NULL, NULL);
  }

  return mesh;
}
