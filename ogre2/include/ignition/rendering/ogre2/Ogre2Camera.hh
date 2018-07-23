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
#ifndef IGNITION_RENDERING_OGRE2_OGRE2CAMERA_HH_
#define IGNITION_RENDERING_OGRE2_OGRE2CAMERA_HH_

#include <string>

#include "ignition/rendering/base/BaseCamera.hh"
#include "ignition/rendering/ogre2/Ogre2RenderTypes.hh"
#include "ignition/rendering/ogre2/Ogre2Sensor.hh"
//#include "ignition/rendering/ogre2/OgreSelectionBuffer.hh"

namespace Ogre
{
  class Camera;
}

namespace ignition
{
  namespace rendering
  {
    class IGNITION_RENDERING_OGRE2_VISIBLE Ogre2Camera :
      public BaseCamera<Ogre2Sensor>
    {
      protected: Ogre2Camera();

      public: virtual ~Ogre2Camera();

      public: virtual void SetHFOV(const math::Angle &_angle) override;

      public: virtual double AspectRatio() const override;

      public: virtual void SetAspectRatio(const double _ratio) override;

      public: virtual unsigned int AntiAliasing() const override;

      public: virtual void SetAntiAliasing(const unsigned int _aa) override;

      public: virtual void SetFarClipPlane(const double _far) override;

      public: virtual void SetNearClipPlane(const double _near) override;

      public: virtual math::Color BackgroundColor() const;

      public: virtual void SetBackgroundColor(const math::Color &_color);

      public: virtual void Render() override;

      public: virtual RenderWindowPtr CreateRenderWindow() override;

      public: virtual math::Matrix4d ProjectionMatrix() const override;

      public: virtual math::Matrix4d ViewMatrix() const override;

      // Documentation inherited
      public: virtual VisualPtr VisualAt(const ignition::math::Vector2i
                  &_mousePos) override;

      // Documentation Inherited.
      // \sa Camera::SetMaterial(const MaterialPtr &)
      public: virtual void SetMaterial(
                  const MaterialPtr &_material) override;

      protected: virtual RenderTargetPtr RenderTarget() const override;

      protected: virtual void Init() override;

      protected: virtual void SetSelectionBuffer();

      private: void CreateCamera();

      protected: virtual void CreateRenderTexture();

      protected: Ogre::Camera *ogreCamera = nullptr;

//      protected: OgreSelectionBuffer *selectionBuffer = nullptr;

      protected: Ogre2RenderTargetPtr renderTexture;

      protected: math::Color backgroundColor;

      private: friend class Ogre2Scene;
      // private: friend class Ogre2RayQuery;
    };
  }
}
#endif
