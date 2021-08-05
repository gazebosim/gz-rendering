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

#include <ignition/common/Console.hh>
#include <ignition/common/Mesh.hh>
#include <ignition/common/MeshManager.hh>
#include <ignition/common/SubMesh.hh>

#include "ignition/rendering/ogre2/Ogre2Includes.hh"
#include "ignition/rendering/ogre2/Ogre2Camera.hh"
#include "ignition/rendering/ogre2/Ogre2Conversions.hh"
#include "ignition/rendering/ogre2/Ogre2RayQuery.hh"
#include "ignition/rendering/ogre2/Ogre2Scene.hh"
#include "ignition/rendering/ogre2/Ogre2SelectionBuffer.hh"

/// \brief Private data class for Ogre2RayQuery
class ignition::rendering::Ogre2RayQueryPrivate
{
  /// \brief Ogre ray scene query object for computing intersection.
  public: Ogre::RaySceneQuery *rayQuery = nullptr;

  //// \brief Pointer to camera
  public: Ogre2CameraPtr camera;

  /// Image pos to cast the ray from
  public: math::Vector2i imgPos;
};

using namespace ignition;
using namespace rendering;

//////////////////////////////////////////////////
Ogre2RayQuery::Ogre2RayQuery()
    : dataPtr(new Ogre2RayQueryPrivate)
{
}

//////////////////////////////////////////////////
Ogre2RayQuery::~Ogre2RayQuery()
{
}

//////////////////////////////////////////////////
void Ogre2RayQuery::SetFromCamera(const CameraPtr &_camera,
    const math::Vector2d &_coord)
{
  // convert to nomalized screen pos for ogre
  math::Vector2d screenPos((_coord.X() + 1.0) / 2.0, (_coord.Y() - 1.0) / -2.0);
  Ogre2CameraPtr camera = std::dynamic_pointer_cast<Ogre2Camera>(_camera);
  Ogre::Ray ray =
      camera->ogreCamera->getCameraToViewportRay(screenPos.X(), screenPos.Y());

  this->origin = Ogre2Conversions::Convert(ray.getOrigin());
  this->direction = Ogre2Conversions::Convert(ray.getDirection());

  this->dataPtr->camera = camera;

  this->dataPtr->imgPos.X() = screenPos.X() * this->dataPtr->camera->ImageWidth();
  this->dataPtr->imgPos.Y() =
      screenPos.Y() * this->dataPtr->camera->ImageHeight();
}

//////////////////////////////////////////////////
RayQueryResult Ogre2RayQuery::ClosestPoint()
{
  RayQueryResult result;
  result.distance = -1;

  // hack to create selection buffer if it does not exist yet
  if (!this->dataPtr->camera->SelectionBuffer())
    this->dataPtr->camera->VisualAt(this->dataPtr->imgPos);

  Ogre::Item *ogreItem =
      this->dataPtr->camera->SelectionBuffer()->OnSelectionClick(
      this->dataPtr->imgPos.X(), this->dataPtr->imgPos.Y());

  if (!ogreItem)
    return result;

  Ogre::Any userAny = ogreItem->getUserObjectBindings().getUserAny();

  if (userAny.isEmpty() || userAny.getType() != typeid(unsigned int))
    return result;

  double distance = -1.0;

  Ogre::Ray mouseRay(Ogre2Conversions::Convert(this->origin),
      Ogre2Conversions::Convert(this->direction));

  // mesh factory creates name with ::CENTER or ::ORIGINAL depending on
  // the params passed in the MeshDescriptor when loading the mesh
  // so strip off the suffix
  std::string meshName = ogreItem->getMesh()->getName();
  size_t idx = meshName.find("::");
  if (idx != std::string::npos)
    meshName = meshName.substr(0, idx);

  const common::Mesh *mesh =
       common::MeshManager::Instance()->MeshByName(meshName);

  if (!mesh)
    return result;

  Ogre::Matrix4 transform = ogreItem->_getParentNodeFullTransform();

  // test for hitting individual triangles on the mesh
  for (unsigned int j = 0; j < mesh->SubMeshCount(); ++j)
  {
    auto s = mesh->SubMeshByIndex(j);
    auto submesh = s.lock();
    if (!submesh || submesh->VertexCount() < 3u)
      continue;
    unsigned int indexCount = submesh->IndexCount();
    for (unsigned int k = 0; k < indexCount; k += 3)
    {
      if (indexCount <= k+2)
        continue;

      ignition::math::Vector3d vertexA =
        submesh->Vertex(submesh->Index(k));
      ignition::math::Vector3d vertexB =
        submesh->Vertex(submesh->Index(k+1));
      ignition::math::Vector3d vertexC =
        submesh->Vertex(submesh->Index(k+2));

      Ogre::Vector3 worldVertexA =
          transform * Ogre2Conversions::Convert(vertexA);
      Ogre::Vector3 worldVertexB =
          transform * Ogre2Conversions::Convert(vertexB);
      Ogre::Vector3 worldVertexC =
          transform * Ogre2Conversions::Convert(vertexC);

      // check for a hit against this triangle
      std::pair<bool, Ogre::Real> hit = Ogre::Math::intersects(mouseRay,
          worldVertexA, worldVertexB, worldVertexC,
          true, false);

      // if it was a hit check if its the closest
      if (hit.first &&
          (distance < 0.0f || hit.second < distance))
      {
        // this is the closest so far, save it off
        distance = hit.second;
        result.distance = distance;
        result.point =
            Ogre2Conversions::Convert(mouseRay.getPoint(distance));
        result.objectId = Ogre::any_cast<unsigned int>(userAny);
      }
    }
  }

  return result;
}
