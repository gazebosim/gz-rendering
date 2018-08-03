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
#ifndef IGNITION_RENDERING_OGRE2_OGRE2RENDERTARGET_HH_
#define IGNITION_RENDERING_OGRE2_OGRE2RENDERTARGET_HH_

#include <ignition/math/Color.hh>

#include "ignition/rendering/base/BaseRenderTypes.hh"
#include "ignition/rendering/base/BaseRenderTarget.hh"
#include "ignition/rendering/ogre2/Ogre2Includes.hh"
#include "ignition/rendering/ogre2/Ogre2Object.hh"
#include "ignition/rendering/ogre2/Ogre2RenderTargetMaterial.hh"

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
    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
    //
    class IGNITION_RENDERING_OGRE2_VISIBLE Ogre2RenderTarget :
      public virtual BaseRenderTarget<Ogre2Object>
    {
      protected: Ogre2RenderTarget();

      public: virtual ~Ogre2RenderTarget();

      public: virtual unsigned int AntiAliasing() const;

      public: virtual void SetAntiAliasing(unsigned int _aa);

      public: virtual void Copy(Image &_image) const;

      public: virtual Ogre::Camera *Camera() const;

      public: virtual void SetCamera(Ogre::Camera *_camera);

      // Documentation inherited
      public: virtual math::Color BackgroundColor() const;

      public: virtual void SetBackgroundColor(math::Color _color);

      public: virtual void PreRender();

      public: virtual void Render();

      public: virtual void Destroy() = 0;

      /// \brief Set a material to render on every object. This method is used
      /// for special cases like the render target of a depth camera.
      /// \param[in] _material The material to render
      public: void SetMaterial(MaterialPtr _material);

      protected: virtual Ogre::RenderTarget *RenderTarget() const = 0;

      protected: virtual void UpdateBackgroundColor();

      protected: virtual void RebuildImpl();

      protected: virtual void RebuildTarget() = 0;

      protected: virtual void RebuildCompositor();

      protected: virtual void RebuildViewport();

      protected: virtual void BuildCompositor();

      protected: virtual void DestroyCompositor();

      /// \brief Re-initializes render target material to apply a material to
      /// everything in the scene. Does nothing if no material has been set
      /// \sa Ogre2RenderTarget::RebuildImpl()
      /// \sa BaseRenderTarget::Rebuild()
      protected: void RebuildMaterial();

      protected: Ogre::Camera *ogreCamera = nullptr;

      // protected: Ogre::Viewport *ogreViewport = nullptr;

      protected: Ogre::CompositorWorkspace *ogreCompositorWorkspace = nullptr;

      protected: Ogre::ColourValue ogreBackgroundColor;

      /// \brief a material used by for the render target
      protected: MaterialPtr material;

      /// \brief Helper class that applies the material to the render target
      protected: Ogre2RenderTargetMaterialPtr materialApplicator;

      protected: bool colorDirty = true;

      protected: unsigned int antiAliasing = 4;
    };

    class IGNITION_RENDERING_OGRE2_VISIBLE Ogre2RenderTexture :
      public virtual BaseRenderTexture<Ogre2RenderTarget>
    {
      protected: Ogre2RenderTexture();

      public: virtual ~Ogre2RenderTexture();

      public: virtual void Destroy();

      protected: virtual Ogre::RenderTarget *RenderTarget() const;

      protected: virtual void RebuildTarget();

      protected: virtual void DestroyTarget();

      protected: virtual void BuildTarget();

      protected: Ogre::Texture *ogreTexture = nullptr;

      private: friend class Ogre2Scene;
    };

    class IGNITION_RENDERING_OGRE2_VISIBLE Ogre2RenderWindow :
      public virtual BaseRenderWindow<Ogre2RenderTarget>
    {
      protected: Ogre2RenderWindow();

      public: virtual ~Ogre2RenderWindow();

      public: virtual void Destroy();

      protected: virtual Ogre::RenderTarget *RenderTarget() const;

      protected: virtual void RebuildTarget();

      protected: virtual void BuildTarget();

      protected: Ogre::RenderTarget *ogreRenderWindow = nullptr;

      private: friend class Ogre2Scene;
    };
    }
  }
}
#endif
