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
#ifndef _IGNITION_RENDERING_BASECAMERA_HH_
#define _IGNITION_RENDERING_BASECAMERA_HH_

#include <ignition/common/Event.hh>

#include "ignition/rendering/Camera.hh"
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

      public: virtual unsigned int GetImageWidth() const;

      public: virtual void SetImageWidth(unsigned int _width);

      public: virtual unsigned int GetImageHeight() const;

      public: virtual void SetImageHeight(unsigned int _height);

      public: virtual PixelFormat GetImageFormat() const = 0;

      public: virtual unsigned int GetImageDepth() const;

      public: virtual unsigned int GetImageMemorySize() const;

      public: virtual void SetHFOV(const math::Angle &_angle) = 0;

      public: virtual void SetAspectRatio(double _ratio) = 0;

      public: virtual void PreRender();

      public: virtual void PostRender();

      public: virtual Image CreateImage() const;

      public: virtual void Capture(Image &_image);

      public: virtual void GetImageData(Image &_image) const;

      public: virtual bool SaveFrame(const std::string &_name);

      public: virtual common::ConnectionPtr ConnectNewImageFrame(
                  Camera::NewFrameListener _listener);

      protected: virtual void *CreateImageBuffer() const;

      protected: virtual void Load();

      protected: virtual void Reset();

      protected: virtual RenderTexturePtr GetRenderTexture() const = 0;

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
    unsigned int BaseCamera<T>::GetImageWidth() const
    {
      return this->GetRenderTexture()->GetWidth();
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseCamera<T>::SetImageWidth(unsigned int _width)
    {
      this->GetRenderTexture()->SetWidth(_width);
    }

    //////////////////////////////////////////////////
    template <class T>
    unsigned int BaseCamera<T>::GetImageHeight() const
    {
      return this->GetRenderTexture()->GetHeight();
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseCamera<T>::SetImageHeight(unsigned int _height)
    {
      this->GetRenderTexture()->SetHeight(_height);
    }

    //////////////////////////////////////////////////
    template <class T>
    unsigned int BaseCamera<T>::GetImageDepth() const
    {
      return PixelUtil::GetChannelCount(this->GetImageFormat());
    }

    //////////////////////////////////////////////////
    template <class T>
    unsigned int BaseCamera<T>::GetImageMemorySize() const
    {
      PixelFormat format = this->GetImageFormat();
      unsigned int width = this->GetImageWidth();
      unsigned int height = this->GetImageHeight();
      return PixelUtil::GetMemorySize(format, width, height);
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseCamera<T>::PreRender()
    {
      T::PreRender();
      this->GetRenderTexture()->PreRender();
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
      PixelFormat format = this->GetImageFormat();
      unsigned int width = this->GetImageWidth();
      unsigned int height = this->GetImageHeight();
      return Image(width, height, format);
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseCamera<T>::Capture(Image &_image)
    {
      this->GetScene()->PreRender();
      this->Render();
      this->PostRender();
      this->GetImageData(_image);
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseCamera<T>::GetImageData(Image &_image) const
    {
      this->GetRenderTexture()->GetImage(_image);
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
      unsigned int size = this->GetImageMemorySize();
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
  }
}
#endif
