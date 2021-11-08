#ifndef IGNITION_RENDERING_LUXCOREENGINE_LUXCOREENGINEMESH_HH_
#define IGNITION_RENDERING_LUXCOREENGINE_LUXCOREENGINEMESH_HH_

#include "ignition/common/Console.hh"

#include "ignition/rendering/base/BaseMesh.hh"

#include "LuxCoreEngineGeometry.hh"
#include "LuxCoreEngineRenderTypes.hh"

#include "luxcore/luxcore.h"

namespace ignition {
namespace rendering {
inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
class LuxCoreEngineMesh : public BaseMesh<LuxCoreEngineGeometry> {
public:
  LuxCoreEngineMesh();

public:
  bool HasParent() const;

public:
  VisualPtr Parent() const;

public:
  void SetMaterial(MaterialPtr _material, bool _unique = true);

public:
  void SetLocalPosition(double _x, double _y, double _z);

public:
  void SetLocalScale(double _x, double _y, double _z);

public:
  void SetLocalRotation(double _r, double _p, double _y);

public:
  void SetName(std::string name);

public:
  void AddSubMesh(const LuxCoreEngineSubMeshPtr _child);

public:
  SubMeshStorePtr SubMeshes() const;

private:
  void UpdateTransformation();

protected:
  LuxCoreEngineSubMeshStorePtr subMeshes;

private:
  float translationMatrix[16];

private:
  float rotationMatrix[16];

private:
  float scaleMatrix[16];

private:
  float previousMatrix[16];
};

class LuxCoreEngineSubMesh : public BaseSubMesh<LuxCoreEngineObject> {
public:
  void SetMaterialImpl(MaterialPtr _material);

public:
  void SetName(std::string name);
};
} // namespace IGNITION_RENDERING_VERSION_NAMESPACE
} // namespace rendering
} // namespace ignition

#endif
