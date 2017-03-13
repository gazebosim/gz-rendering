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
#ifndef _IGNITION_RENDERING_OPTIXRENDERENGINE_HH_
#define _IGNITION_RENDERING_OPTIXRENDERENGINE_HH_

#include <ignition/common/SingletonT.hh>
#include "ignition/rendering/base/BaseRenderEngine.hh"
#include "ignition/rendering/optix/OptixRenderTypes.hh"

namespace ignition
{
  namespace rendering
  {
    class IGNITION_VISIBLE OptixRenderEngine :
      public virtual BaseRenderEngine,
      public common::SingletonT<OptixRenderEngine>
    {
      private: OptixRenderEngine();

      public: virtual ~OptixRenderEngine();

      public: virtual bool Fini();

      public: virtual std::string GetName() const;

      public: std::string GetPtxFile(const std::string& _fileBase) const;

      protected: virtual ScenePtr CreateSceneImpl(unsigned int _id,
                  const std::string &_name);

      protected: virtual SceneStorePtr GetScenes() const;

      protected: virtual bool LoadImpl();

      protected: virtual bool InitImpl();

      private: OptixSceneStorePtr scenes;

      private: static const std::string PTX_PREFIX;

      private: static const std::string PTX_SUFFIX;

      private: friend class SingletonT<OptixRenderEngine>;
    };
  }
}
#endif
