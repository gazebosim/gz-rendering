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
#ifndef IGNITION_RENDERING_OGRE2_OGRE2MESH_HH_
#define IGNITION_RENDERING_OGRE2_OGRE2MESH_HH_

#include <string>
#include <vector>
#include "ignition/rendering/base/BaseMesh.hh"
#include "ignition/rendering/ogre2/Ogre2Geometry.hh"
#include "ignition/rendering/ogre2/Ogre2Object.hh"
#include "ignition/rendering/ogre2/Ogre2RenderTypes.hh"

namespace Ogre
{
  class Item;
  class SubItem;
}

namespace ignition
{
  namespace rendering
  {
    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
    //
    /// \brief Ogre2.x implementation of the mesh class
    class IGNITION_RENDERING_OGRE2_VISIBLE Ogre2Mesh :
      public BaseMesh<Ogre2Geometry>
    {
      /// \brief Constructor
      protected: Ogre2Mesh();

      /// \brief Destructor
      public: virtual ~Ogre2Mesh();

      // Documentation inherited
      public: virtual void Destroy() override;

      // Documentation inherited
      public: virtual Ogre::MovableObject *OgreObject() const override;

      /// \brief Get a list of submeshes in this mesh
      protected: virtual SubMeshStorePtr SubMeshes() const override;

      /// \brief Store containing all the submeshes
      protected: Ogre2SubMeshStorePtr subMeshes;

      /// \brief Pointer to the ogre item object
      protected: Ogre::Item *ogreItem = nullptr;

      /// \brief Make scene our friend so it can create an ogre2 mesh
      private: friend class Ogre2Scene;

      /// \brief Make mesh factory our friend so it can create an ogre2 mesh
      private: friend class Ogre2MeshFactory;
    };

    /// \brief Ogre2.x implementation of the submesh class
    class IGNITION_RENDERING_OGRE2_VISIBLE Ogre2SubMesh :
      public BaseSubMesh<Ogre2Object>
    {
      /// \brief Constructor
      protected: Ogre2SubMesh();

      /// \brief Destructor
      public: virtual ~Ogre2SubMesh();

      /// \brief Get internal ogre subitem created from this submesh
      public: virtual Ogre::SubItem *Ogre2SubItem() const;

      /// \brief Helper function for setting the material to use
      /// \param[in] _material Material to be assigned to the submesh
      protected: virtual void SetMaterialImpl(MaterialPtr _material);

      /// \brief Initialize the submesh
      protected: virtual void Init();

      /// \brief Ogre subitem representing the submesh
      protected: Ogre::SubItem *ogreSubItem = nullptr;

      /// \brief Make scene our friend so it can create an ogre2 mesh
      private: friend class Ogre2Scene;

      /// \brief Make submesh factory our friend so it can create an
      /// ogre2 submesh
      private: friend class Ogre2SubMeshStoreFactory;
    };
    }
  }
}
#endif
