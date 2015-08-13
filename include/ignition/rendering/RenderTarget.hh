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
#ifndef _IGNITION_RENDERING_RENDERTARGET_HH_
#define _IGNITION_RENDERING_RENDERTARGET_HH_

#include "ignition/rendering/RenderTypes.hh"
#include "ignition/rendering/Image.hh"
#include "ignition/rendering/Object.hh"

namespace ignition
{
  namespace rendering
  {
    class IGNITION_VISIBLE RenderTarget :
      public virtual Object
    {
      public: virtual ~RenderTarget() { }

      public: virtual unsigned int GetWidth() const = 0;

      public: virtual unsigned int GetHeight() const = 0;

      public: virtual void GetImage(Image &_image) const = 0;
    };

    class IGNITION_VISIBLE RenderTexture :
      public virtual RenderTarget
    {
      public: virtual ~RenderTexture() { }

      public: virtual void SetWidth(unsigned int _width) = 0;

      public: virtual void SetHeight(unsigned int _height) = 0;
    };

    class IGNITION_VISIBLE RenderWindow :
      public virtual RenderTarget
    {
      public: virtual ~RenderWindow() { }

      public: virtual void OnResize(unsigned int _width,
                  unsigned int _height) = 0;

      public: virtual void OnMove() = 0;
    };
  }
}
#endif
