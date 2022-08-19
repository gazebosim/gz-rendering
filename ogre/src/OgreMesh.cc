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

#include <gz/common/Console.hh>

#include "gz/rendering/ogre/OgreMesh.hh"
#include "gz/rendering/ogre/OgreIncludes.hh"
#include "gz/rendering/ogre/OgreMaterial.hh"
#include "gz/rendering/ogre/OgreStorage.hh"
#include "gz/rendering/ogre/OgreRTShaderSystem.hh"

using namespace gz;
using namespace rendering;

//////////////////////////////////////////////////
OgreMesh::OgreMesh()
{
}

//////////////////////////////////////////////////
OgreMesh::~OgreMesh()
{
  this->Destroy();
}

//////////////////////////////////////////////////
void OgreMesh::Destroy()
{
  if (!this->ogreEntity)
    return;

  if (!this->Scene()->IsInitialized())
    return;

  BaseMesh::Destroy();

  auto ogreScene = std::dynamic_pointer_cast<OgreScene>(this->Scene());

  ogreScene->OgreSceneManager()->destroyEntity(this->ogreEntity);
  this->ogreEntity = nullptr;
}

//////////////////////////////////////////////////
bool OgreMesh::HasSkeleton() const
{
  return this->ogreEntity->hasSkeleton();
}

//////////////////////////////////////////////////
std::map<std::string, math::Matrix4d>
        OgreMesh::SkeletonLocalTransforms() const
{
  std::map<std::string, math::Matrix4d> mapTfs;
  if (this->ogreEntity->hasSkeleton())
  {
    Ogre::SkeletonInstance *skel = this->ogreEntity->getSkeleton();
    for (unsigned int i = 0; i < skel->getNumBones(); ++i)
    {
      Ogre::Bone *bone = skel->getBone(i);
      Ogre::Quaternion quat(bone->getOrientation());
      Ogre::Vector3 p(bone->getPosition());

      math::Quaterniond tfQuat(quat.w, quat.x, quat.y, quat.z);
      math::Vector3d tfTrans(p.x, p.y, p.z);

      math::Matrix4d tf(tfQuat);
      tf.SetTranslation(tfTrans);

      mapTfs[bone->getName()] = tf;
    }
  }

  return mapTfs;
}

//////////////////////////////////////////////////
void OgreMesh::SetSkeletonLocalTransforms(
          const std::map<std::string, math::Matrix4d> &_tfs)
{
  if (!this->ogreEntity->hasSkeleton())
  {
    return;
  }
  Ogre::SkeletonInstance *skel = this->ogreEntity->getSkeleton();

  for (auto const& pair : _tfs)
  {
    if (skel->hasBone(pair.first))
    {
      Ogre::Bone *bone = skel->getBone(pair.first);
      math::Matrix4d tf = pair.second;
      math::Vector3d tf_trans = tf.Translation();
      math::Quaterniond tf_quat = tf.Rotation();

      Ogre::Vector3 p(tf_trans.X(), tf_trans.Y(), tf_trans.Z());

      const Ogre::Quaternion quat(
        tf_quat.W(), tf_quat.X(), tf_quat.Y(), tf_quat.Z());

      bone->setManuallyControlled(true);
      bone->setPosition(p);
      bone->setOrientation(quat);
    }
  }
}

//////////////////////////////////////////////////
Ogre::MovableObject *OgreMesh::OgreObject() const
{
  return this->ogreEntity;
}

//////////////////////////////////////////////////
SubMeshStorePtr OgreMesh::SubMeshes() const
{
  return this->subMeshes;
}

//////////////////////////////////////////////////
OgreSubMesh::OgreSubMesh()
{
}

//////////////////////////////////////////////////
OgreSubMesh::~OgreSubMesh()
{
  this->Destroy();
}

//////////////////////////////////////////////////
Ogre::SubEntity *OgreSubMesh::OgreSubEntity() const
{
  return this->ogreSubEntity;
}

//////////////////////////////////////////////////
void OgreSubMesh::Destroy()
{
  OgreRTShaderSystem::Instance()->DetachEntity(this);

  BaseSubMesh::Destroy();
}

//////////////////////////////////////////////////
void OgreSubMesh::SetMaterialImpl(MaterialPtr _material)
{
  OgreMaterialPtr derived =
      std::dynamic_pointer_cast<OgreMaterial>(_material);

  if (!derived)
  {
    ignerr << "Cannot assign material created by another render-engine"
        << std::endl;

    return;
  }

  std::string materialName = derived->Name();
  Ogre::MaterialPtr ogreMaterial = derived->Material();
  this->ogreSubEntity->setMaterialName(materialName);

  // set cast shadows
  this->ogreSubEntity->getParent()->setCastShadows(_material->CastShadows());
}

//////////////////////////////////////////////////
void OgreSubMesh::Init()
{
  BaseSubMesh::Init();
  OgreRTShaderSystem::Instance()->AttachEntity(this);
}

//////////////////////////////////////////////////
