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

// Skeletal animation of FilamentMesh, with ogre2's semantics (ogre-next
// SkeletonInstance fed by Ogre2MeshFactory):
//  - bones start at their bind pose, the local SkeletonNode transforms;
//  - an animation keyframe is stored relative to the bind pose, like
//    Ogre2MeshFactory's OldNodeAnimationTrack (pos - bindPos,
//    bindRot^-1 * rot), and each enabled animation adds
//    pos += d * w, rot = rot * nlerp(w, identity, dRot), with w the
//    animation weight times the per-bone weight (SkeletonTrack);
//  - SetSkeletonLocalTransforms makes bones manual: they keep the given
//    local transform and are not reset to the bind pose; enabling an
//    animation makes every bone automatic again;
//  - the skinning matrix of a bone is its current model-space transform
//    times the inverse of its bind model-space transform.

#include <algorithm>
#include <cmath>
#include <vector>

#include <filament/Engine.h>
#include <filament/RenderableManager.h>
#include <math/mat4.h>

#include <gz/common/Console.hh>
#include <gz/common/NodeAnimation.hh>
#include <gz/common/SkeletonAnimation.hh>
#include <gz/common/SkeletonNode.hh>

#include "gz/rendering/filament/FilamentConversions.hh"
#include "gz/rendering/filament/FilamentMesh.hh"
#include "gz/rendering/filament/FilamentRenderEngine.hh"

using namespace gz;
using namespace rendering;

struct gz::rendering::FilamentMesh::SkeletonState
{
  common::SkeletonPtr skel;

  struct Bone
  {
    std::string name;
    /// \brief Parent bone index, -1 for a root
    int parent{-1};
    math::Vector3d bindPos;
    math::Quaterniond bindRot;
    math::Vector3d pos;
    math::Quaterniond rot;
    bool manual{false};
    math::Matrix4d invBind;
  };
  std::vector<Bone> bones;

  /// \brief Index of the first bone of each name
  std::unordered_map<std::string, int> boneIndex;

  struct Anim
  {
    std::string name;
    double length{0.0};
    bool enabled{false};
    bool loop{true};
    float weight{1.0f};
    double time{0.0};
    std::vector<float> boneWeights;
    /// \brief Per bone: keyframe track (null if not animated) and the
    /// anim-to-skin alignment Ogre2MeshFactory bakes into its keyframes
    std::vector<common::NodeAnimation *> tracks;
    std::vector<math::Matrix4d> alignTrans;
    std::vector<math::Matrix4d> alignRot;
  };
  std::vector<Anim> anims;

  /// \brief Pose changed since the last upload
  bool dirty{true};

  /// \brief Whether the renderable has GPU skinning (<= 255 bones)
  bool gpu{false};

  int Find(const std::string &_name) const
  {
    auto it = this->boneIndex.find(_name);
    return it == this->boneIndex.end() ? -1 : it->second;
  }

  /// \brief ogre's Quaternion::nlerpShortest(_w, IDENTITY, _q)
  static math::Quaterniond Nlerp(double _w, const math::Quaterniond &_q)
  {
    const double s = _q.W() < 0.0 ? -_w : _w;
    math::Quaterniond r(1.0 - _w + s * _q.W(), s * _q.X(), s * _q.Y(),
        s * _q.Z());
    r.Normalize();
    return r;
  }

  /// \brief Reset automatic bones to the bind pose, then add the enabled
  /// animations (ogre-next SkeletonInstance::update)
  void Pose()
  {
    for (auto &b : this->bones)
    {
      if (!b.manual)
      {
        b.pos = b.bindPos;
        b.rot = b.bindRot;
      }
    }
    for (const auto &a : this->anims)
    {
      if (!a.enabled)
        continue;
      for (size_t j = 0; j < this->bones.size(); ++j)
      {
        Bone &b = this->bones[j];
        // Ogre-next also adds animations on top of manual bones,
        // which accumulates every frame; skipped here
        if (!a.tracks[j] || b.manual)
          continue;
        const math::Matrix4d p = a.alignTrans[j] *
            a.tracks[j]->FrameAt(a.time, false) * a.alignRot[j];
        const double w = a.weight * a.boneWeights[j];
        b.pos += (p.Translation() - b.bindPos) * w;
        b.rot = b.rot * Nlerp(w, b.bindRot.Inverse() * p.Rotation());
      }
    }
  }

