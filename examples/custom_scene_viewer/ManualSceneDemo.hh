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
#ifndef IGNITION_RENDERING_EXAMPLES_CUSTOM_SCENE_VIEWER_MANUALSCENEDEMO_HH_
#define IGNITION_RENDERING_EXAMPLES_CUSTOM_SCENE_VIEWER_MANUALSCENEDEMO_HH_

#include <string>
#include <ignition/rendering.hh>
#include "TestTypes.hh"

namespace ignition
{
  namespace rendering
  {
    class ManualSceneDemo :
      public std::enable_shared_from_this<ManualSceneDemo>
    {
      public: ManualSceneDemo();

      public: virtual ~ManualSceneDemo();

      public: virtual unsigned int SceneCount() const;

      public: virtual void AddScene(SceneBuilderPtr _builder);

      public: virtual void PrevScene();

      public: virtual void NextScene();

      public: virtual void SelectScene(int _index);

      public: virtual SceneBuilderPtr CurrentScene();

      public: virtual unsigned int CameraCount() const;

      public: virtual void AddCamera(const std::string &_engineName);

      public: virtual void PrevCamera();

      public: virtual void NextCamera();

      public: virtual void SelectCamera(int _index);

      public: virtual CameraPtr Camera(int _index);

      public: virtual CameraPtr CurrentCamera();

      public: virtual void Run();

      public: virtual void Update();

      protected: virtual void ChangeScene();

      protected: SceneBuilderList builders;

      protected: SceneList scenes;

      protected: CameraList cameras;

      protected: int builderIndex;

      protected: int cameraIndex;
    };
  }
}

#endif
