/*
 * Copyright (C) 2015 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License") = 0;
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
#ifndef IGNITION_RENDERING_RENDERTARGET_HH_
#define IGNITION_RENDERING_RENDERTARGET_HH_

#include <string>

#include "ignition/rendering/RenderTypes.hh"
#include "ignition/rendering/Image.hh"
#include "ignition/rendering/Object.hh"

namespace ignition
{
  namespace rendering
  {
    /// \class RenderTarget RenderTarget.hh ignition/rendering/RenderTarget.hh
    /// \brief Represents a render-target to which cameras can render images.
    class IGNITION_VISIBLE RenderTarget :
      public virtual Object
    {
      /// \brief Deconstructor
      public: virtual ~RenderTarget() { }

      /// \brief Get image width in pixels
      /// \return The image width in pixels
      public: virtual unsigned int Width() const = 0;

      /// \brief Get image height in pixels
      /// \return The image height in pixels
      public: virtual unsigned int Height() const = 0;

      /// \brief Set the texture width in pixels
      /// \param[in] _width New texture width in pixels
      public: virtual void SetWidth(unsigned int _width) = 0;

      /// \brief Set the texture height in pixels
      /// \param[in] _height New texture height in pixels
      public: virtual void SetHeight(unsigned int _height) = 0;

      /// \brief Write rendered image to given Image. The RenderTarget will
      /// convert the underlying image to the specified format listed in the
      /// given Image. However if the given image is not of the correct size no
      /// work will be done. Calling this function before an image has been
      /// rendered will result in undefined behavior.
      /// \param[out] _image Image to which output will be written
      public: virtual void Copy(Image &_image) const = 0;
    };

    /// \class RenderTexture RenderTexture.hh
    /// ignition/rendering/RenderTexture.hh
    /// \brief Represents a off-screen render-texture to which cameras can
    /// render images.
    class IGNITION_VISIBLE RenderTexture :
      public virtual RenderTarget
    {
      /// \brief Deconstructor
      public: virtual ~RenderTexture() { }
    };

    /// \class RenderWindow RenderWindow.hh
    /// ignition/rendering/RenderWindow.hh
    /// \brief Represents a on-screen render-window to which cameras can
    /// render images.
    class IGNITION_VISIBLE RenderWindow :
      public virtual RenderTarget
    {
      /// \brief Deconstructor
      public: virtual ~RenderWindow() { }

      /// \brief Set the window handle to attach the render window to
      /// \param[in] _handle Window handle
      public: virtual void SetHandle(const std::string &_handle) = 0;

      /// \brief Set the device to pixel ratio
      /// \param[in] _ratio Device to pixel ratio
      public: virtual void SetRatio(const double _ratio) = 0;

      /// \brief Alert the window of a window resize event
      /// \param[in] _width New window width in pixels
      /// \param[in] _height New window height in pixels
      public: virtual void OnResize(unsigned int _width,
                  unsigned int _height) = 0;

      /// \brief Alert the window of a window move event
      public: virtual void OnMove() = 0;
    };
  }
}
#endif