  /// \brief Model-space transform of every bone from local (pos, rot)
  std::vector<math::Matrix4d> World(bool _bind) const
  {
    std::vector<math::Matrix4d> world(this->bones.size());
    for (size_t i = 0; i < this->bones.size(); ++i)
    {
      const Bone &b = this->bones[i];
      math::Matrix4d local(_bind ? b.bindRot : b.rot);
      local.SetTranslation(_bind ? b.bindPos : b.pos);
      // parents come first: InitSkeleton orders by handle, like ogre2,
      // which creates bones in handle order and attaches to the parent
      world[i] = b.parent >= 0 ? world[b.parent] * local : local;
    }
    return world;
  }
};

//////////////////////////////////////////////////
void FilamentMesh::InitSkeleton(common::SkeletonPtr _skeleton, bool _gpu)
{
  if (!_skeleton || _skeleton->NodeCount() == 0u)
    return;
  auto state = std::make_shared<SkeletonState>();
  state->skel = _skeleton;
  state->gpu = _gpu;
  for (unsigned int i = 0; i < _skeleton->NodeCount(); ++i)
  {
    common::SkeletonNode *node = _skeleton->NodeByHandle(i);
    SkeletonState::Bone b;
    b.name = node->Name();
    if (node->Parent())
      b.parent = state->Find(node->Parent()->Name());
    const math::Matrix4d tf = node->Transform();
    b.bindPos = b.pos = tf.Translation();
    b.bindRot = b.rot = tf.Rotation();
    state->boneIndex.emplace(b.name, static_cast<int>(state->bones.size()));
    state->bones.push_back(b);
  }
  const std::vector<math::Matrix4d> bind = state->World(true);
  for (size_t i = 0; i < bind.size(); ++i)
    state->bones[i].invBind = bind[i].Inverse();

  // animations present when the mesh is created, like ogre2
  for (unsigned int i = 0; i < _skeleton->AnimationCount(); ++i)
  {
    common::SkeletonAnimation *src = _skeleton->Animation(i);
    SkeletonState::Anim a;
    a.name = src->Name();
    a.length = src->Length();
    a.boneWeights.assign(state->bones.size(), 1.0f);
    for (const auto &b : state->bones)
    {
      common::NodeAnimation *track = src->NodeAnimationByName(b.name);
      a.tracks.push_back(track && track->FrameCount() > 0 ? track : nullptr);
      a.alignTrans.push_back(_skeleton->AlignTranslation(i, b.name));
      a.alignRot.push_back(_skeleton->AlignRotation(i, b.name));
    }
    state->anims.push_back(a);
  }
  this->skeleton = state;
}

//////////////////////////////////////////////////
void FilamentMesh::UpdateBones()
{
  if (!this->skeleton || !this->skeleton->dirty)
    return;
  SkeletonState &s = *this->skeleton;
  s.Pose();
  s.dirty = false;
  if (!s.gpu)
    return;
  auto &rm = FilamentRenderEngine::Instance()->FilamentEngine()
      ->getRenderableManager();
  if (!rm.hasComponent(this->entity))
    return;
  const std::vector<math::Matrix4d> world = s.World(false);
  std::vector<filament::math::mat4f> m(world.size());
  for (size_t i = 0; i < world.size(); ++i)
  {
    m[i] = filament::math::mat4f(
        FilamentConversions::Convert(world[i] * s.bones[i].invBind));
  }
  rm.setBones(rm.getInstance(this->entity), m.data(), m.size(), 0);
}

//////////////////////////////////////////////////
bool FilamentMesh::HasSkeleton() const
{
  return this->skeleton != nullptr;
}

