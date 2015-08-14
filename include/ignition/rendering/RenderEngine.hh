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
#ifndef _IGNITION_RENDERING_RENDERENGINE_HH_
#define _IGNITION_RENDERING_RENDERENGINE_HH_

#include <string>
#include "gazebo/common/Console.hh"
#include "ignition/rendering/RenderTypes.hh"
#include "ignition/rendering/Util.hh"

namespace ignition
{
  namespace rendering
  {
    class IGNITION_VISIBLE RenderEngine
    {
      public: virtual ~RenderEngine() { }

      public: virtual bool Load() = 0;

      public: virtual bool Init() = 0;

      public: virtual bool Fini() = 0;

      public: virtual bool IsLoaded() const = 0;

      public: virtual bool IsInitialized() const = 0;

      public: virtual bool IsEnabled() const = 0;

      public: virtual std::string GetName() const = 0;

      public: virtual unsigned int GetSceneCount() const = 0;

      public: virtual bool HasScene(ConstScenePtr _scene) const = 0;

      public: virtual bool HasSceneId(unsigned int _id) const = 0;

      public: virtual bool HasSceneName(const std::string &_name) const = 0;

      public: virtual ScenePtr GetSceneById(unsigned int _id) const = 0;

      public: virtual ScenePtr GetSceneByName(
                  const std::string &_name) const = 0;

      public: virtual ScenePtr GetSceneByIndex(unsigned int _index) const = 0;

      public: virtual void DestroyScene(ScenePtr _scene) = 0;

      public: virtual void DestroySceneById(unsigned int _id) = 0;

      public: virtual void DestroySceneByName(const std::string &_name) = 0;

      public: virtual void DestroySceneByIndex(unsigned int _index) = 0;

      public: virtual void DestroyScenes() = 0;

      public: virtual ScenePtr CreateScene(const std::string &_name) = 0;

      public: virtual ScenePtr CreateScene(unsigned int _id,
                  const std::string &_name) = 0;

      public: virtual void Destroy() = 0;
    };
  }
}
#endif
