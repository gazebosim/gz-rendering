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


#include <sstream>

#include <gz/common/Console.hh>
#include <gz/common/Material.hh>
#include <gz/common/Skeleton.hh>
#include <gz/common/SkeletonAnimation.hh>
#include <gz/common/SubMesh.hh>
#include <gz/common/Profiler.hh>

#include <gz/math/Matrix4.hh>

#include "gz/rendering/ogre_next/OgreNextConversions.hh"
#include "gz/rendering/ogre_next/OgreNextMesh.hh"
#include "gz/rendering/ogre_next/OgreNextMeshFactory.hh"
#include "gz/rendering/ogre_next/OgreNextRenderEngine.hh"
#include "gz/rendering/ogre_next/OgreNextRenderTypes.hh"
#include "gz/rendering/ogre_next/OgreNextScene.hh"
#include "gz/rendering/ogre_next/OgreNextStorage.hh"

#ifdef _MSC_VER
  #pragma warning(push, 0)
#endif
#include <OgreHardwareBufferManager.h>
#include <OgreItem.h>
#include <OgreKeyFrame.h>
#include <OgreMesh2.h>
#include <OgreMeshManager.h>
#include <OgreMeshManager2.h>
#include <OgreOldBone.h>
#include <OgreOldSkeletonManager.h>
#include <OgreSceneManager.h>
#include <OgreSkeleton.h>
#include <OgreSubItem.h>
#include <OgreSubMesh.h>
#include <OgreSubMesh2.h>
#ifdef _MSC_VER
  #pragma warning(pop)
#endif

/// \brief Private data for the OgreNextMeshFactory class
class gz::rendering::OgreNextMeshFactoryPrivate
{
};

/// \brief Private data for the OgreNextSubMeshStoreFactory class
class gz::rendering::OgreNextSubMeshStoreFactoryPrivate
{
};

using namespace gz;
using namespace rendering;

//////////////////////////////////////////////////
OgreNextMeshFactory::OgreNextMeshFactory(OgreNextScenePtr _scene) :
  scene(_scene), dataPtr(std::make_unique<OgreNextMeshFactoryPrivate>())
{
}

//////////////////////////////////////////////////
OgreNextMeshFactory::~OgreNextMeshFactory()
{
}

//////////////////////////////////////////////////
void OgreNextMeshFactory::Clear()
{
  for (auto &m : this->ogreMeshes)
    Ogre::MeshManager::getSingleton().remove(m);

  this->ogreMeshes.clear();
}

//////////////////////////////////////////////////
void OgreNextMeshFactory::ClearMaterialsCache(const std::string &)
{
  // no-op
}

//////////////////////////////////////////////////
OgreNextMeshPtr OgreNextMeshFactory::Create(const MeshDescriptor &_desc)
{
  GZ_PROFILE("Ogre2MeshFactory::Create");
  // create ogre entity
  OgreNextMeshPtr mesh(new OgreNextMesh);
  MeshDescriptor normDesc = _desc;
  normDesc.Load();
  mesh->ogreItem = this->OgreItem(normDesc);

  // check if invalid mesh
  if (!mesh->ogreItem)
  {
    gzerr << "Failed to get Ogre item for [" << _desc.meshName << "]"
           << std::endl;
    return nullptr;
  }

  // create sub-mesh store
  OgreNextSubMeshStoreFactory subMeshFactory(this->scene, mesh->ogreItem);
  mesh->subMeshes = subMeshFactory.Create();
  for (unsigned int i = 0; i < mesh->subMeshes->Size(); i++)
  {
    OgreNextSubMeshPtr submesh =
        std::dynamic_pointer_cast<OgreNextSubMesh>(mesh->subMeshes->GetById(i));
    submesh->SetMeshName(this->MeshName(_desc));
  }
  return mesh;
}

