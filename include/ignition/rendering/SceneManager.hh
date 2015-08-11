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
#ifndef _IGNITION_RENDERING_SCENEMANAGER_HH 
#define _IGNITION_RENDERING_SCENEMANAGER_HH 

#include <string>
#include "gazebo/common/SingletonT.hh"
#include "ignition/rendering/RenderTypes.hh"
#include "ignition/rendering/Util.hh"

namespace ignition
{
  namespace rendering
  {
    class IGNITION_VISIBLE SceneManager :
      public virtual SingletonT<SceneManager>
    {
      public: SceneManager();

      public: ~SceneManager();

      public: void Load();

      public: void Init();

      public: void Fini();

      public: unsigned int GetSceneCount() const;

      public: bool HasScene(unsigned int _id) const;

      public: bool HasScene(const std::string &_name) const;

      public: bool HasScene(ConstScenePtr _scene) const;

      public: ScenePtr GetScene(unsigned int _id) const;

      public: ScenePtr GetScene(const std::string &_name) const;

      public: ScenePtr GetSceneAt(unsigned int _index) const;

      public: void AddScene(ScenePtr _scene);

      public: ScenePtr RemoveScene(unsigned int _id);

      public: ScenePtr RemoveScene(const std::string &_name);

      public: ScenePtr RemoveScene(ScenePtr _scene);

      public: ScenePtr RemoveSceneAt(unsigned int _index);

      public: void RemoveScenes();

      public: void UpdateScenes();

      private: class SceneManagerPrivate *pimpl;

      private: friend class SingletonT<SceneManager>;
    };
  }
}
#endif
