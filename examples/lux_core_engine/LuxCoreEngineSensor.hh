#ifndef IGNITION_RENDERING_LUXCOREENGINE_LUXCOREENGINESENSOR_HH_
#define IGNITION_RENDERING_LUXCOREENGINE_LUXCOREENGINESENSOR_HH_

#include "ignition/rendering/base/BaseSensor.hh"

#include "LuxCoreEngineNode.hh"
#include "LuxCoreEngineRenderTypes.hh"

namespace ignition {
namespace rendering {
inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
class LuxCoreEngineSensor : public BaseSensor<LuxCoreEngineNode> {
protected:
  LuxCoreEngineSensor();

public:
  virtual ~LuxCoreEngineSensor();
};
} // namespace IGNITION_RENDERING_VERSION_NAMESPACE
} // namespace rendering
} // namespace ignition

#endif
