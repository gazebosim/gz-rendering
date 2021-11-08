#ifndef IGNITION_RENDERING_LUXCOREENGINE_LUXCOREENGINENODE_HH_
#define IGNITION_RENDERING_LUXCOREENGINE_LUXCOREENGINENODE_HH_

#include "ignition/rendering/base/BaseNode.hh"

#include "LuxCoreEngineObject.hh"
#include "LuxCoreEngineRenderTypes.hh"

namespace ignition {
namespace rendering {
inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
class LuxCoreEngineNode : public BaseNode<LuxCoreEngineObject> {
protected:
  LuxCoreEngineNode();

public:
  virtual ~LuxCoreEngineNode();

public:
  virtual bool HasParent() const override;

public:
  virtual NodePtr Parent() const override;

public:
  virtual void PreRender() override;

public:
  virtual math::Vector3d LocalPosition() const override;

public:
  virtual math::Vector3d LocalScale() const override;

public:
  virtual bool InheritScale() const override;

public:
  virtual void SetInheritScale(bool _inherit) override;

protected:
  virtual void SetLocalScaleImpl(const math::Vector3d &_scale) override;

protected:
  virtual math::Pose3d RawLocalPose() const override;

protected:
  virtual void SetRawLocalPose(const math::Pose3d &_pose) override;

protected:
  virtual void SetParent(LuxCoreEngineNodePtr _parent);

protected:
  virtual void Init();

protected:
  virtual NodeStorePtr Children() const override;

protected:
  virtual bool AttachChild(NodePtr _child) override;

protected:
  virtual bool DetachChild(NodePtr _child) override;

protected:
  LuxCoreEngineNodePtr parent;

protected:
  math::Pose3d pose;

protected:
  bool poseDirty;

  // protected: LuxCoreStorePtr children;

protected:
  math::Vector3d scale = math::Vector3d::One;

protected:
  bool inheritScale = true;
};
} // namespace IGNITION_RENDERING_VERSION_NAMESPACE
} // namespace rendering
} // namespace ignition

#endif
