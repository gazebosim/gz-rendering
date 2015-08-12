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
#ifndef _IGNITION_RENDERING_BASERENDERTARGET_HH_
#define _IGNITION_RENDERING_BASERENDERTARGET_HH_

#include "ignition/rendering/RenderTarget.hh"
#include "ignition/rendering/base/BaseRenderTypes.hh"

namespace ignition
{
  namespace rendering
  {
    template <class T>
    class IGNITION_VISIBLE BaseRenderTarget :
      public virtual RenderTarget,
      public virtual T
    {
      public: BaseRenderTarget();

      public: virtual ~BaseRenderTarget();

      public: virtual void PreRender();

      protected: virtual void Rebuild();

      protected: virtual void RebuildImpl() = 0;

      protected: bool targetDirty;
    };

    template <class T>
    class IGNITION_VISIBLE BaseRenderTexture :
      public virtual RenderTexture,
      public virtual T
    {
      public: BaseRenderTexture();

      public: virtual ~BaseRenderTexture();

      public: virtual unsigned int GetWidth() const;

      public: virtual void SetWidth(unsigned int _width);

      public: virtual unsigned int GetHeight() const;

      public: virtual void SetHeight(unsigned int _height);

      public: virtual PixelFormat GetFormat() const;

      public: virtual void SetFormat(PixelFormat _format);

      protected: unsigned int width;

      protected: unsigned int height;

      protected: PixelFormat format;
    };

    class IGNITION_VISIBLE BaseRenderTextureBuilder
    {
      public: BaseRenderTextureBuilder();

      public: virtual ~BaseRenderTextureBuilder();

      public: virtual unsigned int GetWidth() const;

      public: virtual void SetWidth(unsigned int _width);

      public: virtual unsigned int GetHeight() const;

      public: virtual void SetHeight(unsigned int _height);

      public: virtual void SetSize(unsigned int _width, unsigned int _height);

      public: virtual PixelFormat GetFormat() const;

      public: virtual void SetFormat(PixelFormat _format);

      public: virtual unsigned int GetDepth() const;

      public: virtual unsigned int GetMemorySize() const;

      public: virtual unsigned int GetAntiAliasing() const;

      public: virtual void SetAntiAliasing(unsigned int _aa);

      public: virtual RenderTexturePtr Build() const = 0;

      protected: unsigned int width;

      protected: unsigned int height;

      protected: PixelFormat format;

      protected: unsigned int aa;
    };

    //////////////////////////////////////////////////
    // BaseRenderTarget
    //////////////////////////////////////////////////
    template <class T>
    BaseRenderTarget<T>::BaseRenderTarget() :
      targetDirty(true)
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    BaseRenderTarget<T>::~BaseRenderTarget()
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseRenderTarget<T>::PreRender()
    {
      T::PreRender();
      this->Rebuild();
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseRenderTarget<T>::Rebuild()
    {
      if (this->targetDirty)
      {
        this->RebuildImpl();
        this->targetDirty = false;
      }
    }

    //////////////////////////////////////////////////
    // BaseRenderTexture
    //////////////////////////////////////////////////
    template <class T>
    BaseRenderTexture<T>::BaseRenderTexture() :
      width(0),
      height(0),
      format(PF_UNKNOWN)
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    BaseRenderTexture<T>::~BaseRenderTexture()
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    unsigned int BaseRenderTexture<T>::GetWidth() const
    {
      return this->width;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseRenderTexture<T>::SetWidth(unsigned int _width)
    {
      this->width = _width;
      this->targetDirty = true;
    }

    //////////////////////////////////////////////////
    template <class T>
    unsigned int BaseRenderTexture<T>::GetHeight() const
    {
      return this->height;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseRenderTexture<T>::SetHeight(unsigned int _height)
    {
      this->height = _height;
      this->targetDirty = true;
    }

    //////////////////////////////////////////////////
    template <class T>
    PixelFormat BaseRenderTexture<T>::GetFormat() const
    {
      return this->format;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseRenderTexture<T>::SetFormat(PixelFormat _format)
    {
      this->format = PixelUtil::Sanitize(_format);
      this->targetDirty = true;
    }
  }
}
#endif
