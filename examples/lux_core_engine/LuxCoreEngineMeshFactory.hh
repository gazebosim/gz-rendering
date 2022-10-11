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
#ifndef IGNITION_RENDERING_LUXCOREENGINE_LUXCOREENGINEMESHFACTORY_HH_
#define IGNITION_RENDERING_LUXCOREENGINE_LUXCOREENGINEMESHFACTORY_HH_

#include <luxcore/luxcore.h>

#include "LuxCoreEngineMesh.hh"
#include "LuxCoreEngineRenderTypes.hh"

namespace ignition {
namespace rendering {
inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
/// @brief LuxCorEngineMeshFactory This class creates and returns a mesh using
///        data from the scene. Using the mesh descriptor, the class sets mesh name and 
///        its properties. 
class LuxCoreEngineMeshFactory
{

  public:LuxCoreEngineMeshFactory(LuxCoreEngineScenePtr _scene);

  /// @brief Create Mesh
  /// @param _desc Mesh descriptor
  /// @param _name mesh name
  /// @return 
  public: LuxCoreEngineMeshPtr Create(const MeshDescriptor &_desc,
                                      const std::string &_name);

  protected: LuxCoreEngineScenePtr scene;
};

} // namespace IGNITION_RENDERING_VERSION_NAMESPACE
} // namespace rendering
} // namespace ignition

#endif
