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
#ifndef GZ_RENDERING_LUXCOREENGINE_LUXCOREENGINEOBJECT_HH_
#define GZ_RENDERING_LUXCOREENGINE_LUXCOREENGINEOBJECT_HH_

#include "gz/rendering/base/BaseObject.hh"
#include "gz/rendering/base/BaseScene.hh"

#include "LuxCoreEngineRenderTypes.hh"

namespace gz {
namespace rendering {
inline namespace GZ_RENDERING_VERSION_NAMESPACE {

/// \brief LuxCore implementation of the object class
class LuxCoreEngineObject : public BaseObject
{
  /// \brief Constructor
  protected: LuxCoreEngineObject();

  /// \brief Destructor
  public: virtual ~LuxCoreEngineObject();

  // Documentation inherited.
  public: virtual ScenePtr Scene() const override;

  protected: LuxCoreEngineScenePtr scene;

  private: friend class LuxCoreEngineScene;
};

} // namespace GZ_RENDERING_VERSION_NAMESPACE
} // namespace rendering
} // namespace gz

#endif
