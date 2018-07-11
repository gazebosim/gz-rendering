/*
 * Copyright (C) 2012 Open Source Robotics Foundation
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

#ifndef IGNITION_RENDERING_OGRE_OGREDEPTHCAMERA_HH_
#define IGNITION_RENDERING_OGRE_OGREDEPTHCAMERA_HH_

#ifndef _WIN32
  #include <dirent.h>
#else
  // Ensure that Winsock2.h is included before Windows.h, which can get
  // pulled in by anybody (e.g., Boost).
  #include <Winsock2.h>
  #include "ignition/common/win_dirent.h"
#endif

#include <memory>
#include <string>

#include "ignition/rendering/RenderTypes.hh"
#include "ignition/rendering/base/BaseDepthCamera.hh"
#include "ignition/rendering/ogre/OgreCamera.hh"
#include "ignition/rendering/ogre/OgreConversions.hh"
#include "ignition/rendering/ogre/OgreIncludes.hh"
#include "ignition/rendering/ogre/OgreRenderTarget.hh"
#include "ignition/rendering/ogre/OgreRenderTypes.hh"
#include "ignition/rendering/ogre/OgreScene.hh"
#include "ignition/rendering/ogre/OgreSensor.hh"
#include "ignition/rendering/ogre/OgreSelectionBuffer.hh"

#include "ignition/common/Event.hh"
#include "ignition/common/Console.hh"


namespace Ogre
{
  class Material;
  class RenderTarget;
  class Texture;
  class Viewport;
}

namespace ignition
{
  namespace rendering
  {
    // Forward declare private data.
    class OgreDepthCameraPrivate;

    /// \class DepthCamera DepthCamera.hh rendering/rendering.hh
    /// \brief Depth camera used to render depth data into an image buffer
    class IGNITION_RENDERING_OGRE_VISIBLE OgreDepthCamera :
      public BaseDepthCamera<OgreSensor>
    {
      /// \brief Constructor
      /// \param[in] _namePrefix Unique prefix name for the camera.
      /// \param[in] _scene Scene that will contain the camera
      /// \param[in] _autoRender Almost everyone should leave this as true.
      protected: OgreDepthCamera();

      /// \brief Destructor
      public: virtual ~OgreDepthCamera();

      /// \brief Initialize the camera
      public: virtual void Init() override;

      /// \brief Create a texture which will hold the depth data
      /// \param[in] _textureName Name of the texture to create
      public: virtual void CreateDepthTexture(const std::string &_textureName);

      /// \brief Render the camera
      public: virtual void PostRender();

      /// \brief All things needed to get back z buffer for depth data
      /// \return The z-buffer as a float array
      public: virtual const float *DepthData() const override;

      /// \brief Set the render target, which renders the depth data
      /// \param[in] _target Pointer to the render target
      public: virtual void SetDepthTarget(OgreRenderTargetPtr &_target);

      /// \brief Connect a to the new depth image signal
      /// \param[in] _subscriber Subscriber callback function
      /// \return Pointer to the new Connection. This must be kept in scope
      public: virtual ignition::common::ConnectionPtr ConnectNewDepthFrame(
          std::function<void(const float *, unsigned int, unsigned int,
          unsigned int, const std::string &)>  _subscriber) override;

      /// \brief Connect a to the new rgb point cloud signal
      /// \param[in] _subscriber Subscriber callback function
      /// \return Pointer to the new Connection. This must be kept in scope
      public: virtual ignition::common::ConnectionPtr ConnectNewRGBPointCloud(
          std::function<void(const float *, unsigned int, unsigned int,
          unsigned int, const std::string &)>  _subscriber) override;

      /// \brief Implementation of the render call
      public: virtual void Render() override;

      /// \brief Update a render target
      /// \param[in] _target Render target to update
      /// \param[in] _material Material to use
      /// \param[in] _matName Material name
      protected: void UpdateRenderTarget(OgreRenderTargetPtr &_target,
                                       Ogre::Material *_material,
                                       const std::string &_matName);

      /// \brief Limit field of view taking care of using a valid value for
      /// an OGRE camera.
      /// \param[in] _fov expected field of view
      /// \return valid field of view
      protected: static double LimitFOV(const double _fov);

      /// \brief Communicates that a frams was rendered
      protected: bool newData = false;

      protected: bool captureData = false;

      /// \brief Pointer to the depth texture
      protected: Ogre::Texture *depthTexture = nullptr;

      /// \brief Pointer to the depth target
      protected: OgreRenderTargetPtr depthTarget;

      /// \brief Pointer to the depth viewport
      protected: Ogre::Viewport *depthViewport = nullptr;

      /// \brief Pointer to the ogre camera
      protected: Ogre::Camera *ogreCamera = nullptr;

      /// \internal
      /// \brief Pointer to private data.
      private: std::unique_ptr<OgreDepthCameraPrivate> dataPtr = nullptr;
    };
  }
}
#endif
