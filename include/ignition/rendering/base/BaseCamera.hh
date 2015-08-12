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

#include "gazebo/common/Console.hh"
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

      public: virtual void Load(sdf::ElementPtr _sdf);

      public: virtual unsigned int GetImageWidth() const;

      public: virtual void SetImageWidth(unsigned int _width);

      public: virtual unsigned int GetImageHeight() const;

      public: virtual void SetImageHeight(unsigned int _height);

      public: virtual void SetImageSize(unsigned int _width,
                  unsigned int _height);

      public: virtual PixelFormat GetImageFormat() const;

      public: virtual void SetImageFormat(PixelFormat _format);

      public: virtual unsigned int GetImageDepth() const;

      public: virtual unsigned int GetImageMemorySize() const;

      public: virtual void SetHFOV(const gazebo::math::Angle &_angle) = 0;

      public: virtual void SetAspectRatio(double _ratio) = 0;

      public: virtual unsigned int GetAntiAliasing() const;

      public: virtual void SetAntiAliasing(unsigned int _aa);

      public: virtual void PreRender();

      public: virtual void PostRender();

      public: virtual Image CreateImage() const;

      public: virtual void Capture(Image &_image);

      public: virtual void GetImageData(Image &_image) const;

      public: virtual bool SaveFrame(const std::string &_name);

      public: virtual gazebo::event::ConnectionPtr ConnectNewImageFrame(
                  Camera::NewFrameListener _listener);

      public: virtual void DisconnectNewImageFrame(gazebo::event::ConnectionPtr &_conn);

      protected: virtual void *CreateImageBuffer() const;

      protected: virtual void Reset();

      protected: virtual RenderTexturePtr GetRenderTexture() const = 0;

      protected: virtual BaseRenderTextureBuilderPtr
                     GetTextureBuilder() const = 0;

      protected: RenderTexturePtr renderTexture;

      protected: gazebo::event::EventT<void(const void *, unsigned int, unsigned int,
                     unsigned int, const std::string &)> newFrameEvent;
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
    void BaseCamera<T>::Load(sdf::ElementPtr /*_sdf*/)
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    unsigned int BaseCamera<T>::GetImageWidth() const
    {
      return this->GetTextureBuilder()->GetWidth();
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseCamera<T>::SetImageWidth(unsigned int _width)
    {
      this->GetTextureBuilder()->SetWidth(_width);
    }

    //////////////////////////////////////////////////
    template <class T>
    unsigned int BaseCamera<T>::GetImageHeight() const
    {
      return this->GetTextureBuilder()->GetHeight();
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseCamera<T>::SetImageHeight(unsigned int _height)
    {
      this->GetTextureBuilder()->SetHeight(_height);
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseCamera<T>::SetImageSize(unsigned int _width, unsigned int _height)
    {
      this->GetTextureBuilder()->SetSize(_width, _height);
    }

    //////////////////////////////////////////////////
    template <class T>
    PixelFormat BaseCamera<T>::GetImageFormat() const
    {
      return this->GetTextureBuilder()->GetFormat();
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseCamera<T>::SetImageFormat(PixelFormat _format)
    {
      this->GetTextureBuilder()->SetFormat(_format);
    }

    //////////////////////////////////////////////////
    template <class T>
    unsigned int BaseCamera<T>::GetImageDepth() const
    {
      return this->GetTextureBuilder()->GetDepth();
    }

    //////////////////////////////////////////////////
    template <class T>
    unsigned int BaseCamera<T>::GetImageMemorySize() const
    {
      return this->GetTextureBuilder()->GetMemorySize();
    }

    //////////////////////////////////////////////////
    template <class T>
    unsigned int BaseCamera<T>::GetAntiAliasing() const
    {
      return this->GetTextureBuilder()->GetAntiAliasing();
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseCamera<T>::SetAntiAliasing(unsigned int _aa)
    {
      this->GetTextureBuilder()->SetAntiAliasing(_aa);
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseCamera<T>::PreRender()
    {
      if (!this->renderTexture)
      {
        this->renderTexture = this->GetTextureBuilder()->Build();
      }
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
      unsigned int width = this->GetImageWidth();
      unsigned int height = this->GetImageHeight();
      PixelFormat format = this->GetImageFormat();
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
      this->renderTexture->GetImage(_image);
    }

    //////////////////////////////////////////////////
    template <class T>
    bool BaseCamera<T>::SaveFrame(const std::string &/*_name*/)
    {
      return false;
    }

    //////////////////////////////////////////////////
    template <class T>
    gazebo::event::ConnectionPtr BaseCamera<T>::ConnectNewImageFrame(
        Camera::NewFrameListener _listener)
    {
      return newFrameEvent.Connect(_listener);
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseCamera<T>::DisconnectNewImageFrame(gazebo::event::ConnectionPtr &_conn)
    {
      newFrameEvent.Disconnect(_conn);
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
    void BaseCamera<T>::Reset()
    {
      gazebo::math::Angle fov;
      fov.SetFromDegree(80);
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
