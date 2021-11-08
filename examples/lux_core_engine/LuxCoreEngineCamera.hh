#ifndef IGNITION_RENDERING_LUXCOREENGINE_LUXCOREENGINECAMERA_HH_
#define IGNITION_RENDERING_LUXCOREENGINE_LUXCOREENGINECAMERA_HH_

#include "ignition/rendering/base/BaseCamera.hh"

#include "LuxCoreEngineRenderTarget.hh"
#include "LuxCoreEngineRenderTypes.hh"
#include "LuxCoreEngineSensor.hh"

#include "luxcore/luxcore.h"

namespace ignition {
namespace rendering {
inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
class LuxCoreEngineCamera : public BaseCamera<LuxCoreEngineSensor> {
public:
  LuxCoreEngineCamera();

public:
  virtual ~LuxCoreEngineCamera();

public:
  virtual void Render();

public:
  virtual void Update();

public:
  virtual void SetLocalPosition(double _x, double _y, double _z);

public:
  virtual void SetLocalRotation(double _r, double _p, double _y);

public:
  virtual void SetHFOV(const math::Angle &_hfov);

protected:
  virtual RenderTargetPtr RenderTarget() const;

protected:
  LuxCoreEngineRenderTargetPtr renderTarget;

protected:
  luxcore::RenderSession *renderSessionLux;

protected:
  float localPositionX, localPositionY, localPositionZ;

protected:
  float localRotationR, localRotationP, localRotationY;
};
} // namespace IGNITION_RENDERING_VERSION_NAMESPACE
} // namespace rendering
} // namespace ignition

#endif
