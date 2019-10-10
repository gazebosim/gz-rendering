/*
 * Copyright (C) 2019 Open Source Robotics Foundation
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

#include <ignition/common/Console.hh>

#include "ignition/rendering/ogre/OgreDynamicLines.hh"
#include "ignition/rendering/ogre/OgreMarker.hh"
#include "ignition/rendering/ogre/OgreMaterial.hh"
#include "ignition/rendering/ogre/OgreScene.hh"

class ignition::rendering::OgreMarkerPrivate
{
  /// \brief Grid materal
  public: OgreMaterialPtr material;

  public: std::shared_ptr<OgreDynamicLines> dynamicRenderable;

  public: Ogre::MovableObject *movableObject = nullptr;

  public: Type type;

  public: Visibility visibility;

  public: Action action;
};

using namespace ignition;
using namespace rendering;

//////////////////////////////////////////////////
OgreMarker::OgreMarker()
  : dataPtr(new OgreMarkerPrivate)
{
}
// TODO(jshep1): add Destroy implementation
//////////////////////////////////////////////////
OgreMarker::~OgreMarker()
{
}

//////////////////////////////////////////////////
void OgreMarker::PreRender()
{
  this->dataPtr->dynamicRenderable->Update();
}

// TODO(jshep1): Add actual implementation for ogreobject
//////////////////////////////////////////////////
Ogre::MovableObject *OgreMarker::OgreObject() const
{
  return this->dataPtr->movableObject;
}

//////////////////////////////////////////////////
void OgreMarker::Init()
{
  this->Create();
}

//////////////////////////////////////////////////
void OgreMarker::Create()
{
  // TODO(jshep1): init dynamicRenderable to default rendertype
  this->dataPtr->type = LINE_STRIP;
  this->dataPtr->visibility = GUI;
  this->dataPtr->action = ADD_MODIFY;
  this->dataPtr->dynamicRenderable.reset(new OgreDynamicLines(LINE_STRIP));

  if (!this->dataPtr->movableObject)
  {
    // this->dataPtr->movableObject = this->scene->CreateBox();
  }

  //this->dataPtr->movableObject->clear();


  //this->dataPtr->movableObject->end();

}
/*
//////////////////////////////////////////////////
void OgreMarker::InsertMesh(const common::Mesh *_mesh, const std::string &_subMesh,
                            const bool _centerSubmesh)
{
  Ogre::MeshPtr ogreMesh;

  IGN_ASSERT(_mesh != nullptr, "Unable to insert a null mesh");

  RenderEngine::Instance()->AddResourcePath(_mesh->GetPath());

  if (_mesh->GetSubMeshCount() == 0)
  {
    ignerr << "Visual::InsertMesh no submeshes, this is an invalid mesh\n";
    return;
  }

  // Don't re-add existing meshes
  if (Ogre::MeshManager::getSingleton().resourceExists(_mesh->GetName()))
  {
    return;
  }

  try
  {
    // Create a new mesh specifically for manual definition.
    if (_subMesh.empty())
    {
      ogreMesh = Ogre::MeshManager::getSingleton().createManual(
          _mesh->GetName(),
          Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    }
    else
    {
      ogreMesh = Ogre::MeshManager::getSingleton().createManual(
          _mesh->GetName() + "::" + _subMesh,
          Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    }

    Ogre::SkeletonPtr ogreSkeleton;

    if (_mesh->HasSkeleton())
    {
      common::Skeleton *skel = _mesh->GetSkeleton();
      ogreSkeleton = Ogre::SkeletonManager::getSingleton().create(
        _mesh->GetName() + "_skeleton",
        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
        true);

      for (unsigned int i = 0; i < skel->GetNumNodes(); i++)
      {
        common::SkeletonNode *node = skel->GetNodeByHandle(i);
        Ogre::Bone *bone = ogreSkeleton->createBone(node->GetName());

        if (node->GetParent())
          ogreSkeleton->getBone(node->GetParent()->GetName())->addChild(bone);

        ignition::math::Matrix4d trans = node->Transform();
        ignition::math::Vector3d pos = trans.Translation();
        ignition::math::Quaterniond q = trans.Rotation();
        bone->setPosition(Ogre::Vector3(pos.X(), pos.Y(), pos.Z()));
        bone->setOrientation(Ogre::Quaternion(q.W(), q.X(), q.Y(), q.Z()));
        bone->setInheritOrientation(true);
        bone->setManuallyControlled(true);
        bone->setInitialState();
      }
      ogreMesh->setSkeletonName(_mesh->GetName() + "_skeleton");
    }

    for (unsigned int i = 0; i < _mesh->GetSubMeshCount(); i++)
    {
      if (!_subMesh.empty() && _mesh->GetSubMesh(i)->GetName() != _subMesh)
        continue;

      Ogre::SubMesh *ogreSubMesh;
      Ogre::VertexData *vertexData;
      Ogre::VertexDeclaration* vertexDecl;
      Ogre::HardwareVertexBufferSharedPtr vBuf;
      Ogre::HardwareIndexBufferSharedPtr iBuf;
      Ogre::HardwareVertexBufferSharedPtr texBuf;
      float *vertices;
      float *texMappings = nullptr;
      uint32_t *indices;

      size_t currOffset = 0;

      // Copy the original submesh. We may need to modify the vertices, and
      // we don't want to change the original.
      common::SubMesh subMesh(_mesh->GetSubMesh(i));

      // Recenter the vertices if requested.
      if (_centerSubmesh)
        subMesh.Center(ignition::math::Vector3d::Zero);

      ogreSubMesh = ogreMesh->createSubMesh();
      ogreSubMesh->useSharedVertices = false;
      if (subMesh.GetPrimitiveType() == common::SubMesh::TRIANGLES)
        ogreSubMesh->operationType = Ogre::RenderOperation::OT_TRIANGLE_LIST;
      else if (subMesh.GetPrimitiveType() == common::SubMesh::LINES)
        ogreSubMesh->operationType = Ogre::RenderOperation::OT_LINE_LIST;
      else if (subMesh.GetPrimitiveType() == common::SubMesh::LINESTRIPS)
        ogreSubMesh->operationType = Ogre::RenderOperation::OT_LINE_STRIP;
      else if (subMesh.GetPrimitiveType() == common::SubMesh::TRIFANS)
        ogreSubMesh->operationType = Ogre::RenderOperation::OT_TRIANGLE_FAN;
      else if (subMesh.GetPrimitiveType() == common::SubMesh::TRISTRIPS)
        ogreSubMesh->operationType = Ogre::RenderOperation::OT_TRIANGLE_STRIP;
      else if (subMesh.GetPrimitiveType() == common::SubMesh::POINTS)
        ogreSubMesh->operationType = Ogre::RenderOperation::OT_POINT_LIST;
      else
        ignerr << "Unknown primitive type["
              << subMesh.GetPrimitiveType() << "]\n";

      ogreSubMesh->vertexData = new Ogre::VertexData();
      vertexData = ogreSubMesh->vertexData;
      vertexDecl = vertexData->vertexDeclaration;

      // The vertexDecl should contain positions, blending weights, normals,
      // diffiuse colors, specular colors, tex coords. In that order.
      vertexDecl->addElement(0, currOffset, Ogre::VET_FLOAT3,
                             Ogre::VES_POSITION);
      currOffset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);

      // TODO: blending weights

      // normals
      if (subMesh.GetNormalCount() > 0)
      {
        vertexDecl->addElement(0, currOffset, Ogre::VET_FLOAT3,
                               Ogre::VES_NORMAL);
        currOffset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);
      }

      // TODO: diffuse colors

      // TODO: specular colors

      // two dimensional texture coordinates
      // allocate buffer for texture mapping, when doing animations, OGRE
      // requires the vertex position and normals reside in their own buffer,
      // see `https://ogrecave.github.io/ogre/api/1.11/_animation.html` under,
      // `Vertex buffer arrangements`.
      currOffset = 0;
      if (subMesh.GetTexCoordCount() > 0)
      {
        vertexDecl->addElement(1, currOffset, Ogre::VET_FLOAT2,
            Ogre::VES_TEXTURE_COORDINATES, 0);
        currOffset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT2);
      }

      // allocate the vertex buffer
      vertexData->vertexCount = subMesh.GetVertexCount();

      vBuf = Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
                 vertexDecl->getVertexSize(0),
                 vertexData->vertexCount,
                 Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY,
                 false);

      if (subMesh.GetTexCoordCount() > 0)
      {
        texBuf = Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
            vertexDecl->getVertexSize(1),
            vertexData->vertexCount,
            Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY,
            false);
      }

      vertexData->vertexBufferBinding->setBinding(0, vBuf);
      vertices = static_cast<float*>(vBuf->lock(
                      Ogre::HardwareBuffer::HBL_DISCARD));

      if (subMesh.GetTexCoordCount() > 0)
      {
        vertexData->vertexBufferBinding->setBinding(1, texBuf);
        texMappings = static_cast<float*>(texBuf->lock(
                        Ogre::HardwareBuffer::HBL_DISCARD));
      }

      if (_mesh->HasSkeleton())
      {
        if (subMesh.GetNodeAssignmentsCount() > 0)
        {
          common::Skeleton *skel = _mesh->GetSkeleton();
          for (unsigned int j = 0; j < subMesh.GetNodeAssignmentsCount(); j++)
          {
            common::NodeAssignment na = subMesh.GetNodeAssignment(j);
            Ogre::VertexBoneAssignment vba;
            vba.vertexIndex = na.vertexIndex;
            vba.boneIndex = ogreSkeleton->getBone(skel->GetNodeByHandle(
                                na.nodeIndex)->GetName())->getHandle();
            vba.weight = na.weight;
            ogreSubMesh->addBoneAssignment(vba);
          }
        }
        else
        {
          // When there is a skeleton associated with the mesh,
          // OGRE requires at least 1 bone assignment to compile the blend
          // weights.
          // The submeshs loaded from COLLADA may not have weights so we need
          // to add a dummy bone assignment for OGRE.
          Ogre::VertexBoneAssignment vba;
          vba.vertexIndex = 0;
          vba.boneIndex = 0;
          vba.weight = 0;
          ogreSubMesh->addBoneAssignment(vba);
        }
      }

      // allocate index buffer
      ogreSubMesh->indexData->indexCount = subMesh.GetIndexCount();

      ogreSubMesh->indexData->indexBuffer =
        Ogre::HardwareBufferManager::getSingleton().createIndexBuffer(
            Ogre::HardwareIndexBuffer::IT_32BIT,
            ogreSubMesh->indexData->indexCount,
            Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY,
            false);

      iBuf = ogreSubMesh->indexData->indexBuffer;
      indices = static_cast<uint32_t*>(
          iBuf->lock(Ogre::HardwareBuffer::HBL_DISCARD));

      unsigned int j;

      // Add all the vertices
      for (j = 0; j < subMesh.GetVertexCount(); j++)
      {
        *vertices++ = subMesh.Vertex(j).X();
        *vertices++ = subMesh.Vertex(j).Y();
        *vertices++ = subMesh.Vertex(j).Z();

        if (subMesh.GetNormalCount() > 0)
        {
          *vertices++ = subMesh.Normal(j).X();
          *vertices++ = subMesh.Normal(j).Y();
          *vertices++ = subMesh.Normal(j).Z();
        }

        if (subMesh.GetTexCoordCount() > 0)
        {
          *texMappings++ = subMesh.TexCoord(j).X();
          *texMappings++ = subMesh.TexCoord(j).Y();
        }
      }

      // Add all the indices
      for (j = 0; j < subMesh.GetIndexCount(); j++)
        *indices++ = subMesh.GetIndex(j);

      const common::Material *material;
      material = _mesh->GetMaterial(subMesh.GetMaterialIndex());
      if (material)
      {
        rendering::Material::Update(material);
        ogreSubMesh->setMaterialName(material->GetName());
      }
      else
      {
        ogreSubMesh->setMaterialName("Gazebo/White");
      }

      // Unlock
      vBuf->unlock();
      iBuf->unlock();
      if (subMesh.GetTexCoordCount() > 0)
      {
        texBuf->unlock();
      }
    }

    ignition::math::Vector3d max = _mesh->Max();
    ignition::math::Vector3d min = _mesh->Min();

    if (_mesh->HasSkeleton())
    {
      min = ignition::math::Vector3d(-1, -1, -1);
      max = ignition::math::Vector3d(1, 1, 1);
    }

    if (!max.IsFinite())
      ignthrow("Max bounding box is not finite[" << max << "]\n");

    if (!min.IsFinite())
      ignthrow("Min bounding box is not finite[" << min << "]\n");

    ogreMesh->_setBounds(Ogre::AxisAlignedBox(
          Ogre::Vector3(min.X(), min.Y(), min.Z()),
          Ogre::Vector3(max.X(), max.Y(), max.Z())),
          false);

    // this line makes clear the mesh is loaded (avoids memory leaks)
    ogreMesh->load();
  }
  catch(Ogre::Exception &e)
  {
    ignerr << "Unable to insert mesh[" << e.getDescription() << "]" << std::endl;
  }
}*/

