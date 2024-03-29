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
#ifndef GZ_RENDERING_OPTIX_OPTIXTEXTUREFACTORY_HH_
#define GZ_RENDERING_OPTIX_OPTIXTEXTUREFACTORY_HH_

#include <string>
#include "gz/rendering/optix/OptixRenderTypes.hh"
#include "gz/rendering/optix/OptixIncludes.hh"
#include "gz/rendering/optix/Export.hh"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    //
    class GZ_RENDERING_OPTIX_VISIBLE OptixTextureFactory
    {
      public: explicit OptixTextureFactory(OptixScenePtr _scene);

      public: virtual ~OptixTextureFactory();

      public: optix::TextureSampler Create(const std::string &_filename);

      public: optix::TextureSampler Create();

      protected: optix::Buffer CreateBuffer(const std::string &_filename);

      protected: optix::Buffer CreateBuffer();

      protected: optix::TextureSampler CreateSampler(optix::Buffer _buffer);

      protected: OptixScenePtr scene;
    };
    }
  }
}
#endif
