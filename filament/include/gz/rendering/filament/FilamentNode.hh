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
#ifndef GZ_RENDERING_FILAMENT_FILAMENTNODE_HH_
#define GZ_RENDERING_FILAMENT_FILAMENTNODE_HH_

#include <utils/Entity.h>

#include "gz/rendering/config.hh"
#include "gz/rendering/base/BaseNode.hh"
#include "gz/rendering/filament/FilamentObject.hh"
#include "gz/rendering/filament/FilamentRenderTypes.hh"
#include "gz/rendering/filament/Export.hh"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    //
    /// \brief Scene-graph node: a filament entity with a transform component
    class GZ_RENDERING_FILAMENT_VISIBLE FilamentNode :
      public BaseNode<FilamentObject>
    {
      /// \brief Constructor
      protected: FilamentNode() = default;

      /// \brief Destructor
      public: virtual ~FilamentNode() = default;

      // Documentation inherited
      public: virtual bool HasParent() const override;

      // Documentation inherited
      public: virtual NodePtr Parent() const override;

      /// \brief Get the entity carrying this node's transform component.
      /// \return The entity, null before Init and after Destroy.
      public: ::utils::Entity Entity() const;

      // Documentation inherited
      public: virtual void Destroy() override;

      // Documentation inherited
      public: virtual math::Vector3d LocalScale() const override;

      // Documentation inherited
      public: virtual bool InheritScale() const override;

      // Documentation inherited
      public: virtual void SetInheritScale(bool _inherit) override;

      // Documentation inherited
      protected: virtual void SetLocalScaleImpl(
                     const math::Vector3d &_scale) override;

      /// \brief Get the local pose of this node as stored, relative to the
      /// parent and without the origin offset that LocalPose adds.
      /// \return The stored local pose.
      protected: virtual math::Pose3d RawLocalPose() const override;

      /// \brief Store the local pose of this node, without the origin
      /// offset, and push it into the filament transform component.
      /// \param[in] _pose Local pose relative to the parent, without the
      /// origin offset.
      protected: virtual void SetRawLocalPose(const math::Pose3d &_pose)
                     override;

      /// \brief Get the store of children of this node.
      /// \return The store of children, null before Init.
      protected: virtual NodeStorePtr Children() const override;

      /// \brief Parent _child's entity to this node's. Fails with an error
      /// if _child was created by another render engine or is an ancestor
      /// of this node.
      /// \param[in] _child Node to attach.
      /// \return True if _child was attached, false otherwise.
      protected: virtual bool AttachChild(NodePtr _child) override;

      /// \brief Unparent _child's entity from this node's. Fails with an
      /// error if _child was created by another render engine.
      /// \param[in] _child Node to detach.
      /// \return True if _child was detached, false otherwise.
      protected: virtual bool DetachChild(NodePtr _child) override;

      /// \brief Store the parent node. The filament parenting is done by
      /// AttachChild and DetachChild.
      /// \param[in] _parent New parent node, null when detached.
      protected: virtual void SetParent(FilamentNodePtr _parent);

      /// \brief True if the parent chain reaches the scene's root visual
      /// \return True if this node, or one of its ancestors, is the scene's
      /// root visual. False if it has no scene.
      protected: bool InSceneGraph() const;

      /// \brief Add (or remove) the renderables of this node's visual
      /// descendants to (from) the filament scene, through any node type
      /// (a light's visual is drawn like ogre2's)
      /// \param[in] _inScene True to add, false to remove
      protected: virtual void SetInScene(bool _inScene);

      /// \brief Create the entity with its transform component and the
      /// store of children.
      protected: virtual void Init() override;

      /// \brief Push pose * scale into the filament transform component,
      /// without the parent's scale if this node does not inherit it
      protected: void UpdateTransform();

      /// \brief UpdateTransform the descendants that do not inherit scale
      /// and whose parent's world scale depends on this node's
      protected: void UpdateScaleDependents();

      /// \brief Parent node, null if there is none.
      protected: FilamentNodePtr parent;

      /// \brief Entity carrying the transform component, null before Init
      /// and after Destroy.
      protected: ::utils::Entity entity;

      /// \brief Local pose relative to the parent, see RawLocalPose.
      protected: math::Pose3d pose;

      /// \brief Local scale.
      protected: math::Vector3d scale = math::Vector3d::One;

      /// \brief Whether the parent's scale applies to this node.
      protected: bool inheritScale = true;

      /// \brief Child nodes, created in Init.
      protected: FilamentNodeStorePtr children;

      /// \brief Get a shared pointer to this node.
      /// \return Shared pointer to this node.
      private: FilamentNodePtr SharedThis();

      private: friend class FilamentScene;
    };
    }
  }
}
#endif
