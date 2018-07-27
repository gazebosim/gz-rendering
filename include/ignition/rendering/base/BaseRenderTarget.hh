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
#ifndef IGNITION_RENDERING_BASE_BASERENDERTARGET_HH_
#define IGNITION_RENDERING_BASE_BASERENDERTARGET_HH_

#include <string>

#include "ignition/rendering/RenderTarget.hh"
#include "ignition/rendering/Scene.hh"
#include "ignition/rendering/base/BaseRenderTypes.hh"

namespace ignition
{
  namespace rendering
  {
    template <class T>
    class BaseRenderTarget :
      public virtual RenderTarget,
      public virtual T
    {
      public: BaseRenderTarget();

      public: virtual ~BaseRenderTarget();

      // Documentation inherited.
      public: virtual void PreRender();

      // Documentation inherited.
      public: virtual void PostRender();

      public: virtual unsigned int Width() const;

      public: virtual void SetWidth(const unsigned int _width);

      public: virtual unsigned int Height() const;

      public: virtual void SetHeight(const unsigned int _height);

      public: virtual PixelFormat Format() const;

      public: virtual void SetFormat(PixelFormat _format);

      // Documentation inherited
      public: virtual math::Color BackgroundColor() const;

      protected: virtual void Rebuild();

      protected: virtual void RebuildImpl() = 0;

      protected: PixelFormat format = PF_UNKNOWN;

      protected: bool targetDirty = true;

      protected: unsigned int width = 0u;

      protected: unsigned int height = 0u;
    };

    template <class T>
    class BaseRenderTexture :
      public virtual RenderTexture,
      public virtual T
    {
      public: BaseRenderTexture();

      public: virtual ~BaseRenderTexture();

      // Documentation inherited.
      public: virtual GLuint GLId() const;
    };

    template <class T>
    class BaseRenderWindow :
      public virtual RenderWindow,
      public virtual T
    {
      public: BaseRenderWindow();

      public: virtual ~BaseRenderWindow();

      public: virtual std::string Handle() const;

      public: virtual void SetHandle(const std::string &_handle);

      public: virtual double DevicePixelRatio() const;

      public: virtual void SetDevicePixelRatio(const double _ratio);

      public: virtual void OnResize(const unsigned int _width,
                  const unsigned int _height);

      public: virtual void OnMove();

      protected: std::string handle;

      protected: double ratio = 1.0;
    };

    //////////////////////////////////////////////////
    // BaseRenderTarget
    //////////////////////////////////////////////////
    template <class T>
    BaseRenderTarget<T>::BaseRenderTarget()
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
    void BaseRenderTarget<T>::PostRender()
    {
      T::PostRender();
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
    template <class T>
    unsigned int BaseRenderTarget<T>::Width() const
    {
      return this->width;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseRenderTarget<T>::SetWidth(const unsigned int _width)
    {
      this->width = _width;
      this->targetDirty = true;
    }

    //////////////////////////////////////////////////
    template <class T>
    unsigned int BaseRenderTarget<T>::Height() const
    {
      return this->height;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseRenderTarget<T>::SetHeight(const unsigned int _height)
    {
      this->height = _height;
      this->targetDirty = true;
    }

    //////////////////////////////////////////////////
    template <class T>
    PixelFormat BaseRenderTarget<T>::Format() const
    {
      return this->format;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseRenderTarget<T>::SetFormat(PixelFormat _format)
    {
      this->format = PixelUtil::Sanitize(_format);
      this->targetDirty = true;
    }

    //////////////////////////////////////////////////
    template <class T>
    math::Color BaseRenderTarget<T>::BackgroundColor() const
    {
      return this->Scene()->BackgroundColor();
    }


    //////////////////////////////////////////////////
    // BaseRenderTexture
    //////////////////////////////////////////////////
    template <class T>
    BaseRenderTexture<T>::BaseRenderTexture()
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    BaseRenderTexture<T>::~BaseRenderTexture()
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    GLuint BaseRenderTexture<T>::GLId() const
    {
      return GLuint(0u);
    }


    //////////////////////////////////////////////////
    // BaseRenderWindow
    //////////////////////////////////////////////////
    template <class T>
    BaseRenderWindow<T>::BaseRenderWindow()
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    BaseRenderWindow<T>::~BaseRenderWindow()
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    std::string BaseRenderWindow<T>::Handle() const
    {
      return this->handle;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseRenderWindow<T>::SetHandle(const std::string &_handle)
    {
      this->handle = _handle;
      this->targetDirty = true;
    }

    //////////////////////////////////////////////////
    template <class T>
    double BaseRenderWindow<T>::DevicePixelRatio() const
    {
      return this->ratio;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseRenderWindow<T>::SetDevicePixelRatio(const double _ratio)
    {
      this->ratio = _ratio;
      this->targetDirty = true;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseRenderWindow<T>::OnResize(const unsigned int _width,
        const unsigned int _height)
    {
      this->width = _width;
      this->height = _height;
      this->targetDirty = true;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseRenderWindow<T>::OnMove()
    {
      this->targetDirty = true;
    }
  }
}
#endif
