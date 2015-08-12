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
#ifndef _IGNITION_RENDERING_OPTIXRENDERTARGET_HH_
#define _IGNITION_RENDERING_OPTIXRENDERTARGET_HH_

#include "gazebo/common/Color.hh"
#include "ignition/rendering/base/BaseRenderTypes.hh"
#include "ignition/rendering/base/BaseRenderTarget.hh"
#include "ignition/rendering/optix/OptixIncludes.hh"
#include "ignition/rendering/optix/OptixRenderTypes.hh"

namespace ignition
{
  namespace rendering
  {
    class IGNITION_VISIBLE OptixRenderTarget :
      public virtual BaseRenderTarget
    {
      protected: OptixRenderTarget();

      public: virtual ~OptixRenderTarget();

      public: virtual unsigned int GetWidth() const;

      public: virtual unsigned int GetHeight() const;

      public: virtual void Update();

      public: virtual void Destroy() = 0;

      protected: virtual void Initialize();

      protected: OptixScenePtr scene;

      protected: unsigned int entryId;

      protected: unsigned int width;

      protected: unsigned int height;

      protected: optix::Program optixRenderProgram;

      protected: optix::Buffer optixImageBuffer;
    };

    class IGNITION_VISIBLE OptixRenderTexture :
      public virtual OptixRenderTarget,
      public virtual BaseRenderTexture
    {
      protected: OptixRenderTexture();

      public: virtual ~OptixRenderTexture();

      public: virtual void GetData(void *data) const;

      public: virtual void Destroy();

      private: friend class OptixRenderTextureBuilder;
    };

    class IGNITION_VISIBLE OptixRenderTextureBuilder :
      public virtual BaseRenderTextureBuilder
    {
      public: OptixRenderTextureBuilder(OptixScenePtr _scene);

      public: virtual ~OptixRenderTextureBuilder();

      public: virtual std::string GetProgramName() const;

      public: virtual void SetProgramName(const std::string &_name);

      public: virtual std::string GetFunctionName() const;

      public: virtual void SetFunctionName(const std::string &_name);

      public: virtual void SetFormat(PixelFormat _format);

      public: virtual gazebo::common::Color GetBackgroundColor() const;

      public: virtual void SetBackgroundColor(gazebo::common::Color _color);

      public: virtual BaseRenderTexturePtr Build() const;

      protected: virtual OptixRenderTexturePtr BuildSafe() const;

      protected: OptixScenePtr scene;

      protected: std::string name;

      protected: gazebo::common::Color backgroundColor;
    };
  }
}
#endif
