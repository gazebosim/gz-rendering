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

#include <cmath>

#include <filament/Engine.h>
#include <filament/TransformManager.h>
#include <utils/EntityManager.h>

#include <gz/common/Console.hh>

#include "gz/rendering/filament/FilamentConversions.hh"
#include "gz/rendering/filament/FilamentNode.hh"
#include "gz/rendering/filament/FilamentRenderEngine.hh"
#include "gz/rendering/filament/FilamentScene.hh"

using namespace gz;
using namespace rendering;

//////////////////////////////////////////////////
bool FilamentNode::HasParent() const
{
  return this->parent != nullptr;
}

//////////////////////////////////////////////////
NodePtr FilamentNode::Parent() const
{
  return this->parent;
}

//////////////////////////////////////////////////
::utils::Entity FilamentNode::Entity() const
{
  return this->entity;
}

//////////////////////////////////////////////////
void FilamentNode::Destroy()
{
  if (this->entity.isNull())
    return;
  BaseNode::Destroy();
  auto *engine = FilamentRenderEngine::Instance()->FilamentEngine();
  if (engine)
  {
    engine->getTransformManager().destroy(this->entity);
    ::utils::EntityManager::get().destroy(this->entity);
  }
  this->entity = ::utils::Entity();
  this->parent.reset();
}

//////////////////////////////////////////////////
math::Vector3d FilamentNode::LocalScale() const
{
  return this->scale;
}

//////////////////////////////////////////////////
bool FilamentNode::InheritScale() const
{
  return this->inheritScale;
}

//////////////////////////////////////////////////
void FilamentNode::SetInheritScale(bool _inherit)
{
  this->inheritScale = _inherit;
  this->UpdateTransform();
}

//////////////////////////////////////////////////
void FilamentNode::SetLocalScaleImpl(const math::Vector3d &_scale)
{
  this->scale = _scale;
  this->UpdateTransform();
  this->UpdateScaleDependents();
}

//////////////////////////////////////////////////
math::Pose3d FilamentNode::RawLocalPose() const
{
  return this->pose;
}

//////////////////////////////////////////////////
void FilamentNode::SetRawLocalPose(const math::Pose3d &_pose)
{
  this->pose = _pose;
  this->UpdateTransform();
}

//////////////////////////////////////////////////
void FilamentNode::UpdateTransform()
{
  if (this->entity.isNull())
    return;
  auto &tm = FilamentRenderEngine::Instance()->FilamentEngine()
      ->getTransformManager();
  filament::math::mat4f m = FilamentConversions::Convert(this->pose,
      this->scale);
  if (!this->inheritScale && this->parent)
  {
    // filament composes the parent's world transform: undo its scale on
    // the rotation and scale part, like ogre's inherit scale off (the
    // position stays in the parent's scaled frame).
    // TODO(anyone): exact for parents whose world transform has no shear only
    const math::Vector3d parentScale = this->parent->WorldScale();
    for (int r = 0; r < 3; ++r)
    {
      if (std::abs(parentScale[r]) < 1e-9)
        continue;
      const float inv = static_cast<float>(1.0 / parentScale[r]);
      for (int c = 0; c < 3; ++c)
        m[c][r] *= inv;
    }
  }
  tm.setTransform(tm.getInstance(this->entity), m);
}

//////////////////////////////////////////////////
void FilamentNode::UpdateScaleDependents()
{
  if (!this->children)
    return;
  for (auto it = this->children->Begin(); it != this->children->End(); ++it)
  {
    auto child = std::dynamic_pointer_cast<FilamentNode>(*it);
    if (!child)
      continue;
    // a child that does not inherit scale cancels this node's: its own
    // children see only its local scale
    if (child->inheritScale)
      child->UpdateScaleDependents();
    else
      child->UpdateTransform();
  }
}

//////////////////////////////////////////////////
NodeStorePtr FilamentNode::Children() const
{
  return this->children;
}

//////////////////////////////////////////////////
bool FilamentNode::AttachChild(NodePtr _child)
{
  FilamentNodePtr derived = std::dynamic_pointer_cast<FilamentNode>(_child);
  if (!derived)
  {
    gzerr << "Cannot attach node created by another render-engine"
          << std::endl;
    return false;
  }
  // cycle check
  for (FilamentNodePtr p = this->parent; p; p = p->parent)
  {
    if (p.get() == derived.get())
    {
      gzerr << "Node cycle detected. Not adding Node: " << _child->Name()
            << std::endl;
      return false;
    }
  }
  derived->SetParent(this->SharedThis());
  auto &tm = FilamentRenderEngine::Instance()->FilamentEngine()
      ->getTransformManager();
  tm.setParent(tm.getInstance(derived->entity), tm.getInstance(this->entity));
  derived->UpdateTransform();
  derived->UpdateScaleDependents();
  derived->SetInScene(this->InSceneGraph());
  return true;
}

//////////////////////////////////////////////////
bool FilamentNode::DetachChild(NodePtr _child)
{
  FilamentNodePtr derived = std::dynamic_pointer_cast<FilamentNode>(_child);
  if (!derived)
  {
    gzerr << "Cannot detach node created by another render-engine"
          << std::endl;
    return false;
  }
  auto &tm = FilamentRenderEngine::Instance()->FilamentEngine()
      ->getTransformManager();
  if (!derived->entity.isNull())
    tm.setParent(tm.getInstance(derived->entity), {});
  derived->SetParent(nullptr);
  derived->UpdateTransform();
  derived->UpdateScaleDependents();
  derived->SetInScene(false);
  return true;
}

//////////////////////////////////////////////////
bool FilamentNode::InSceneGraph() const
{
  if (!this->scene)
    return false;
  const Node *root = this->scene->RootVisual().get();
  for (const Node *n = this; n; n = n->Parent().get())
  {
    if (n == root)
      return true;
  }
  return false;
}

//////////////////////////////////////////////////
void FilamentNode::SetInScene(bool _inScene)
{
  for (auto it = this->children->Begin(); it != this->children->End(); ++it)
  {
    auto child = std::dynamic_pointer_cast<FilamentNode>(*it);
    if (child)
      child->SetInScene(_inScene);
  }
}

//////////////////////////////////////////////////
void FilamentNode::SetParent(FilamentNodePtr _parent)
{
  this->parent = _parent;
}

//////////////////////////////////////////////////
void FilamentNode::Init()
{
  auto *engine = FilamentRenderEngine::Instance()->FilamentEngine();
  if (!engine)
  {
    gzerr << "Failed to initialize node: filament engine is NULL"
          << std::endl;
    return;
  }
  this->entity = ::utils::EntityManager::get().create();
  engine->getTransformManager().create(this->entity);
  this->children = FilamentNodeStorePtr(new FilamentNodeStore);
  this->UpdateTransform();
}

//////////////////////////////////////////////////
FilamentNodePtr FilamentNode::SharedThis()
{
  return std::dynamic_pointer_cast<FilamentNode>(this->shared_from_this());
}
