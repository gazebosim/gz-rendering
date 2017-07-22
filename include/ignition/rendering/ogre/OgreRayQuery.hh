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
#ifndef IGNITION_RENDERING_OGRE_OGRERAYQUERY_HH_
#define IGNITION_RENDERING_OGRE_OGRERAYQUERY_HH_

#include "ignition/rendering/base/BaseRayQuery.hh"

namespace Ogre
{
  class RaySceneQuery;
}

namespace ignition
{
  namespace rendering
  {
    /// \class OgreRayQuery OgreRayQuery.hh ignition/rendering/base/OgreRayQuery.hh
    /// \brief A Ray Query class used for computing ray object intersections
    class IGNITION_VISIBLE OgreRayQuery
        : public virtual BaseRayQuery
    {
      /// \brief Constructor
      public: OgreRayQuery(const math::Vector3d &_origin,
                const math::Vector3d &_dir);

      /// \brief Deconstructor
      public: virtual ~OgreRayQuery();

      /// \brief Compute intersections
      /// \param[] Scene where the ray query will be executed
      /// \param[out] A vector of intersection results
      /// \return True if results are not empty
      public: virtual bool Intersect(const ScenePtr &_scene,
                std::vector<RayQueryResult> &_result);

      /// \brief Ogre ray scene query object for computing intersection.
      private: Ogre::RaySceneQuery *rayQuery = nullptr;
    };
  }
}
#endif
