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
#ifndef IGNITION_RENDERING_LUXCOREENGINE_LUXCOREENGINEVISUAL_HH_
#define IGNITION_RENDERING_LUXCOREENGINE_LUXCOREENGINEVISUAL_HH_

#include "ignition/rendering/base/BaseVisual.hh"

#include "LuxCoreEngineGeometry.hh"
#include "LuxCoreEngineNode.hh"
#include "LuxCoreEngineRenderTypes.hh"

namespace ignition {
namespace rendering {
inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {

class LuxCoreEngineVisual : public BaseVisual<LuxCoreEngineNode> {

  public: void SetLocalPosition(double _x, double _y, double _z);
  
  public: void SetLocalScale(double _x, double _y, double _z);
  
  public: void SetLocalRotation(double _r, double _p, double _y);
  
  public: void SetMaterial(MaterialPtr _material, bool _unique = true);
  
  protected: GeometryStorePtr Geometries() const;
  
  protected: bool AttachGeometry(GeometryPtr _geometry);
  
  protected: bool DetachGeometry(GeometryPtr _geometry);
  
  protected: void Init();
  
  private: void CreateStorage();
  
  private: LuxCoreEngineVisualPtr SharedThis();
  
  protected: LuxCoreEngineGeometryStorePtr geometries;
  
  private: friend class LuxCoreEngineScene;
};

} // namespace IGNITION_RENDERING_VERSION_NAMESPACE
} // namespace rendering
} // namespace ignition

#endif
