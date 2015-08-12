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
#ifndef _IGNITION_RENDERING_OGRERENDERTARGET_HH_
#define _IGNITION_RENDERING_OGRERENDERTARGET_HH_

#include "gazebo/common/Color.hh"
#include "ignition/rendering/base/BaseRenderTypes.hh"
#include "ignition/rendering/base/BaseRenderTarget.hh"
#include "ignition/rendering/ogre/OgreRenderTypes.hh"
#include "ignition/rendering/ogre/OgreIncludes.hh"
#include "ignition/rendering/ogre/OgreObject.hh"

namespace Ogre
{
  class Camera;
  class RenderTarget;
  class Texture;
}

namespace ignition
{
  namespace rendering
  {
    class IGNITION_VISIBLE OgreRenderTarget :
      public virtual BaseRenderTarget<OgreObject>
    {
      protected: OgreRenderTarget();

      public: virtual ~OgreRenderTarget();

      public: virtual unsigned int GetWidth() const;

      public: virtual unsigned int GetHeight() const;

      public: virtual void GetImage(Image &_image) const;

      public: virtual void Update();

      public: virtual void Destroy() = 0;

      protected: virtual Ogre::RenderTarget *GetOgreRenderTarget() const = 0;

      protected: virtual void Initialize();

      protected: OgreScenePtr scene;

      protected: Ogre::Camera *ogreCamera;

      protected: Ogre::Viewport *ogreViewport;

      protected: Ogre::ColourValue ogreBackgroundColor;
    };

    class IGNITION_VISIBLE OgreRenderTexture :
      public virtual BaseRenderTexture<OgreRenderTarget>
    {
      protected: OgreRenderTexture();

      public: virtual ~OgreRenderTexture();

      public: virtual void Destroy();

      protected: virtual Ogre::RenderTarget *GetOgreRenderTarget() const;

      protected: virtual void RebuildImpl();

      protected: Ogre::Texture *ogreTexture;

      protected: Ogre::PixelFormat ogreFormat;

      private: friend class OgreScene;

      private: friend class OgreRenderTextureBuilder;
    };

    class IGNITION_VISIBLE OgreRenderTextureBuilder :
      public virtual BaseRenderTextureBuilder
    {
      public: OgreRenderTextureBuilder(OgreScenePtr _scene);

      public: virtual ~OgreRenderTextureBuilder();

      public: virtual Ogre::Camera *GetCamera() const;

      public: virtual void SetCamera(Ogre::Camera *_camera);

      public: virtual std::string GetName() const;

      public: virtual void SetName(const std::string &_name);

      public: virtual void SetFormat(PixelFormat _format);

      public: virtual gazebo::common::Color GetBackgroundColor() const;

      public: virtual void SetBackgroundColor(gazebo::common::Color _color);

      public: virtual RenderTexturePtr Build() const;

      protected: virtual OgreRenderTexturePtr BuildSafe() const;

      protected: OgreScenePtr scene;

      protected: Ogre::Camera *ogreCamera;

      protected: std::string name;

      protected: gazebo::common::Color backgroundColor;
    };
  }
}
#endif
