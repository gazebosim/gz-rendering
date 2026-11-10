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
      protected: FilamentNode();

      public: virtual ~FilamentNode();

      public: virtual bool HasParent() const override;

      public: virtual NodePtr Parent() const override;

      /// \brief Entity carrying this node's transform component
      public: ::utils::Entity Entity() const;

      public: virtual void Destroy() override;

      public: virtual math::Vector3d LocalScale() const override;

      public: virtual bool InheritScale() const override;

      public: virtual void SetInheritScale(bool _inherit) override;

      protected: virtual void SetLocalScaleImpl(
                     const math::Vector3d &_scale) override;

      protected: virtual math::Pose3d RawLocalPose() const override;

      protected: virtual void SetRawLocalPose(const math::Pose3d &_pose)
                     override;

      protected: virtual NodeStorePtr Children() const override;

      protected: virtual bool AttachChild(NodePtr _child) override;

      protected: virtual bool DetachChild(NodePtr _child) override;

      protected: virtual void SetParent(FilamentNodePtr _parent);

      /// \brief True if the parent chain reaches the scene's root visual
      protected: bool InSceneGraph() const;

      /// \brief Add (or remove) the renderables of this node's visual
      /// descendants to (from) the filament scene, through any node type
      /// (a light's visual is drawn like ogre2's)
      /// \param[in] _inScene True to add, false to remove
      protected: virtual void SetInScene(bool _inScene);

      protected: virtual void Init() override;

      /// \brief Push pose * scale into the filament transform component
      protected: void UpdateTransform();

      protected: FilamentNodePtr parent;

      protected: ::utils::Entity entity;

      protected: math::Pose3d pose;

      protected: math::Vector3d scale = math::Vector3d::One;

      protected: bool inheritScale = true;

      protected: FilamentNodeStorePtr children;

      private: FilamentNodePtr SharedThis();

      private: friend class FilamentScene;
    };
    }
  }
}
#endif