Ogre::MovableObject *OgreMarker::CreateMesh(const std::string &_meshName,
                                            const std::string &_subMesh,
                                            bool _centerSubmesh,
                                            const std::string &_objName)
{
  if (_meshName.empty())
    return nullptr;

  this->dataPtr->meshName = _meshName;
  this->dataPtr->subMeshName = _subMesh;

  Ogre::MovableObject *obj;
  std::string objName = _objName;
  std::string meshName = _meshName;
  meshName += _subMesh.empty() ? "" : "::" + _subMesh;

  if (objName.empty())
    objName = this->scene->Name() + "_ENTITY_" + meshName;

  this->InsertMesh(_meshName, _subMesh, _centerSubmesh);

  obj = (Ogre::MovableObject*)
      (this->dataPtr->sceneNode->getCreator()->createEntity(objName,
      meshName));

  return obj;
}

//////////////////////////////////////////////////
void OgreMarker::SetMaterial(MaterialPtr _material, bool _unique)
{
  _material = (_unique) ? _material->Clone() : _material;

  OgreMaterialPtr derived =
    std::dynamic_pointer_cast<OgreMaterial>(_material);

  if (!derived)
  {
    ignerr << "Cannot assign material created by another render-engine"
      << std::endl;

    return;
  }

  this->SetMaterialImpl(derived);
}

