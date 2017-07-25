/*
 * Copyright (C) 2017 Open Source Robotics Foundation
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

#include <typeinfo>

#include <ignition/common/Console.hh>

#include "ignition/rendering/ogre/OgreIncludes.hh"
#include "ignition/rendering/ogre/OgreCamera.hh"
#include "ignition/rendering/ogre/OgreConversions.hh"
#include "ignition/rendering/ogre/OgreRayQuery.hh"
#include "ignition/rendering/ogre/OgreScene.hh"

namespace ignition
{
  namespace rendering
  {
    class OgreRayQueryPrivate
    {
      /// \brief Ogre ray scene query object for computing intersection.
      public: Ogre::RaySceneQuery *rayQuery = nullptr;
    };
  }
}


using namespace ignition;
using namespace rendering;

//////////////////////////////////////////////////
OgreRayQuery::OgreRayQuery()
    : dataPtr(new OgreRayQueryPrivate)
{
}

//////////////////////////////////////////////////
OgreRayQuery::~OgreRayQuery()
{
}

//////////////////////////////////////////////////
void OgreRayQuery::SetFromCamera(const CameraPtr &_camera,
    const ignition::math::Vector2d &_coord)
{
  OgreCameraPtr camera = std::dynamic_pointer_cast<OgreCamera>(_camera);
  Ogre::Ray ray =
      camera->ogreCamera->getCameraToViewportRay(_coord.X(), _coord.Y());
  this->origin = OgreConversions::Convert(ray.getOrigin());
  this->direction = OgreConversions::Convert(ray.getDirection());
}

//////////////////////////////////////////////////
bool OgreRayQuery::Intersect(std::vector<RayQueryResult> &_results)
{
  OgreScenePtr scene = std::dynamic_pointer_cast<OgreScene>(this->Scene());

  if (!scene)
    return false;

  Ogre::Ray mouseRay(OgreConversions::Convert(this->origin),
      OgreConversions::Convert(this->direction));

  if (!this->dataPtr->rayQuery)
  {
    this->dataPtr->rayQuery =
        scene->OgreSceneManager()->createRayQuery(mouseRay);
  }
  this->dataPtr->rayQuery->setSortByDistance(true);

  _results.clear();

  // Perform the scene query
  Ogre::RaySceneQueryResult &result = this->dataPtr->rayQuery->execute();
  // Iterate over all the results.
  for (auto iter = result.begin(); iter != result.end(); ++iter)
  {
    if (iter->distance <= 0.0)
      continue;

    if (iter->movable && iter->movable->getVisible())
    {
      unsigned int id = 0;
      auto userAny = iter->movable->getUserObjectBindings().getUserAny();
      if (!userAny.isEmpty() && userAny.getType() == typeid(unsigned int))
      {
        id = Ogre::any_cast<unsigned int>(userAny);
        RayQueryResult r;
        r.distance = iter->distance;
        r.point = OgreConversions::Convert(mouseRay.getPoint(iter->distance));
        r.id = id;
        _results.push_back(r);
      }
    }
  }

  return !_results.empty();
}

