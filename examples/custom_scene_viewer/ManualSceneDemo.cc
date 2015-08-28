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
#include <iostream>
#include "ManualSceneDemo.hh"
#include "SceneBuilder.hh"
#include "DemoWindow.hh"

using namespace ignition;
using namespace rendering;

//////////////////////////////////////////////////
ManualSceneDemo::ManualSceneDemo() :
  builderIndex(0),
  cameraIndex(0)
{
}

//////////////////////////////////////////////////
ManualSceneDemo::~ManualSceneDemo()
{
}

//////////////////////////////////////////////////
unsigned int ManualSceneDemo::GetSceneCount() const
{
  return this->builders.size();
}

//////////////////////////////////////////////////
void ManualSceneDemo::AddScene(SceneBuilderPtr _builder)
{
  this->builders.push_back(_builder);
}

//////////////////////////////////////////////////
void ManualSceneDemo::PrevScene()
{
  this->SelectScene(this->builderIndex - 1);
}

//////////////////////////////////////////////////
void ManualSceneDemo::NextScene()
{
  this->SelectScene(this->builderIndex + 1);
}

//////////////////////////////////////////////////
void ManualSceneDemo::SelectScene(int _index)
{
  this->builderIndex = _index;
  this->builderIndex %= this->builders.size();
  this->ChangeScene();
}

//////////////////////////////////////////////////
SceneBuilderPtr ManualSceneDemo::GetCurrentScene()
{
  return this->builders[this->builderIndex];
}

//////////////////////////////////////////////////
unsigned int ManualSceneDemo::GetCameraCount() const
{
  return this->cameras.size();
}

//////////////////////////////////////////////////
void ManualSceneDemo::AddCamera(const std::string &_engineName)
{
  RenderEngine *engine = get_engine(_engineName);
  ScenePtr scene = engine->CreateScene("Scene");
  VisualPtr root = scene->GetRootVisual();

  CameraPtr camera = scene->CreateCamera("Camera");
  camera->SetImageWidth(640);
  camera->SetImageHeight(480);
  camera->SetAntiAliasing(2);
  camera->SetAspectRatio(1.333);
  camera->SetHFOV(M_PI / 2);
  root->AddChild(camera);

  this->scenes.push_back(scene);
  this->cameras.push_back(camera);
}

//////////////////////////////////////////////////
void ManualSceneDemo::PrevCamera()
{
  this->SelectCamera(this->cameraIndex - 1);
}

//////////////////////////////////////////////////
void ManualSceneDemo::NextCamera()
{
  this->SelectCamera(this->cameraIndex + 1);
}

//////////////////////////////////////////////////
void ManualSceneDemo::SelectCamera(int _index)
{
  this->cameraIndex = _index;
  this->cameraIndex %= this->cameras.size();
}

//////////////////////////////////////////////////
CameraPtr ManualSceneDemo::GetCamera(int _index)
{
  unsigned int i = _index % this->cameras.size();
  return this->cameras[i];
}

//////////////////////////////////////////////////
CameraPtr ManualSceneDemo::GetCurrentCamera()
{
  return this->cameras[this->cameraIndex];
}

//////////////////////////////////////////////////
void ManualSceneDemo::Run()
{
  for (auto builder : this->builders)
  {
    builder->SetScenes(this->scenes);
    builder->SetCameras(this->cameras);
  }

  this->SelectScene(0);
  this->SelectCamera(0);
  GlutRun(shared_from_this());
}

//////////////////////////////////////////////////
void ManualSceneDemo::Update()
{
  this->GetCurrentScene()->UpdateScenes();
}

//////////////////////////////////////////////////
void ManualSceneDemo::ChangeScene()
{
  SceneBuilderPtr builder = this->GetCurrentScene();
  builder->ResetCameras();
  builder->BuildScenes();
}

//////////////////////////////////////////////////
//////////////////////////////////////////////////
int main(int, char**)
{
  gazebo::common::Console::SetQuiet(false);
  ManualSceneDemoPtr sceneDemo(new ManualSceneDemo);
  sceneDemo->AddScene(SceneBuilderPtr(new SceneBuilder));
  // sceneDemo->AddScene(SceneBuilderPtr(new BoxSceneBuilder));
  sceneDemo->AddScene(SceneBuilderPtr(new SimpleSceneBuilder));
  sceneDemo->AddScene(SceneBuilderPtr(new AllShapesSceneBuilder));
  sceneDemo->AddScene(SceneBuilderPtr(new TextureSceneBuilder));
  sceneDemo->AddScene(SceneBuilderPtr(new NormalMapSceneBuilder));
  sceneDemo->AddScene(SceneBuilderPtr(new ReflectionSceneBuilder));
  // sceneDemo->AddScene(SceneBuilderPtr(new TransparencySceneBuilder));
  sceneDemo->AddScene(SceneBuilderPtr(new ShadowSceneBuilder(1)));
  sceneDemo->AddScene(SceneBuilderPtr(new ShadowSceneBuilder(2)));
  sceneDemo->AddScene(SceneBuilderPtr(new ShadowSceneBuilder(3)));
  sceneDemo->AddScene(SceneBuilderPtr(new ShadowSceneBuilder(4)));
  sceneDemo->AddScene(SceneBuilderPtr(new ShadowSceneBuilder(5)));
  // sceneDemo->AddCamera("ogre");
  sceneDemo->AddCamera("optix");
  sceneDemo->Run();
  return 0;
}