//////////////////////////////////////////////////
void OgreMarker::SetMaterialImpl(OgreMaterialPtr _material)
{
  std::string materialName = _material->Name();
  Ogre::MaterialPtr ogreMaterial = _material->Material();
  //this->dataPtr->movableObject->setMaterialName(0, materialName);
  this->dataPtr->material = _material;

  this->dataPtr->material->SetReceiveShadows(false);
  this->dataPtr->material->SetLightingEnabled(false);
}

//////////////////////////////////////////////////
MaterialPtr OgreMarker::Material() const
{
  return this->dataPtr->material;
}

void OgreMarker::SetRenderOperation(const Type _type)
{
  // TODO update a parsing of the type to create a dynamic
  // renderable if it is those 6 types and update the movable
  // object otherwise to be the primitive types
  /*switch (_type)
  {
    case NONE:
    case BOX:
      
    case CYLINDER:
    case SPHERE:
    case TEXT:
    case LINE_STRIP:
    case LINE_LIST:
    case POINTS:
    case TRIANGLE_FAN:
    case TRIANGLE_LIST:
    case TRIANGLE_STRIP:
    this->dataPtr->dynamicRenderable->SetOperationType(_type);
  
  }*/
}

Type OgreMarker::RenderOperation() const
{
  return this->dataPtr->dynamicRenderable->OperationType();
}

