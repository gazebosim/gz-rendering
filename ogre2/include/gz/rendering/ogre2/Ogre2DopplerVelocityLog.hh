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

#ifndef GZ_RENDERING_OGRE2_OGRE2DOPPLERVELOCITYLOG_HH_
#define GZ_RENDERING_OGRE2_OGRE2DOPPLERVELOCITYLOG_HH_

#include <gz/rendering/DVL.hh>
#include <gz/rendering/base/BaseDVL.hh>
#include "ignition/rendering/ogre2/Ogre2RenderTarget.hh"


namespace ignition{
  namespace rendering
  {
    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {

    class Ogre2DopplerVelocityLogPrivate;

    class IGNITION_RENDERING_VISIBLE Ogre2DopplerVelocityLog :
      public virtual BaseDVL<Ogre2Sensor>,
      public virtual Ogre2ObjectInterface
    {
      public: Ogre2DopplerVelocityLog ();

      public: virtual ~Ogre2DopplerVelocityLog ();

      public: virtual void Init() override;

      // Documentation inherited
      public: virtual void Destroy() override;

      // Documentation inherited
      public: virtual void PreRender() override;

      // Documentation inherited
      public: virtual void PostRender() override;

      // Documentation inherited
      public: virtual ignition::common::ConnectionPtr
        ConnectNewDepthVisibilitymapFrame(
        std::function<void(const uint8_t *, unsigned int, unsigned int,
        unsigned int, const std::string &)>  _subscriber) override;

      // Documentation inherited
      public: virtual uint8_t *VisibilityDepthData() const override;

      // Documentation inherited
      public: virtual void Render() override;

      /// \brief Get a pointer to the render target.
      /// \return Pointer to the render target
      protected: virtual RenderTargetPtr RenderTarget() const override;

      // Documentation inherited.
      public: virtual Ogre::Camera *OgreCamera() const override;

      /// \brief Create the camera.
      protected: void CreateCamera();

      /// \brief Create render texture
      protected: virtual void CreateRenderTexture();

      // Documentation inherited
      protected: virtual void CreateDopplerVelocityLogTexture() override;

      /// \brief Pointer to the ogre camera
      protected: Ogre::Camera *ogreCamera = nullptr;

      /// \internal
      /// \brief Pointer to private data.
      private: std::unique_ptr<Ogre2DopplerVelocityLogPrivate> dataPtr;

      /// \brief Make scene our friend so it can create a camera
      private: friend class Ogre2Scene;
    };
    }
  }
}
#endif