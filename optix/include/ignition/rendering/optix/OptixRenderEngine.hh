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
#ifndef IGNITION_RENDERING_OPTIX_OPTIXRENDERENGINE_HH_
#define IGNITION_RENDERING_OPTIX_OPTIXRENDERENGINE_HH_

#include <string>
#include <ignition/common/SingletonT.hh>
#include "ignition/rendering/base/BaseRenderEngine.hh"
#include "ignition/rendering/optix/OptixRenderTypes.hh"

namespace ignition
{
  namespace rendering
  {
    class IGNITION_RENDERING_OPTIX_VISIBLE OptixRenderEngine :
      public virtual BaseRenderEngine,
      public common::SingletonT<OptixRenderEngine>
    {
      /// \brief Constructor
      /// Note: This needs to be public in order for IGN_REGISTER_RENDER_ENGINE
      /// to work.
      public: OptixRenderEngine();
      // private: OptixRenderEngine();

      public: virtual ~OptixRenderEngine();

      public: virtual bool Fini();

      public: virtual std::string Name() const;

      public: std::string PtxFile(const std::string& _fileBase) const;

      protected: virtual ScenePtr CreateSceneImpl(unsigned int _id,
                  const std::string &_name);

      protected: virtual SceneStorePtr Scenes() const;

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