void OgreMarker::SetPoint(const unsigned int _index,
    const ignition::math::Vector3d &_value)
{
  this->dataPtr->dynamicRenderable->SetPoint(_index, _value);
}

void OgreMarker::AddPoint(const double _x, const double _y, const double _z,
    const ignition::math::Color &_color)
{
  this->dataPtr->dynamicRenderable->AddPoint(_x, _y, _z, _color);
}

void OgreMarker::AddPoint(const ignition::math::Vector3d &_pt,
    const ignition::math::Color &_color)
{
  this->dataPtr->dynamicRenderable->AddPoint(_pt, _color);
}

void OgreMarker::ClearPoints()
{
  this->dataPtr->dynamicRenderable->Clear();
}

void OgreMarker::SetType(Type _type)
{
  this->dataPtr->type = _type;
  SetRenderOperation(_type);
}

void OgreMarker::SetAction(Action _action)
{
  this->dataPtr->action = _action;
}

void OgreMarker::SetVisibility(Visibility _visibility)
{
  this->dataPtr->visibility = _visibility;
}

Type OgreMarker::getType() const
{
  return this->dataPtr->type;
}

Action OgreMarker::getAction() const
{
  return this->dataPtr->action;
}

Visibility OgreMarker::getVisibility() const
{
  return this->dataPtr->visibility;
}

