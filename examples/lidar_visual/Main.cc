/*
 * Copyright (C) 2019 Open Source Robotics Foundation
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
#include <ignition/common/Skeleton.hh>
#include <ignition/common/SkeletonAnimation.hh>
#include <ignition/common/MeshManager.hh>
#include <ignition/common/Mesh.hh>
#include <ignition/rendering.hh>

#include "example_config.hh"
#include "GlutWindow.hh"

using namespace ignition;
using namespace rendering;

const std::string RESOURCE_PATH =
    common::joinPaths(std::string(PROJECT_BINARY_PATH), "media");

//////////////////////////////////////////////////
void buildScene(ScenePtr _scene)
{
  // initialize _scene
  _scene->SetAmbientLight(0.3, 0.3, 0.3);
  _scene->SetBackgroundColor(0.3, 0.3, 0.3);
  VisualPtr root = _scene->RootVisual();

  // create directional light
  DirectionalLightPtr light0 = _scene->CreateDirectionalLight();
  light0->SetDirection(0.5, 0.5, -1);
  light0->SetDiffuseColor(0.8, 0.8, 0.8);
  light0->SetSpecularColor(0.5, 0.5, 0.5);
  root->AddChild(light0);

  // create gray material
  MaterialPtr gray = _scene->CreateMaterial();
  gray->SetAmbient(0.7, 0.7, 0.7);
  gray->SetDiffuse(0.7, 0.7, 0.7);
  gray->SetSpecular(0.7, 0.7, 0.7);

  // create grid visual
  GridPtr gridGeom = _scene->CreateGrid();
  if (gridGeom)
  {
    VisualPtr grid = _scene->CreateVisual();
    gridGeom->SetCellCount(20);
    gridGeom->SetCellLength(1);
    gridGeom->SetVerticalCellCount(0);
    grid->AddGeometry(gridGeom);
    grid->SetLocalPosition(3, 0, 0.0);
    grid->SetMaterial(gray);
    root->AddChild(grid);
  }

  //create lidar visual
  LidarVisualPtr lidar = _scene->CreateLidarVisual();

  if ( !lidar)
  {
    std::cout << "NULLPOINTER IS RETURNED\n";
  }
  std::vector<double> pts{10.0, 15.0, 15.0, 15.0, INFINITY, INFINITY, INFINITY, 10, 3.5};
  lidar->SetMinHorizontalAngle(0.0);
  lidar->SetHorizontalAngleStep(0.3);
  lidar->SetHorizontalRayCount(3);
  lidar->SetMaxHorizontalAngle(5);
  lidar->SetMaxVerticalAngle(10);
  lidar->SetMinVerticalAngle(0.2);
  lidar->SetVerticalAngleStep(0.08);
  lidar->SetVerticalRayCount(3);
  lidar->SetMaxRange(50);
  lidar->SetMinRange(0.5);
  lidar->OnMsg(pts);
  lidar->Update();
  lidar->SetLocalPosition(3,0,0);
  root->AddChild(lidar);

  // create camera
  CameraPtr camera = _scene->CreateCamera("camera");
  camera->SetLocalPosition(0.0, 0.0, 2.0);
  camera->SetLocalRotation(0.0, 0.5, 0.0);
  camera->SetImageWidth(1600);
  camera->SetImageHeight(1200);
  camera->SetAntiAliasing(2);
  camera->SetAspectRatio(1.333);
  camera->SetHFOV(IGN_PI / 2);
  root->AddChild(camera);

}

//////////////////////////////////////////////////
CameraPtr createCamera(const std::string &_engineName)
{
  // create and populate scene
  RenderEngine *engine = rendering::engine(_engineName);
  if (!engine)
  {
    ignwarn << "Engine '" << _engineName
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
  std::string engine("ogre");
  if (_argc > 1)
  {
    engine = _argv[1];
  }

  common::Console::SetVerbosity(4);
  std::vector<std::string> engineNames;
  std::vector<CameraPtr> cameras;

  engineNames.push_back(engine);

  std::vector<VisualPtr> visuals;
  ic::SkeletonPtr skel = nullptr;

  for (auto engineName : engineNames)
  {
    std::cout << "Starting engine [" << engineName << "]" << std::endl;
    try
    {
      CameraPtr camera = createCamera(engineName);
      if (camera)
      {
        cameras.push_back(camera);
      }
    }
    catch (...)
    {
      std::cerr << "Error starting up: " << engineName << std::endl;
    }
  }
  run(cameras);
  return 0;
}
