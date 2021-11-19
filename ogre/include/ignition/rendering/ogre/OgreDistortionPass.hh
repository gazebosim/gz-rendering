/*
 * Copyright (C) 2021 Open Source Robotics Foundation
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
#ifndef IGNITION_RENDERING_OGRE_OGREDISTORTIONPASS_HH_
#define IGNITION_RENDERING_OGRE_OGREDISTORTIONPASS_HH_

#include <memory>

#include <ignition/common/SuppressWarning.hh>

#include "ignition/rendering/base/BaseDistortionPass.hh"
#include "ignition/rendering/ogre/OgreIncludes.hh"
#include "ignition/rendering/ogre/OgreRenderPass.hh"
#include "ignition/rendering/ogre/Export.hh"

namespace ignition
{
  namespace rendering
  {
    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
    /* \class OgreDistortionPass OgreDistortionPass.hh \
     * ignition/rendering/ogre/OgreDistortionPass.hh
     */
    /// \brief Ogre implementation of the DistortionPass class
    class IGNITION_RENDERING_OGRE_VISIBLE OgreDistortionPass :
      public BaseDistortionPass<OgreRenderPass>
    {
      /// \brief Constructor
      public: OgreDistortionPass();

      /// \brief Destructor
      public: virtual ~OgreDistortionPass();

      // Documentation inherited
      public: void PreRender() override;

      // Documentation inherited
      public: void Destroy() override;

      // Documentation inherited
      public: void CreateRenderPass() override;

      /// \brief Distortion compositor.
      public: Ogre::CompositorInstance *distortionInstance = nullptr;
    };
    }
  }
}
#endif