//////////////////////////////////////////////////
Ogre::Item *OgreNextMeshFactory::OgreItem(const MeshDescriptor &_desc)
{
  GZ_PROFILE("Ogre2MeshFactory::OgreItem");
  if (!this->Load(_desc))
  {
    return nullptr;
  }

  std::string name = this->MeshName(_desc);
  Ogre::SceneManager *sceneManager = this->scene->OgreSceneManager();

  // check if a v2 mesh already exists
  Ogre::MeshPtr mesh =
      Ogre::MeshManager::getSingleton().getByName(name);

  // if not, it probably has not been imported from v1 yet
  if (!mesh)
  {
    Ogre::v1::MeshPtr v1Mesh =
        Ogre::v1::MeshManager::getSingleton().getByName(name);
    if (!v1Mesh)
      return nullptr;

    // create v2 mesh from v1
    mesh = Ogre::MeshManager::getSingleton().createManual(
        name, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    mesh->importV1(v1Mesh.get(), false, true, true);
    this->ogreMeshes.push_back(name);
  }

  return sceneManager->createItem(mesh, Ogre::SCENE_DYNAMIC);
}

//////////////////////////////////////////////////
bool OgreNextMeshFactory::Load(const MeshDescriptor &_desc)
{
  if (!this->Validate(_desc))
  {
    return false;
  }

  if (this->IsLoaded(_desc))
  {
    return true;
  }

  return this->LoadImpl(_desc);
}

//////////////////////////////////////////////////
bool OgreNextMeshFactory::IsLoaded(const MeshDescriptor &_desc)
{
  std::string name = this->MeshName(_desc);
  return Ogre::MeshManager::getSingleton().resourceExists(name);
}

//////////////////////////////////////////////////
bool OgreNextMeshFactory::LoadImpl(const MeshDescriptor &_desc)
{
  GZ_PROFILE("Ogre2MeshFactory::LoadImpl");
  Ogre::v1::MeshPtr ogreMesh;

  OgreNextRenderEngine::Instance()->AddResourcePath(_desc.mesh->Path());

  try
  {
    const auto &name = this->MeshName(_desc);
    const auto &group = Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME;
    ogreMesh = Ogre::v1::MeshManager::getSingleton().createManual(name, group);

    // load skeleton
    Ogre::v1::SkeletonPtr ogreSkeleton;
    if (_desc.mesh->HasSkeleton())
    {
      common::SkeletonPtr skel = _desc.mesh->MeshSkeleton();
      ogreSkeleton = Ogre::v1::OldSkeletonManager::getSingleton().create(
        _desc.mesh->Name() + "_skeleton",
        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
        true);

      // load bones
      for (unsigned int i = 0; i < skel->NodeCount(); i++)
      {
        common::SkeletonNode *node = skel->NodeByHandle(i);
        Ogre::v1::OldBone *bone = ogreSkeleton->createBone(node->Name());

        if (node->Parent())
          ogreSkeleton->getBone(node->Parent()->Name())->addChild(bone);

        math::Matrix4d trans = node->Transform();
        math::Vector3d pos = trans.Translation();
        math::Quaterniond q = trans.Rotation();
        bone->setPosition(Ogre::Vector3(pos.X(), pos.Y(), pos.Z()));
        bone->setOrientation(Ogre::Quaternion(q.W(), q.X(), q.Y(), q.Z()));
        bone->setInheritOrientation(true);
        bone->setManuallyControlled(true);
        bone->setInitialState();
      }

      // load skeletal animations
      for (unsigned int i = 0; i < skel->AnimationCount(); ++i)
      {
        common::SkeletonAnimation *skelAnim = skel->Animation(i);
        if (ogreSkeleton->hasAnimation(skelAnim->Name()))
        {
          continue;
        }

        Ogre::v1::Animation *ogreAnim = ogreSkeleton->createAnimation(
            skelAnim->Name(), skelAnim->Length());

        for (unsigned int j = 0; j < skel->NodeCount(); ++j)
        {
          common::SkeletonNode *node = skel->NodeByHandle(j);
          common::NodeAnimation *nodeAnim = skelAnim->NodeAnimationByName(
              node->Name());
          if (!nodeAnim)
          {
            continue;
          }

          Ogre::v1::OldBone *bone = ogreSkeleton->getBone(node->Name());
          Ogre::v1::OldNodeAnimationTrack *ogreNodeAnimTrack =
              ogreAnim->createOldNodeTrack(j, bone);

          // set up transform (needed for bvh)
          math::Matrix4d alignTrans = skel->AlignTranslation(i, node->Name());
          math::Matrix4d alignRot = skel->AlignRotation(i, node->Name());

          for (unsigned int k = 0; k < nodeAnim->FrameCount(); ++k)
          {
            std::pair<double, math::Matrix4d> keyFrame = nodeAnim->KeyFrame(k);
            Ogre::v1::TransformKeyFrame *kf =
                ogreNodeAnimTrack->createNodeKeyFrame(keyFrame.first);

            math::Matrix4d p = keyFrame.second;

            // apply anim-skin transform
            p = alignTrans * p * alignRot;

            auto rot = bone->getOrientation().Inverse() *
                OgreNextConversions::Convert(p.Rotation());
            auto pos = OgreNextConversions::Convert(p.Translation()) -
                bone->getPosition();
            kf->setRotation(rot);
            kf->setTranslate(pos);
          }
        }
      }

      ogreMesh->setSkeletonName(_desc.mesh->Name() + "_skeleton");
    }

    for (unsigned int i = 0; i < _desc.mesh->SubMeshCount(); i++)
    {
      // if submesh is specified then load only that particular submesh
      auto s = _desc.mesh->SubMeshByIndex(i).lock();
      if (!_desc.subMeshName.empty() && s &&
          s->Name() != _desc.subMeshName)
      {
        continue;
      }

      // todo(iche033) use SubMesh::HasValidIndices() when gz-common 6.0.3
      // is released
      bool validIndices = true;
      for (unsigned int j = 0u; j < s->IndexCount(); ++j)
      {
        int index = s->Index(j);
        if (index > 0 && static_cast<unsigned int>(index) >= s->VertexCount())
        {
          validIndices = false;
          break;
        }
      }
      if (!validIndices)
      {
        gzwarn << "Mesh[" << _desc.mesh->Name() << "] submesh[" << s->Name()
               << "] has invalid indices. Skipping submesh creation."
               << std::endl;
        continue;
      }

      Ogre::v1::SubMesh *ogreSubMesh;
      Ogre::v1::VertexData *vertexData;
      Ogre::v1::VertexDeclaration* vertexDecl;
      Ogre::v1::HardwareVertexBufferSharedPtr vBuf;
      Ogre::v1::HardwareIndexBufferSharedPtr iBuf;
      float *vertices;
      uint32_t *indices;

      size_t currOffset = 0;

      // Copy the original submesh. We may need to modify the vertices, and
      // we don't want to change the original.
      common::SubMesh subMesh(*s.get());

      // Recenter the vertices if requested.
      if (_desc.centerSubMesh)
        subMesh.Center(math::Vector3d::Zero);

      ogreSubMesh = ogreMesh->createSubMesh(subMesh.Name());
      ogreSubMesh->useSharedVertices = false;
      if (subMesh.SubMeshPrimitiveType() == common::SubMesh::TRIANGLES)
      {
        ogreSubMesh->operationType =
            Ogre::OT_TRIANGLE_LIST;
      }
      else if (subMesh.SubMeshPrimitiveType() == common::SubMesh::LINES)
      {
        ogreSubMesh->operationType = Ogre::OT_LINE_LIST;
      }
      else if (subMesh.SubMeshPrimitiveType() == common::SubMesh::LINESTRIPS)
      {
        ogreSubMesh->operationType = Ogre::OT_LINE_STRIP;
      }
      else if (subMesh.SubMeshPrimitiveType() == common::SubMesh::TRIFANS)
      {
        ogreSubMesh->operationType = Ogre::OT_TRIANGLE_FAN;
      }
      else if (subMesh.SubMeshPrimitiveType() == common::SubMesh::TRISTRIPS)
      {
        ogreSubMesh->operationType =
            Ogre::OT_TRIANGLE_STRIP;
      }
      else if (subMesh.SubMeshPrimitiveType() == common::SubMesh::POINTS)
      {
        ogreSubMesh->operationType = Ogre::OT_POINT_LIST;
      }
      else
      {
        gzerr << "Unknown primitive type["
              << subMesh.SubMeshPrimitiveType() << "]\n";
      }

      ogreSubMesh->vertexData[Ogre::VpNormal] =
        new Ogre::v1::VertexData(ogreMesh->getHardwareBufferManager());
      vertexData = ogreSubMesh->vertexData[Ogre::VpNormal];
      vertexDecl = vertexData->vertexDeclaration;

      // The vertexDecl should contain positions, blending weights, normals,
      // diffiuse colors, specular colors, tex coords. In that order.
      vertexDecl->addElement(0, currOffset, Ogre::VET_FLOAT3,
                             Ogre::VES_POSITION);
      currOffset += Ogre::v1::VertexElement::getTypeSize(Ogre::VET_FLOAT3);

      // TODO(anyone): blending weights

      // normals
      if (subMesh.NormalCount() > 0)
      {
        vertexDecl->addElement(0, currOffset, Ogre::VET_FLOAT3,
                               Ogre::VES_NORMAL);
        currOffset += Ogre::v1::VertexElement::getTypeSize(Ogre::VET_FLOAT3);
      }

      // TODO(anyone): diffuse colors

      // TODO(anyone): specular colors

      // two dimensional texture coordinates
      // If submesh does not have texcoord sets, add one default set.
      // This is needed otherwise ogre_next will fail to generate tangents
      // during Ogre::v2::Mesh::importV1() and throw an ogre exception if we try
      // to apply normal maps to a submesh. Resulting object would then appear
      // white without any PBR textures.
      if (subMesh.TexCoordSetCount() == 0u)
      {
        vertexDecl->addElement(0, currOffset, Ogre::VET_FLOAT2,
            Ogre::VES_TEXTURE_COORDINATES, 0);
        currOffset += Ogre::v1::VertexElement::getTypeSize(Ogre::VET_FLOAT2);
      }
      // Add all texture coordinate sets
      else
      {
        for (unsigned int k = 0u; k < subMesh.TexCoordSetCount(); ++k)
        {
          if (subMesh.TexCoordCountBySet(k) > 0)
          {
            vertexDecl->addElement(0, currOffset, Ogre::VET_FLOAT2,
                Ogre::VES_TEXTURE_COORDINATES, k);
            currOffset +=
                Ogre::v1::VertexElement::getTypeSize(Ogre::VET_FLOAT2);
          }
        }
      }

      // allocate the vertex buffer
      vertexData->vertexCount = subMesh.VertexCount();

      vBuf = Ogre::v1::HardwareBufferManager::getSingleton().createVertexBuffer(
                 vertexDecl->getVertexSize(0),
                 vertexData->vertexCount,
                 Ogre::v1::HardwareBuffer::HBU_STATIC,
                 true);

      vertexData->vertexBufferBinding->setBinding(0, vBuf);
      vertices = static_cast<float*>(vBuf->lock(
                      Ogre::v1::HardwareBuffer::HBL_DISCARD));

      if (_desc.mesh->HasSkeleton())
      {
        common::SkeletonPtr skel = _desc.mesh->MeshSkeleton();
        for (unsigned int j = 0; j < subMesh.NodeAssignmentsCount(); j++)
        {
          common::NodeAssignment na = subMesh.NodeAssignmentByIndex(j);
          Ogre::v1::VertexBoneAssignment vba;
          vba.vertexIndex = na.vertexIndex;
          vba.boneIndex = ogreSkeleton->getBone(skel->NodeByHandle(
                              na.nodeIndex)->Name())->getHandle();
          vba.weight = na.weight;
          ogreSubMesh->addBoneAssignment(vba);
        }
      }

      // Add all the vertices
      for (unsigned int j = 0; j < subMesh.VertexCount(); ++j)
      {
        *vertices++ = subMesh.Vertex(j).X();
        *vertices++ = subMesh.Vertex(j).Y();
        *vertices++ = subMesh.Vertex(j).Z();

        // Add all normals
        if (subMesh.NormalCount() > 0)
        {
          *vertices++ = subMesh.Normal(j).X();
          *vertices++ = subMesh.Normal(j).Y();
          *vertices++ = subMesh.Normal(j).Z();
        }

        // Add all texture coordinate sets
        if (subMesh.TexCoordSetCount() == 0u)
        {
          *vertices++ = 0;
          *vertices++ = 0;
        }
        else
        {
          for (unsigned int k = 0u; k < subMesh.TexCoordSetCount(); ++k)
          {
            if (subMesh.TexCoordCountBySet(k) > 0u)
            {
              *vertices++ = subMesh.TexCoordBySet(j, k).X();
              *vertices++ = subMesh.TexCoordBySet(j, k).Y();
            }
          }
        }
      }

      vBuf->unlock();

      // Add all the indices
      // allocate index buffer
      ogreSubMesh->indexData[Ogre::VpNormal]->indexCount = subMesh.IndexCount();

      ogreSubMesh->indexData[Ogre::VpNormal]->indexBuffer =
        Ogre::v1::HardwareBufferManager::getSingleton().createIndexBuffer(
            Ogre::v1::HardwareIndexBuffer::IT_32BIT,
            ogreSubMesh->indexData[Ogre::VpNormal]->indexCount,
            Ogre::v1::HardwareBuffer::HBU_STATIC,
            true);

      iBuf = ogreSubMesh->indexData[Ogre::VpNormal]->indexBuffer;
      indices = static_cast<uint32_t*>(
          iBuf->lock(Ogre::v1::HardwareBuffer::HBL_DISCARD));


      for (unsigned int j = 0; j < subMesh.IndexCount(); ++j)
        *indices++ = static_cast<uint32_t>(subMesh.Index(j));

      iBuf->unlock();

      common::MaterialPtr material;
      if (const auto subMeshIdx = subMesh.GetMaterialIndex())
      {
        material = _desc.mesh->MaterialByIndex(subMeshIdx.value());
      }

      MaterialPtr mat = this->scene->CreateMaterial();
      if (material)
      {
        mat->CopyFrom(*material);
      }
      else
      {
        MaterialPtr defaultMat = this->scene->Material("Default/White");
        if (defaultMat != nullptr)
          mat->CopyFrom(defaultMat);
      }
      ogreSubMesh->setMaterialName(mat->Name());
    }

    math::Vector3d max = _desc.mesh->Max();
    math::Vector3d min = _desc.mesh->Min();

    if (_desc.mesh->HasSkeleton() &&
        _desc.mesh->MeshSkeleton()->AnimationCount() != 0)
    {
      min = math::Vector3d(-1, -1, -1);
      max = math::Vector3d(1, 1, 1);
    }

    if (!max.IsFinite())
    {
      gzerr << "Max bounding box is not finite[" << max << "]" << std::endl;
      return false;
    }

    if (!min.IsFinite())
    {
      gzerr << "Min bounding box is not finite[" << min << "]" << std::endl;
      return false;
    }

    if (!ogreMesh->hasValidShadowMappingBuffers())
      ogreMesh->prepareForShadowMapping(false);

    ogreMesh->_setBounds(Ogre::AxisAlignedBox(
          Ogre::Vector3(min.X(), min.Y(), min.Z()),
          Ogre::Vector3(max.X(), max.Y(), max.Z())),
          false);
    ogreMesh->_setBoundingSphereRadius((max - min).Length());

    // this line makes clear the mesh is loaded (avoids memory leaks)
    // ogreMesh->load();
  }
  catch(Ogre::Exception &e)
  {
    gzerr << "Unable to insert mesh[" << e.getDescription() << "]"
        << std::endl;
    return false;
  }

  if (ogreMesh->getNumSubMeshes() == 0u)
  {
    std::stringstream ss;
    ss << "Unable to load mesh: '" << _desc.meshName << "'";
    if (!_desc.subMeshName.empty())
      ss << ", submesh: '" << _desc.subMeshName << "'";
    ss << ". Mesh will be empty." << std::endl;
    gzwarn << ss.str();
  }

  return true;
}

//////////////////////////////////////////////////
std::string OgreNextMeshFactory::MeshName(const MeshDescriptor &_desc)
{
  std::stringstream ss;
  ss << _desc.meshName << "::";
  ss << _desc.subMeshName << "::";
  ss << ((_desc.centerSubMesh) ? "CENTERED" : "ORIGINAL");
  return ss.str();
}

//////////////////////////////////////////////////
bool OgreNextMeshFactory::Validate(const MeshDescriptor &_desc)
{
  if (!_desc.mesh && _desc.meshName.empty())
  {
    gzerr << "Invalid mesh-descriptor, no mesh specified" << std::endl;
    return false;
  }

  if (!_desc.mesh)
  {
    gzerr << "Cannot load null mesh [" << _desc.meshName << "]" << std::endl;
    return false;
  }

  if (_desc.mesh->SubMeshCount() == 0)
  {
    gzerr << "Cannot load mesh with zero sub-meshes" << std::endl;
    return false;
  }

  return true;
}

//////////////////////////////////////////////////
OgreNextSubMeshStoreFactory::OgreNextSubMeshStoreFactory(
    OgreNextScenePtr _scene,
    Ogre::Item *_item) :
  scene(_scene),
  ogreItem(_item),
  dataPtr(new OgreNextSubMeshStoreFactoryPrivate)
{
  this->CreateNameList();
}

//////////////////////////////////////////////////
OgreNextSubMeshStoreFactory::~OgreNextSubMeshStoreFactory()
{
}

//////////////////////////////////////////////////
OgreNextSubMeshStorePtr OgreNextSubMeshStoreFactory::Create()
{
  OgreNextSubMeshStorePtr subMeshes(new OgreNextSubMeshStore);
  unsigned int count = this->ogreItem->getNumSubItems();

  for (unsigned int i = 0; i < count; ++i)
  {
    OgreNextSubMeshPtr subMesh = this->CreateSubMesh(i);
    subMeshes->Add(subMesh);
  }

  return subMeshes;
}

//////////////////////////////////////////////////
OgreNextSubMeshPtr OgreNextSubMeshStoreFactory::CreateSubMesh(
    unsigned int _index)
{
  OgreNextSubMeshPtr subMesh(new OgreNextSubMesh);

  subMesh->id = _index;
  subMesh->name = this->names[_index];
  subMesh->scene = this->scene;
  subMesh->ogreSubItem = this->ogreItem->getSubItem(_index);

  MaterialPtr mat;
  Ogre::HlmsDatablock *ogreDatablock = subMesh->ogreSubItem->getDatablock();
  if (ogreDatablock)
  {
    std::string matName = subMesh->ogreSubItem->getSubMesh()->getMaterialName();
    mat = this->scene->Material(matName);
  }

  if (mat)
  {
    // assign material to submesh who will make a copy of this material
    subMesh->SetMaterial(mat);
  }

  subMesh->Load();
  subMesh->Init();

  return subMesh;
}

//////////////////////////////////////////////////
void OgreNextSubMeshStoreFactory::CreateNameList()
{
  this->PopulateDefaultNames();
  // this->PopulateGivenNames();
}

//////////////////////////////////////////////////
void OgreNextSubMeshStoreFactory::PopulateDefaultNames()
{
  unsigned int count = this->ogreItem->getNumSubItems();
  this->names.reserve(count);

  for (unsigned int i = 0; i < count; ++i)
  {
    this->names.push_back("SubMesh(" + std::to_string(i) + ")");
  }
}

//////////////////////////////////////////////////
void OgreNextSubMeshStoreFactory::PopulateGivenNames()
{
  const Ogre::MeshPtr ogreMesh = this->ogreItem->getMesh();

  for (const auto &pair : ogreMesh->getSubMeshNameMap())
  {
    unsigned int index = pair.second;
    this->names[index] = pair.first;
  }
}
