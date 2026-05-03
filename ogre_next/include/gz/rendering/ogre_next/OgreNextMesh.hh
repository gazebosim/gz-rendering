/*
 * Copyright (C) 2018 Open Source Robotics Foundation
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
#ifndef GZ_RENDERING_OGRE_NEXT_OGRE_NEXTMESH_HH_
#define GZ_RENDERING_OGRE_NEXT_OGRE_NEXTMESH_HH_

#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include "gz/rendering/base/BaseMesh.hh"
#include "gz/rendering/ogre_next/OgreNextGeometry.hh"
#include "gz/rendering/ogre_next/OgreNextObject.hh"
#include "gz/rendering/ogre_next/OgreNextRenderTypes.hh"

namespace Ogre
{
  class Item;
  class SubItem;
}

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    //
    // forward declaration
    class OgreNextMeshPrivate;
    class OgreNextSubMeshPrivate;

    /// \brief OgreNext.x implementation of the mesh class
    class GZ_RENDERING_OGRE_NEXT_VISIBLE OgreNextMesh :
      public BaseMesh<OgreNextGeometry>
    {
      /// \brief Constructor
      protected: OgreNextMesh();

      /// \brief Destructor
      public: virtual ~OgreNextMesh();

      // Documentation inherited
      public: virtual void Destroy() override;

      // Documentation inherited.
      public: virtual bool HasSkeleton() const override;

      // Documentation inherited.
      public: virtual std::map<std::string, math::Matrix4d>
                          SkeletonLocalTransforms() const override;

      // Documentation inherited.
      public: virtual void SetSkeletonLocalTransforms(
            const std::map<std::string, math::Matrix4d> &_tfs) override;

      // Documentation inherited.
      public: virtual std::unordered_map<std::string, float>
                          SkeletonWeights() const override;

      // Documentation inherited.
      public: virtual void SetSkeletonWeights(
            const std::unordered_map<std::string, float> &_weights) override;

      // Documentation inherited.
      public: virtual void SetSkeletonAnimationEnabled(const std::string &_name,
            bool _enabled, bool _loop = true, float _weight = 1.0) override;

      // Documentation inherited.
      public: virtual bool SkeletonAnimationEnabled(const std::string &_name)
            const override;

      // Documentation inherited.
      public: virtual void UpdateSkeletonAnimation(
            std::chrono::steady_clock::duration _time) override;

      // Documentation inherited
      public: virtual Ogre::MovableObject *OgreObject() const override;

      /// \brief Get a list of submeshes in this mesh
      protected: virtual SubMeshStorePtr SubMeshes() const override;

      /// \brief Store containing all the submeshes
      protected: OgreNextSubMeshStorePtr subMeshes;

      /// \brief Pointer to the ogre item object
      protected: Ogre::Item *ogreItem = nullptr;

      /// \brief Make scene our friend so it can create an ogre_next mesh
      private: friend class OgreNextScene;

      /// \brief Make mesh factory our friend so it can create an ogre_next mesh
      private: friend class OgreNextMeshFactory;

      /// \brief Pointer to private data
      private: std::unique_ptr<OgreNextMeshPrivate> dataPtr;
    };

    /// \brief OgreNext.x implementation of the submesh class
    class GZ_RENDERING_OGRE_NEXT_VISIBLE OgreNextSubMesh :
      public BaseSubMesh<OgreNextObject>
    {
      /// \brief Constructor
      protected: OgreNextSubMesh();

      /// \brief Destructor
      public: virtual ~OgreNextSubMesh();

      // Documentation inherited
      public: virtual void Destroy() override;

      /// \brief Set the name of the mesh stored in OgreNext
      /// \param[in] _name Name of the mesh
      public: void SetMeshName(const std::string &_name);

      /// \brief Get internal ogre subitem created from this submesh
      public: virtual Ogre::SubItem *OgreNextSubItem() const;

      /// \brief Helper function for setting the material to use
      /// \param[in] _material Material to be assigned to the submesh
      protected: virtual void SetMaterialImpl(MaterialPtr _material) override;

      /// \brief Initialize the submesh
      protected: virtual void Init() override;

      /// \brief Ogre subitem representing the submesh
      protected: Ogre::SubItem *ogreSubItem = nullptr;

      /// \brief Make scene our friend so it can create an ogre_next mesh
      private: friend class OgreNextScene;

      /// \brief Make submesh factory our friend so it can create an
      /// ogre_next submesh
      private: friend class OgreNextSubMeshStoreFactory;

      /// \brief Pointer to private data
      private: std::unique_ptr<OgreNextSubMeshPrivate> dataPtr;
    };
    }
  }
}
#endif
