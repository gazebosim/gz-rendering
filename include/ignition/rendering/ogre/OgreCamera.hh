/*
 * Copyright (C) 2015 Open Source Robotics Foundation
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
#ifndef IGNITION_RENDERING_OGRE_OGRECAMERA_HH_
#define IGNITION_RENDERING_OGRE_OGRECAMERA_HH_

#include "ignition/rendering/base/BaseCamera.hh"
#include "ignition/rendering/ogre/OgreRenderTypes.hh"
#include "ignition/rendering/ogre/OgreSensor.hh"

namespace Ogre
{
  class Camera;
}

namespace ignition
{
  namespace rendering
  {
    class IGNITION_VISIBLE OgreCamera :
      public BaseCamera<OgreSensor>
    {
      protected: OgreCamera();

      public: virtual ~OgreCamera();

      public: virtual PixelFormat ImageFormat() const;

      public: virtual void SetImageFormat(PixelFormat _format);

      public: virtual math::Angle HFOV() const;

      public: virtual void SetHFOV(const math::Angle &_angle);

      public: virtual double AspectRatio() const;

      public: virtual void SetAspectRatio(double _ratio);

      public: virtual unsigned int AntiAliasing() const;

      public: virtual void SetAntiAliasing(unsigned int _aa);

      public: virtual math::Color BackgroundColor() const;

      public: virtual void SetBackgroundColor(const math::Color &_color);

      public: virtual void Render();

      public: virtual ScenePtr GetScene() const;

      protected: virtual RenderTexturePtr RenderTexture() const;

      public: virtual void CameraToViewportRay(const int _screenx, const int _screeny,
                            ignition::math::Vector3d &_origin,
                            ignition::math::Vector3d &_dir) const;

      protected: virtual unsigned int ViewportHeight() const;

      protected: virtual unsigned int ViewportWidth() const;

      protected: virtual void Init();

      private: void CreateCamera();

      protected: virtual void CreateRenderTexture();

      protected: Ogre::Camera *ogreCamera;

      protected: OgreRenderTexturePtr renderTexture;

      protected: OgreRenderTarget *ogreRenderTarget;

      protected: math::Angle xfov;

      protected: math::Color backgroundColor;

      private: friend class OgreScene;
    };
  }
}
#endif
