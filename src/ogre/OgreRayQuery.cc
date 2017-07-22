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


#include "ignition/rendering/ogre/OgreIncludes.hh"
#include "ignition/rendering/ogre/OgreConversions.hh"
#include "ignition/rendering/ogre/OgreRayQuery.hh"
#include "ignition/rendering/ogre/OgreScene.hh"

using namespace ignition;
using namespace rendering;

//////////////////////////////////////////////////
OgreRayQuery::OgreRayQuery(const ignition::math::Vector3d &_origin,
    const ignition::math::Vector3d &_direction)
    : BaseRayQuery(_origin, _direction)
{
}

//////////////////////////////////////////////////
OgreRayQuery::~OgreRayQuery()
{
}

//////////////////////////////////////////////////
bool OgreRayQuery::Intersect(const ScenePtr &_scene, std::vector<RayQueryResult> &_results)
{
  Ogre::Ray mouseRay(OgreConversions::Convert(this->origin),
      OgreConversions::Convert(this->direction));

  OgreScenePtr scene = std::dynamic_pointer_cast<OgreScene>(_scene);
  if (!scene)
    return false;

  if (!this->rayQuery)
    this->rayQuery =
      scene->OgreSceneManager()->createRayQuery(mouseRay);
  this->rayQuery->setSortByDistance(true);

  _results.clear();

  // Perform the scene query
  Ogre::RaySceneQueryResult &result = this->rayQuery->execute();
  // Iterate over all the results.
  for (auto iter = result.begin(); iter != result.end(); ++iter)
  {
    RayQueryResult r;
    r.distance = iter->distance;
//    r.visual
    _results.push_back(r);
  }

  return !_results.empty();
}

