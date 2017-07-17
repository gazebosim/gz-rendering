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
#ifndef IGNITION_RENDERING_BASE_BASECAMERA_HH_
#define IGNITION_RENDERING_BASE_BASECAMERA_HH_

#include <string>
#include <ignition/common/Event.hh>

#include "ignition/rendering/Camera.hh"
#include "ignition/rendering/Image.hh"
#include "ignition/rendering/Scene.hh"
#include "ignition/rendering/base/BaseRenderTarget.hh"

namespace ignition
{
  namespace rendering
  {
    template <class T>
    class IGNITION_VISIBLE BaseCamera :
      public virtual Camera,
      public virtual T
    {
      protected: BaseCamera();

      public: virtual ~BaseCamera();

      public: virtual unsigned int ImageWidth() const;

      public: virtual void SetImageWidth(unsigned int _width);

      public: virtual unsigned int ImageHeight() const;

      public: virtual void SetImageHeight(unsigned int _height);

      public: virtual PixelFormat ImageFormat() const = 0;

      public: virtual unsigned int ImageDepth() const;

      public: virtual unsigned int ImageMemorySize() const;

      public: virtual void SetHFOV(const math::Angle &_angle) = 0;

      public: virtual void SetAspectRatio(double _ratio) = 0;

      public: virtual void PreRender();

      public: virtual void PostRender();

      public: virtual void Update();

      public: virtual Image CreateImage() const;

      public: virtual void Capture(Image &_image);

      public: virtual void Copy(Image &_image) const;

      public: virtual bool SaveFrame(const std::string &_name);

      public: virtual common::ConnectionPtr ConnectNewImageFrame(
                  Camera::NewFrameListener _listener);

      public: virtual RenderWindowPtr CreateRenderWindow();

      protected: virtual void *CreateImageBuffer() const;

      protected: virtual void Load();

      protected: virtual void Reset();

      protected: virtual RenderTargetPtr RenderTarget() const = 0;

      protected: common::EventT<void(const void *, unsigned int, unsigned int,
                     unsigned int, const std::string &)> newFrameEvent;

      protected: ImagePtr imageBuffer;
    };

    //////////////////////////////////////////////////
    template <class T>
    BaseCamera<T>::BaseCamera()
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    BaseCamera<T>::~BaseCamera()
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    unsigned int BaseCamera<T>::ImageWidth() const
    {
      return this->RenderTarget()->Width();
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseCamera<T>::SetImageWidth(unsigned int _width)
    {
      this->RenderTarget()->SetWidth(_width);
    }

    //////////////////////////////////////////////////
    template <class T>
    unsigned int BaseCamera<T>::ImageHeight() const
    {
      return this->RenderTarget()->Height();
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseCamera<T>::SetImageHeight(unsigned int _height)
    {
      this->RenderTarget()->SetHeight(_height);
    }

    //////////////////////////////////////////////////
    template <class T>
    unsigned int BaseCamera<T>::ImageDepth() const
    {
      return PixelUtil::ChannelCount(this->ImageFormat());
    }

    //////////////////////////////////////////////////
    template <class T>
    unsigned int BaseCamera<T>::ImageMemorySize() const
    {
      PixelFormat format = this->ImageFormat();
      unsigned int width = this->ImageWidth();
      unsigned int height = this->ImageHeight();
      return PixelUtil::MemorySize(format, width, height);
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseCamera<T>::PreRender()
    {
      T::PreRender();
      this->RenderTarget()->PreRender();
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseCamera<T>::PostRender()
    {
      // do nothing by default
    }

    //////////////////////////////////////////////////
    template <class T>
    Image BaseCamera<T>::CreateImage() const
    {
      PixelFormat format = this->ImageFormat();
      unsigned int width = this->ImageWidth();
      unsigned int height = this->ImageHeight();
      return Image(width, height, format);
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseCamera<T>::Update()
    {
      this->Scene()->PreRender();
      this->Render();
      this->PostRender();
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseCamera<T>::Capture(Image &_image)
    {
      this->Scene()->PreRender();
      this->Render();
      this->PostRender();
      this->Copy(_image);
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseCamera<T>::Copy(Image &_image) const
    {
      this->RenderTarget()->Copy(_image);
    }

    //////////////////////////////////////////////////
    template <class T>
    bool BaseCamera<T>::SaveFrame(const std::string &/*_name*/)
    {
      return false;
    }

    //////////////////////////////////////////////////
    template <class T>
    common::ConnectionPtr BaseCamera<T>::ConnectNewImageFrame(
        Camera::NewFrameListener _listener)
    {
      return newFrameEvent.Connect(_listener);
    }

    //////////////////////////////////////////////////
    template <class T>
    void *BaseCamera<T>::CreateImageBuffer() const
    {
      // TODO: determine proper type
      unsigned int size = this->ImageMemorySize();
      return new unsigned char *[size];
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseCamera<T>::Load()
    {
      T::Load();
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseCamera<T>::Reset()
    {
      math::Angle fov;
      fov.Degree(80);
      this->SetImageWidth(1);
      this->SetImageHeight(1);
      this->SetImageFormat(PF_R8G8B8);
      this->SetAspectRatio(1);
      this->SetAntiAliasing(0);
      this->SetHFOV(fov);
    }

    //////////////////////////////////////////////////
    template <class T>
    RenderWindowPtr BaseCamera<T>::CreateRenderWindow()
    {
      // Does nothing by default
      std::cerr << "Create empty render window " << std::endl;
      return RenderWindowPtr();
    }

  }
}
#endif
