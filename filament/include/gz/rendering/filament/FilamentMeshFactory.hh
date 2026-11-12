/*
 * Copyright (C) 2026 Open Source Robotics Foundation
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
#ifndef GZ_RENDERING_FILAMENT_FILAMENTMESHFACTORY_HH_
#define GZ_RENDERING_FILAMENT_FILAMENTMESHFACTORY_HH_

#include <gz/common/SubMesh.hh>
#include <gz/math/Vector3.hh>

#include "gz/rendering/config.hh"
#include "gz/rendering/MeshDescriptor.hh"
#include "gz/rendering/filament/FilamentRenderTypes.hh"
#include "gz/rendering/filament/Export.hh"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    //
    /// \brief Uploads gz::common meshes into filament vertex/index buffers
    /// and builds the renderable component of a FilamentMesh.
    class GZ_RENDERING_FILAMENT_VISIBLE FilamentMeshFactory
    {
      /// \brief Constructor
      /// \param[in] _scene Scene the meshes are loaded into
      public: explicit FilamentMeshFactory(FilamentScenePtr _scene);

      /// \brief Destructor
      public: virtual ~FilamentMeshFactory() = default;

      /// \brief Fill _mesh (already initialized, with an entity) from _desc.
      /// \param[in] _mesh Mesh to fill
      /// \param[in] _desc Descriptor of the mesh to load
      /// \return True on success, false (with an error logged) if the
      /// descriptor has no mesh or none of its submeshes can be drawn
      public: bool Load(FilamentMeshPtr _mesh, const MeshDescriptor &_desc);

      /// \brief Get what to subtract from the vertices of _sub: its bounding
      /// box center if _desc.centerSubMesh (common::SubMesh::Center, as
      /// ogre2), else zero.
      /// \param[in] _sub Submesh whose vertices are placed
      /// \param[in] _desc Descriptor the submesh was loaded with
      /// \return The center of _sub, or zero if not centering
      public: static math::Vector3d SubMeshCenter(const common::SubMesh &_sub,
          const MeshDescriptor &_desc);

      /// \brief Scene the meshes are loaded into
      protected: FilamentScenePtr scene;
    };
    }
  }
}
#endif
