/*
 * Copyright (C) 2022 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#ifndef IGNITION_RENDERING_LUXCOREENGINE_LUXCOREENGINECAMERA_HH_
#define IGNITION_RENDERING_LUXCOREENGINE_LUXCOREENGINECAMERA_HH_

#include <luxcore/luxcore.h>

#include "ignition/rendering/base/BaseCamera.hh"

#include "LuxCoreEngineRenderTarget.hh"
#include "LuxCoreEngineRenderTypes.hh"
#include "LuxCoreEngineSensor.hh"

namespace ignition {
namespace rendering {

inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {

class LuxCoreEngineCamera : public BaseCamera<LuxCoreEngineSensor> {

  public: LuxCoreEngineCamera();

  public: virtual ~LuxCoreEngineCamera();

  public: virtual void Render();

  public: virtual void Update();

  public: virtual void SetLocalPosition(double _x, double _y, double _z);

  public: virtual void SetLocalRotation(double _r, double _p, double _y);

  public: virtual void SetHFOV(const math::Angle &_hfov);

  protected: virtual RenderTargetPtr RenderTarget() const;

  protected: LuxCoreEngineRenderTargetPtr renderTarget;

  protected: luxcore::RenderSession *renderSessionLux;

  protected: float localPositionX, localPositionY, localPositionZ;

  protected: float localRotationR, localRotationP, localRotationY;
};

} // namespace IGNITION_RENDERING_VERSION_NAMESPACE
} // namespace rendering
} // namespace ignition

#endif