#ifndef IGNITION_RENDERING_LUXCOREENGINE_LUXCOREENGINEOBJECT_HH_
#define IGNITION_RENDERING_LUXCOREENGINE_LUXCOREENGINEOBJECT_HH_

#include "ignition/rendering/base/BaseObject.hh"
#include "ignition/rendering/base/BaseScene.hh"

#include "LuxCoreEngineRenderTypes.hh"

namespace ignition {
namespace rendering {
inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
class LuxCoreEngineObject : public BaseObject {
protected:
  LuxCoreEngineObject();

public:
  virtual ~LuxCoreEngineObject();

  // This functions seems to be the issue, but for some reason i cant return
  // this->scene
public:
  virtual ScenePtr Scene() const;

protected:
  LuxCoreEngineScenePtr scene;

private:
  friend class LuxCoreEngineScene;
};
} // namespace IGNITION_RENDERING_VERSION_NAMESPACE
} // namespace rendering
} // namespace ignition

#endif
