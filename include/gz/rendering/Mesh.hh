/*
 * Copyright (C) 2015 Open Source Robotics Foundation
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
#ifndef GZ_RENDERING_MESH_HH_
#define GZ_RENDERING_MESH_HH_

#include <map>
#include <string>
#include <gz/math/Matrix4.hh>
#include "gz/rendering/config.hh"
#include "gz/rendering/Geometry.hh"
#include "gz/rendering/Object.hh"

namespace ignition
{
  namespace rendering
  {
    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
    //
    /// \class Mesh Mesh.hh ignition/rendering/Mesh
    /// \brief Represents a collection of mesh geometries
    class IGNITION_RENDERING_VISIBLE Mesh :
      public virtual Geometry
    {
      /// \brief Destructor
      public: virtual ~Mesh() { }

      /// \brief Check whether the mesh has skeleton
      /// \return True if the mesh has skeleton
      public: virtual bool HasSkeleton() const = 0;

      /// \brief Get the skeleton local transforms
      /// \return Map of skeleton local transformations
      /// * Map holding:
      ///     * Skeleton node names
      ///     * Local transformations of the skeleton nodes
      public: virtual std::map<std::string, math::Matrix4d>
            SkeletonLocalTransforms() const = 0;

      /// \brief Set transforms for the skeleton
      /// \param[in] _tfs Map of skeleton local transformations
      /// * Map holding:
      ///     * Skeleton node names
      ///     * Local transformations of the skeleton nodes
      /// \return True if all the transformations are set to the skeleton
      public: virtual void SetSkeletonLocalTransforms(
            const std::map<std::string, math::Matrix4d> &_tfs) = 0;

      /// \brief Get the sub-mesh count
      /// \return The sub-mesh count
      public: virtual unsigned int SubMeshCount() const = 0;

      /// \brief Determine if has given sub-mesh
      /// \param[in] _subMesh Sub-mesh in question
      /// \return True if has given sub-mesh
      public: virtual bool HasSubMesh(ConstSubMeshPtr _subMesh) const = 0;

      /// \brief Determine if has sub-mesh with given name
      /// \param[in] _name Name of sub-mesh in question
      /// \return True if has sub-mesh with given name
      public: virtual bool HasSubMeshName(const std::string &_name) const = 0;

      /// \brief Get sub-mesh with given name
      /// \param[in] _name Name of sub-mesh
      /// \return The sub-mesh with the given name
      public: virtual SubMeshPtr SubMeshByName(
                  const std::string &_name) const = 0;

      /// \brief Get sub-mesh at given index
      /// \param[in] _index Index of sub-mesh
      /// \return The sub-mesh at the given index
      public: virtual SubMeshPtr SubMeshByIndex(
                  unsigned int _index) const = 0;
    };

    /// \class SubMesh Mesh.hh ignition/rendering/Mesh.hh
    /// \brief Represents a single mesh geometry
    class IGNITION_RENDERING_VISIBLE SubMesh :
      public virtual Object
    {
      /// \brief Deconstructor
      public: virtual ~SubMesh() { }

      /// \brief Get the currently assigned material
      /// \return The currently assigned material
      public: virtual MaterialPtr Material() const = 0;

      /// \brief Set the materials of this SubMesh. The specified material
      /// will be retrieved from the parent Scene. If no material is registered
      /// by the given name, no work will be done.
      /// \param[in] _name Name of registered Material
      /// \param[in] _unique True if the specified material should be cloned
      public: virtual void SetMaterial(const std::string &_name,
                  bool _unique = true) = 0;

      /// \brief Set the materials of this SubMesh
      /// \param[in] _material New Material to be assigned
      /// \param[in] _unique True if the given material should be cloned
      public: virtual void SetMaterial(MaterialPtr _material,
                  bool _unique = true) = 0;
    };
    }
  }
}
#endif
