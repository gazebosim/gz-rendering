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

#if defined(__APPLE__)
  #include <OpenGL/gl.h>
  #include <GLUT/glut.h>
#elif not defined(_WIN32)
  #include <GL/glew.h>
  #include <GL/gl.h>
  #include <GL/glut.h>
#endif

#include <iostream>
#include <vector>

#include <ignition/common/Console.hh>
#include <ignition/rendering.hh>

#include "GlutWindow.hh"

using namespace ignition;
using namespace rendering;

//////////////////////////////////////////////////
void buildScene(ScenePtr _scene)
{
  // DirectionalLightPtr light0 = _scene->CreateDirectionalLight();
  // light0->SetDirection(-0.5, 0.5, -1);
  // light0->SetDiffuseColor(0.00001, 0.00001, 0.00001);

  // PointLightPtr light2 = _scene->CreatePointLight();
  // light2->SetDiffuseColor(0.5, 0.5, 0.5);
  // light2->SetLocalPosition(3, 5, 5);

  // MaterialPtr red = _scene->CreateMaterial();
  // red->SetDiffuse(1.0, 0.0, 0.0);

  // VisualPtr leftWall = _scene->CreateVisual();
  // leftWall->AddGeometry(_scene->CreatePlane());
  // leftWall->SetLocalScale(1, 5, 5);
  // leftWall->SetLocalRotation(0, IGN_PI / 2, 0);
  // leftWall->SetLocalPosition(0, 0, 2.5);
  // leftWall->SetMaterial(red);
  // 
  // MaterialPtr green = _scene->CreateMaterial();
  // green->SetDiffuse(0.0, 1.0, 0.0);
  // 
  // VisualPtr rightWall = _scene->CreateVisual();
  // rightWall->AddGeometry(_scene->CreatePlane());
  // rightWall->SetLocalScale(1, 5, 5);
  // rightWall->SetLocalRotation(0, IGN_PI / 2, 0);
  // rightWall->SetLocalPosition(0, 0, -2.5);
  // rightWall->SetMaterial(green);

  // MaterialPtr white = _scene->CreateMaterial();
  // white->SetDiffuse(1.0, 1.0, 1.0);
  // 
  // VisualPtr backWall = _scene->CreateVisual();
  // backWall->AddGeometry(_scene->CreatePlane());
  // backWall->SetLocalScale(5, 1, 5);
  // backWall->SetLocalRotation(IGN_PI / 2, 0, 0);
  // backWall->SetLocalPosition(0, 0, -2.5);
  // backWall->SetMaterial(white);

  // VisualPtr floor = _scene->CreateVisual();
  // floor->AddGeometry(_scene->CreatePlane());
  // floor->SetLocalScale(5, 5, 1);
  // floor->SetLocalRotation(0, 0, 0);
  // floor->SetLocalPosition(0, 0, -2.5);
  // floor->SetMaterial(white);

  // VisualPtr ceiling = _scene->CreateVisual();
  // ceiling->AddGeometry(_scene->CreatePlane());
  // ceiling->SetLocalScale(5, 5, 1);
  // ceiling->SetLocalRotation(0, 0, 0);
  // ceiling->SetLocalPosition(0, 0, 2.5);
  // ceiling->SetMaterial(white);

  // MaterialPtr lightMaterial = _scene->CreateMaterial();
  // lightMaterial->SetEmissive(1.0, 1.0, 1.0);
  // lightMaterial->SetDiffuse(1.0, 1.0, 1.0);

  // VisualPtr ceilingLight = _scene->CreateVisual();
  // ceiling->AddGeometry(_scene->CreatePlane());
  // ceiling->SetLocalScale(2.5, 2.5, 1);
  // ceiling->SetLocalRotation(0, 0, 0);
  // ceiling->SetLocalPosition(0, 0, 2.5);
  // ceiling->SetMaterial(lightMaterial);

  MaterialPtr boxMaterial1 = _scene->CreateMaterial();
  boxMaterial1->SetDiffuse(1.0, 1.0, 1.0);

  // MaterialPtr boxMaterial2 = _scene->CreateMaterial();
  // boxMaterial2->SetDiffuse(1.0, 1.0, 1.0);

  VisualPtr box1 = _scene->CreateVisual();
  box1->AddGeometry(_scene->CreateBox());
  box1->SetLocalPosition(0, -0.85, -1.25);
  box1->SetLocalRotation(0, 0, -IGN_PI / 4);
  box1->SetLocalScale(1.5, 1.5, 1.5);
  box1->SetMaterial(boxMaterial1);

  // VisualPtr box2 = _scene->CreateVisual();
  // box2->AddGeometry(_scene->CreateBox());
  // box2->SetLocalPosition(0.5, 0.5, -0.5);
  // box2->SetLocalRotation(0, 0, -IGN_PI / 10);
  // box2->SetLocalScale(1.5, 1.5, 3.0);
  // box2->SetMaterial(boxMaterial2);
 
  VisualPtr mesh = _scene->CreateVisual();
  MeshDescriptor descriptor;
  descriptor.meshName = "media/duck.dae";
  common::MeshManager *meshManager = common::MeshManager::Instance();
  descriptor.mesh = meshManager->Load(descriptor.meshName);
  MeshPtr meshGeom = _scene->CreateMesh(descriptor);
  mesh->AddGeometry(meshGeom);
  mesh->SetLocalPosition(3, 0, 0);
  mesh->SetLocalRotation(1.5708, 0, 2.0);
 
  CameraPtr camera = _scene->CreateCamera("camera");
  camera->SetLocalPosition(0.0, 0.0, 0.0);
  camera->SetLocalRotation(0.0, 0.0, 0.0);
  camera->SetImageWidth(400);
  camera->SetImageHeight(300);
  camera->SetAspectRatio(1.333);
  camera->SetHFOV(IGN_PI / 2);

  camera->SetTrackTarget(mesh);
}

//////////////////////////////////////////////////
CameraPtr createCamera(const std::string &_engineName)
{
  // create and populate scene
  RenderEngine *engine = rendering::engine(_engineName);
  if (!engine)
  {
    std::cout << "Engine '" << _engineName
              << "' is not supported" << std::endl;
    return CameraPtr();
  }
  ScenePtr scene = engine->CreateScene("scene");
  buildScene(scene);

  // return camera sensor
  SensorPtr sensor = scene->SensorByName("camera");
  return std::dynamic_pointer_cast<Camera>(sensor);
}

//////////////////////////////////////////////////
int main(int _argc, char** _argv)
{
  glutInit(&_argc, _argv);

  // Expose engine name to command line because we can't instantiate both
  // ogre and ogre2 at the same time
  std::string ogreEngineName("ogre");
  if (_argc > 1)
  {
    ogreEngineName = _argv[1];
  }

  common::Console::SetVerbosity(4);
  std::vector<std::string> engineNames;
  std::vector<CameraPtr> cameras;

  // engineNames.push_back(ogreEngineName);
  engineNames.push_back("LuxCoreEngine");

  for (auto engineName : engineNames)
  {
    try
    {
      CameraPtr camera = createCamera(engineName);
      if (camera)
      {
        cameras.push_back(camera);
      }
    }
    catch (std::exception & ex)
    {
      std::cout << ex.what() << std::endl;
      // std::cerr << "Error starting up: " << engineName << std::endl;
    }
  }
  run(cameras);
  return 0;
}
