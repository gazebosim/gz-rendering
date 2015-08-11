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

#include "ignition/rendering/PixelFormat.hh"
#include "ignition/rendering/base/BaseRenderTypes.hh"
#include "ignition/rendering/Util.hh"

namespace ignition
{
  namespace rendering
  {
    class IGNITION_VISIBLE BaseRenderTarget
    {
      public: BaseRenderTarget();

      public: virtual ~BaseRenderTarget();

      public: virtual unsigned int GetWidth() const = 0;

      public: virtual unsigned int GetHeight() const = 0;

      public: virtual void Update() = 0;

      public: virtual void Destroy() = 0;
    };

    class IGNITION_VISIBLE BaseRenderTexture :
      public virtual BaseRenderTarget
    {
      public: BaseRenderTexture();

      public: virtual ~BaseRenderTexture();

      public: virtual void GetData(void *data) const = 0;
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

      public: virtual BaseRenderTexturePtr Build() const = 0;

      protected: unsigned int width;

      protected: unsigned int height;

      protected: PixelFormat format;

      protected: unsigned int aa;
    };
  }
}
#endif