//////////////////////////////////////////////////
std::map<std::string, math::Matrix4d>
    FilamentMesh::SkeletonLocalTransforms() const
{
  std::map<std::string, math::Matrix4d> tfs;
  if (!this->skeleton)
    return tfs;
  // ogre2 returns the pose of its last scene update; this is the pose the
  // next frame renders
  if (this->skeleton->dirty)
    this->skeleton->Pose();
  for (const auto &b : this->skeleton->bones)
  {
    math::Matrix4d tf(b.rot);
    tf.SetTranslation(b.pos);
    tfs[b.name] = tf;
  }
  return tfs;
}

//////////////////////////////////////////////////
void FilamentMesh::SetSkeletonLocalTransforms(
    const std::map<std::string, math::Matrix4d> &_tfs)
{
  if (!this->skeleton)
    return;
  for (const auto &[boneName, tf] : _tfs)
  {
    const int i = this->skeleton->Find(boneName);
    if (i < 0)
      continue;
    auto &b = this->skeleton->bones[i];
    b.manual = true;
    b.pos = tf.Translation();
    b.rot = tf.Rotation();
  }
  this->skeleton->dirty = true;
}

//////////////////////////////////////////////////
std::unordered_map<std::string, float> FilamentMesh::SkeletonWeights() const
{
  std::unordered_map<std::string, float> weights;
  if (!this->skeleton || this->skeleton->anims.empty())
    return weights;
  // like ogre2: all animations are assumed to share the bone weights
  const auto &a = this->skeleton->anims.front();
  for (size_t i = 0; i < this->skeleton->bones.size(); ++i)
    weights[this->skeleton->bones[i].name] = a.boneWeights[i];
  return weights;
}

//////////////////////////////////////////////////
void FilamentMesh::SetSkeletonWeights(
    const std::unordered_map<std::string, float> &_weights)
{
  if (!this->skeleton)
    return;
  for (auto &a : this->skeleton->anims)
  {
    for (const auto &[boneName, weight] : _weights)
    {
      const int i = this->skeleton->Find(boneName);
      if (i >= 0)
        a.boneWeights[i] = weight;
    }
  }
  this->skeleton->dirty = true;
}

//////////////////////////////////////////////////
void FilamentMesh::SetSkeletonAnimationEnabled(const std::string &_name,
    bool _enabled, bool _loop, float _weight)
{
  if (!this->skeleton)
    return;
  for (auto &a : this->skeleton->anims)
  {
    if (a.name != _name)
      continue;
    if (_enabled)
    {
      for (auto &b : this->skeleton->bones)
        b.manual = false;
    }
    a.enabled = _enabled;
    a.loop = _loop;
    a.weight = _weight;
    this->skeleton->dirty = true;
    return;
  }
  gzerr << "Skeleton animation name not found: " << _name << std::endl;
}

//////////////////////////////////////////////////
bool FilamentMesh::SkeletonAnimationEnabled(const std::string &_name) const
{
  if (!this->skeleton)
    return false;
  for (const auto &a : this->skeleton->anims)
  {
    if (a.name == _name)
      return a.enabled;
  }
  gzerr << "Skeleton animation name not found: " << _name << std::endl;
  return false;
}

//////////////////////////////////////////////////
void FilamentMesh::UpdateSkeletonAnimation(
    std::chrono::steady_clock::duration _time)
{
  if (!this->skeleton)
    return;
  // ogre2: millisecond resolution, then SkeletonAnimation::setTime
  const double seconds =
      std::chrono::duration_cast<std::chrono::milliseconds>(_time).count() /
      1000.0;
  for (auto &a : this->skeleton->anims)
  {
    if (!a.enabled)
      continue;
    if (a.loop)
    {
      a.time = a.length > 0.0 ? std::fmod(seconds, a.length) : 0.0;
      if (a.time < 0.0)
        a.time += a.length;
    }
    else
    {
      a.time = std::clamp(seconds, 0.0, a.length);
    }
    this->skeleton->dirty = true;
  }
}
