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
    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE
    {
    /// \internal
    /// \brief Private data for the OgreDepthCamera class
    class OgreDepthCameraPrivate
    {
      /// \brief The depth buffer
      public: float *depthBuffer = nullptr;

      /// \brief The depth material
      public: Ogre::Material *depthMaterial = nullptr;

      /// \brief True to generate point clouds
      public: bool outputPoints = false;

      /// \brief Point cloud data buffer
      public: float *pcdBuffer = nullptr;

      /// \brief Point cloud view port
      public: Ogre::Viewport *pcdViewport = nullptr;

      /// \brief Point cloud material
      public: Ogre::Material *pcdMaterial = nullptr;

      /// \brief Point cloud texture
      public: OgreRenderTexturePtr pcdTexture;

      /// \brief Event used to signal rgb point cloud data
      public: ignition::common::EventT<void(const float *,
                  unsigned int, unsigned int, unsigned int,
                  const std::string &)> newRGBPointCloud;

      /// \brief Event used to signal depth data
      public: ignition::common::EventT<void(const float *,
                  unsigned int, unsigned int, unsigned int,
                  const std::string &)> newDepthFrame;
    };

    /// \class DepthCamera DepthCamera.hh rendering/rendering.hh
    /// \brief Depth camera used to render depth data into an image buffer
    class IGNITION_RENDERING_OGRE_VISIBLE OgreDepthCamera :
      public BaseDepthCamera<OgreSensor>
    {
      /// \brief Constructor
      protected: OgreDepthCamera();

      /// \brief Destructor
      public: virtual ~OgreDepthCamera();

      /// \brief Initialize the camera
      public: virtual void Init() override;

      /// \brief Create a texture which will hold the depth data
      public: virtual void CreateDepthTexture() override;

      /// \brief Render the camera
      public: virtual void PostRender();

      /// \brief All things needed to get back z buffer for depth data
      /// \return The z-buffer as a float array
      public: virtual const float *DepthData() const override;

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

      public: virtual void SetFarClipPlane(const double _far) override;

      public: virtual void SetNearClipPlane(const double _near) override;

      /// \brief Get the near clip distance
      /// \return Near clip distance
      public: double NearClipPlane() const override;

      /// \brief Get the far clip distance
      /// \return Far clip distance
      public: double FarClipPlane() const override;

      /// \brief Update a render target
      /// \param[in] _target Render target to update
      /// \param[in] _material Material to use
      /// \param[in] _matName Material name
      protected: void UpdateRenderTarget(OgreRenderTexturePtr _target,
                                       Ogre::Material *_material,
                                       const std::string &_matName);

      protected: virtual RenderTargetPtr RenderTarget() const override;

      /// \brief Limit field of view taking care of using a valid value for
      /// an OGRE camera.
      /// \param[in] _fov expected field of view
      /// \return valid field of view
      protected: static double LimitFOV(const double _fov);

      protected: void CreateCamera();

      /// \brief Communicates that a frams was rendered
      protected: bool newData = false;

      protected: bool captureData = false;

      /// \brief Pointer to the depth texture
      protected: OgreRenderTexturePtr depthTexture;

      /// \brief Pointer to the depth viewport
      protected: Ogre::Viewport *depthViewport = nullptr;

      /// \brief Pointer to the ogre camera
      protected: Ogre::Camera *ogreCamera;

      /// \internal
      /// \brief Pointer to private data.
      private: std::unique_ptr<OgreDepthCameraPrivate> dataPtr = nullptr;

      private: friend class OgreScene;
      private: friend class OgreRayQuery;
    };
    }
  }
}
#endif
