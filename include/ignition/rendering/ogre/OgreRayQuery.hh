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

#include <memory>

#include "ignition/rendering/base/BaseRayQuery.hh"
#include "ignition/rendering/ogre/OgreObject.hh"
#include "ignition/rendering/ogre/OgreRenderTypes.hh"

namespace ignition
{
  namespace rendering
  {
    // forward declaration
    class OgreRayQueryPrivate;

    /// \class OgreRayQuery OgreRayQuery.hh ignition/rendering/base/OgreRayQuery.hh
    /// \brief A Ray Query class used for computing ray object intersections
    class IGNITION_VISIBLE OgreRayQuery :
        public BaseRayQuery<OgreObject>
    {
      /// \brief Constructor
      protected: OgreRayQuery();

      /// \brief Deconstructor
      public: virtual ~OgreRayQuery();

      /// \brief Create the ray query from camera
      /// \param[in] _camera Camera to construct ray
      /// \param[in] _coord normalized device coords [-1, +1]
      public: virtual void SetFromCamera(const CameraPtr &_camera,
                const math::Vector2d &_coord);

      /// \brief Compute intersections
      /// \param[out] A vector of intersection results
      /// \return True if results are not empty
      public: virtual bool Intersect(std::vector<RayQueryResult> &_result);

      /// \brief Private data pointer
      private: std::unique_ptr<OgreRayQueryPrivate> dataPtr;

      /// \brief Pointer to friend scene class for creating ray query
      private: friend class OgreScene;
    };
  }
}
#endif
