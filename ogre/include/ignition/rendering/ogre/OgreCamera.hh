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

#include <string>

#include "ignition/rendering/base/BaseCamera.hh"
#include "ignition/rendering/ogre/OgreRenderTypes.hh"
#include "ignition/rendering/ogre/OgreSensor.hh"
#include "ignition/rendering/ogre/OgreSelectionBuffer.hh"

namespace Ogre
{
  class Camera;
}

namespace ignition
{
  namespace rendering
  {
    class IGNITION_RENDERING_OGRE_VISIBLE OgreCamera :
      public BaseCamera<OgreSensor>
    {
      protected: OgreCamera();

      public: virtual ~OgreCamera();

      public: virtual void SetHFOV(const math::Angle &_angle);

      public: virtual double AspectRatio() const;

      public: virtual void SetAspectRatio(const double _ratio);

      public: virtual unsigned int AntiAliasing() const;

      public: virtual void SetAntiAliasing(const unsigned int _aa);

      public: virtual void SetFarClipPlane(const double _far);

      public: virtual void SetNearClipPlane(const double _near);

      public: virtual math::Color BackgroundColor() const;

      public: virtual void SetBackgroundColor(const math::Color &_color);

      public: virtual void Render();

      public: virtual RenderWindowPtr CreateRenderWindow();

      public: virtual math::Matrix4d ProjectionMatrix() const;

      public: virtual math::Matrix4d ViewMatrix() const;

      public: void SetVFOV(double cameraVFOV) const;

      /// \brief Get the near clip distance
      /// \return Near clip distance
      public: double NearClip() const;

      /// \brief Get the far clip distance
      /// \return Far clip distance
      public: double FarClip() const;

      // Documentation inherited
      public: virtual VisualPtr VisualAt(const ignition::math::Vector2i
                  &_mousePos);

      // Documentation Inherited.
      // \sa Camera::SetMaterial(const MaterialPtr &)
      public: virtual void SetMaterial(
                  const MaterialPtr &_material) override;

      // Documentation inherited.
      public: virtual GLuint RenderTextureGLId() const;

      protected: virtual RenderTargetPtr RenderTarget() const;

      protected: virtual void Init();

      protected: virtual void SetSelectionBuffer();

      protected: virtual void CreateRenderTexture();

      private: void CreateCamera();

      protected: Ogre::Camera *ogreCamera = nullptr;

      protected: OgreSelectionBuffer *selectionBuffer = nullptr;

      protected: OgreRenderTargetPtr renderTexture;

      protected: math::Color backgroundColor;

      private: friend class OgreScene;
      private: friend class OgreRayQuery;
    };
  }
}
#endif
