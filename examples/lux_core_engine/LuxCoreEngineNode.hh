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
#ifndef IGNITION_RENDERING_LUXCOREENGINE_LUXCOREENGINENODE_HH_
#define IGNITION_RENDERING_LUXCOREENGINE_LUXCOREENGINENODE_HH_

#include "ignition/rendering/base/BaseNode.hh"

#include "LuxCoreEngineObject.hh"
#include "LuxCoreEngineRenderTypes.hh"

namespace ignition {
namespace rendering {
inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {

class LuxCoreEngineNode : public BaseNode<LuxCoreEngineObject> {

  protected: LuxCoreEngineNode();

  public: virtual ~LuxCoreEngineNode();
  // Documentation inherited.
  public: virtual bool HasParent() const override;
  // Documentation inherited.
  public: virtual NodePtr Parent() const override;
  // Documentation inherited.
  public: virtual void PreRender() override;
  // Documentation inherited.
  public: virtual math::Vector3d LocalPosition() const override;
  // Documentation inherited.
  public: virtual math::Vector3d LocalScale() const override;
  /// @brief Inherit Scale  
  public: virtual bool InheritScale() const override;
  /// @brief Set Inherit Scale 
  /// @param _inherit 
  public: virtual void SetInheritScale(bool _inherit) override;
  /// @brief Set Local scale implementation 
  protected: virtual void SetLocalScaleImpl(const math::Vector3d &_scale) override;
  /// @brief Get Raw local pose 
  protected: virtual math::Pose3d RawLocalPose() const override;
  /// @brief Set Raw local Pose
  /// @param _pose 
  protected: virtual void SetRawLocalPose(const math::Pose3d &_pose) override;
  // Documentation inherited.
  protected: virtual void SetParent(LuxCoreEngineNodePtr _parent);
  /// @brief Initialize Node
  protected: virtual void Init();
  /// @brief Get Child Nodes
  /// @return pointer to child node
  protected: virtual NodeStorePtr Children() const override;
  /// @brief  @brief Attach Child Node
  /// @param _child 
  /// @return true if successful false otherwise
  protected: virtual bool AttachChild(NodePtr _child) override;
  /// @brief Dettach Child Node
  /// @param _child 
  /// @return true if successful false otherwise
  protected: virtual bool DetachChild(NodePtr _child) override;
  
  protected: LuxCoreEngineNodePtr parent;
  
  protected: math::Pose3d pose;
  
  protected: bool poseDirty;
  
  protected: math::Vector3d scale = math::Vector3d::One;
  
  protected: bool inheritScale = true;
};

} // namespace IGNITION_RENDERING_VERSION_NAMESPACE
} // namespace rendering
} // namespace ignition

#endif
