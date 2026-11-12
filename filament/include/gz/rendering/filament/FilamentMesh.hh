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
#ifndef GZ_RENDERING_FILAMENT_FILAMENTMESH_HH_
#define GZ_RENDERING_FILAMENT_FILAMENTMESH_HH_

#include <chrono>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <utils/Entity.h>

#include <gz/common/Skeleton.hh>

#include "gz/rendering/config.hh"
#include "gz/rendering/base/BaseMesh.hh"
#include "gz/rendering/filament/FilamentGeometry.hh"
#include "gz/rendering/filament/FilamentObject.hh"
#include "gz/rendering/filament/FilamentRenderTypes.hh"
#include "gz/rendering/filament/Export.hh"

namespace filament
{
  class IndexBuffer;
  class MaterialInstance;
  class VertexBuffer;
}

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    //
    /// \brief A mesh is one filament renderable entity; each submesh is one
    /// primitive of that renderable.
    class GZ_RENDERING_FILAMENT_VISIBLE FilamentMesh :
      public BaseMesh<FilamentGeometry>
    {
      /// \brief Constructor
      protected: FilamentMesh();

      /// \brief Destructor
      public: virtual ~FilamentMesh();

      // Documentation inherited
      public: virtual void Destroy() override;

      // Documentation inherited
      public: virtual ::utils::Entity Entity() const override;

      // Documentation inherited
      public: virtual void SetLayers(uint8_t _layers) override;

      // Documentation inherited
      public: virtual void PreRender() override;

      // Documentation inherited (skeleton API, FilamentMeshSkeleton.cc)
      public: virtual bool HasSkeleton() const override;

      // Documentation inherited
      public: virtual std::map<std::string, math::Matrix4d>
          SkeletonLocalTransforms() const override;

      // Documentation inherited
      public: virtual void SetSkeletonLocalTransforms(
          const std::map<std::string, math::Matrix4d> &_tfs) override;

      // Documentation inherited
      public: virtual std::unordered_map<std::string, float>
          SkeletonWeights() const override;

      // Documentation inherited
      public: virtual void SetSkeletonWeights(
          const std::unordered_map<std::string, float> &_weights) override;

      // Documentation inherited
      public: virtual void SetSkeletonAnimationEnabled(
          const std::string &_name, bool _enabled, bool _loop = true,
          float _weight = 1.0) override;

      // Documentation inherited
      public: virtual bool SkeletonAnimationEnabled(
          const std::string &_name) const override;

      // Documentation inherited
      public: virtual void UpdateSkeletonAnimation(
          std::chrono::steady_clock::duration _time) override;

      /// \brief Set up the bone state of a skinned mesh (mesh factory).
      /// \param[in] _skeleton Skeleton of the mesh.
      /// \param[in] _gpu True if the renderable was built with
      /// skinning(bone count), so the bone matrices are uploaded to the GPU.
      public: void InitSkeleton(common::SkeletonPtr _skeleton, bool _gpu);

      /// \brief Upload the bone matrices if the pose changed
      private: void UpdateBones();

      /// \brief Temporarily render every submesh with _instance. With
      /// _alphaTest, alpha tested submeshes discard their cut-out texels in
      /// it too (when its material has the alphaMap / alphaThreshold
      /// parameters), like ogre2's sensor passes.
      /// \param[in] _instance Material instance to render with.
      /// \param[in] _alphaTest Whether alpha tested submeshes keep their
      /// alpha test in _instance.
      public: virtual void OverrideMaterial(
          filament::MaterialInstance *_instance, bool _alphaTest = true);

      /// \brief Temporarily render submesh _index with _instance.
      /// \param[in] _index Index of the submesh.
      /// \param[in] _instance Material instance to render with.
      public: virtual void OverrideMaterial(unsigned int _index,
                  filament::MaterialInstance *_instance);

      /// \brief Whether a submesh draws an alpha tested copy of the
      /// override instance. Copies are new instances: their parameters
      /// reach the GPU when the next frame begins.
      /// \return True if a submesh draws an alpha tested copy.
      public: bool HasAlphaTestedOverride() const;

      /// \brief Undo OverrideMaterial.
      public: virtual void RestoreMaterials();

      /// \brief Rebind the submeshes bound to _old to _new: the material
      /// owning _old is destroyed while still in use (ogre2 falls back to
      /// its default datablock the same way).
      /// \param[in] _old Material instance to replace.
      /// \param[in] _new Material instance to bind instead.
      public: void ReplaceInstance(filament::MaterialInstance *_old,
          filament::MaterialInstance *_new);

      /// \brief Draw the triangle submeshes as their edges (LINES), or back
      /// as triangles. ogre2's wireframe polygon mode, which filament lacks.
      /// \param[in] _wireframe True to draw edges, false to draw triangles.
      public: void SetWireframe(bool _wireframe);

      /// \brief Depth-only copy of the triangles drawn while in wireframe
      /// (null otherwise): it hides the back edges.
      /// \return The entity of the depth-only copy, null if there is none.
      public: ::utils::Entity HiddenSurface() const;

      /// \brief Register a submesh built by the mesh factory.
      /// \param[in] _subMesh Submesh to add to this mesh.
      public: void AddSubMesh(FilamentSubMeshPtr _subMesh);

      /// \brief Heightmap terrain, which ogre2 draws with Terra rather than
      /// as an Item: thermal draws it white unless it is a heat source, and
      /// the bounding box camera treats it as an unlabeled occluder.
      public: bool terrain{false};

      /// \brief Get the store of submeshes of this mesh.
      protected: virtual SubMeshStorePtr SubMeshes() const override;

      /// \brief Create the mesh entity (with a transform) and register the
      /// mesh with the scene.
      protected: virtual void Init() override;

      /// \brief Filament entity of the renderable, null before Init and
      /// after Destroy.
      protected: ::utils::Entity entity;

      /// \brief See HiddenSurface()
      private: ::utils::Entity hiddenSurface;

      /// \brief Destroy hiddenSurface
      private: void DestroyHiddenSurface();

      /// \brief Store of the submeshes of this mesh.
      protected: FilamentSubMeshStorePtr subMeshes;

      /// \brief Bone and animation state of a skinned mesh, defined in
      /// FilamentMeshSkeleton.cc.
      private: struct SkeletonState;

      /// \brief Bone and animation state, null without a skeleton
      /// (shared_ptr: deletable where SkeletonState is incomplete).
      private: std::shared_ptr<SkeletonState> skeleton;

      private: friend class FilamentScene;
      private: friend class FilamentMeshFactory;
      private: friend class FilamentSubMesh;
    };

    /// \brief Filament implementation of a submesh: one primitive of the
    /// renderable of its FilamentMesh.
    class GZ_RENDERING_FILAMENT_VISIBLE FilamentSubMesh :
      public BaseSubMesh<FilamentObject>
    {
      /// \brief Constructor
      protected: FilamentSubMesh() = default;

      /// \brief Destructor
      public: virtual ~FilamentSubMesh() = default;

      // Documentation inherited
      public: virtual void Destroy() override;

      // Documentation inherited
      public: virtual void PreRender() override;

      // Documentation inherited
      protected: virtual void SetMaterialImpl(MaterialPtr _material) override;

      /// \brief Bind _instance as the primitive's material instance. An
      /// override (not the material's own instance) of an alpha tested
      /// material is bound as an alpha tested copy, see OverrideMaterial.
      protected: void ApplyInstance(filament::MaterialInstance *_instance,
          bool _alphaTest = true);

      /// \brief Mesh this submesh belongs to.
      protected: FilamentMesh *mesh{nullptr};

      /// \brief Index of this submesh's primitive in the mesh's renderable.
      protected: size_t index{0u};

      /// \brief Vertex buffer of the primitive, owned by geometry.
      protected: filament::VertexBuffer *vertexBuffer{nullptr};

      /// \brief Index buffer of the primitive, owned by geometry.
      protected: filament::IndexBuffer *indexBuffer{nullptr};

      /// \brief Owns vertexBuffer / indexBuffer, which are shared by every
      /// submesh built from the same common::SubMesh
      protected: std::shared_ptr<void> geometry;

      /// \brief Index of the common::SubMesh this was built from
      protected: unsigned int subMeshIndex{0u};

      /// \brief Edge (LINES) index buffer while drawn as wireframe
      protected: filament::IndexBuffer *lineBuffer{nullptr};

      /// \brief Instance currently bound on the GPU side
      protected: filament::MaterialInstance *bound{nullptr};

      /// \brief Alpha tested copy of the last override, owned
      protected: filament::MaterialInstance *alphaCopy{nullptr};

      /// \brief Material() as a FilamentMaterial (null if it is none), kept
      /// by SetMaterialImpl so the per frame paths skip the cast
      protected: FilamentMaterial *filamentMaterial{nullptr};

      private: friend class FilamentScene;
      private: friend class FilamentMesh;
      private: friend class FilamentDynamicMesh;
      private: friend class FilamentMeshFactory;
    };
    }
  }
}
#endif
