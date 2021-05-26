/*
 * Copyright (C) 2021 Open Source Robotics Foundation
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
#include <ignition/common/MeshManager.hh>
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

  ////////////////////// Lights ///////////////////////
  // create directional light
  DirectionalLightPtr light0 = _scene->CreateDirectionalLight();
  light0->SetDirection(-0.5, 0.5, -1);
  light0->SetDiffuseColor(0.5, 0.5, 0.5);
  light0->SetSpecularColor(0.5, 0.5, 0.5);
  root->AddChild(light0);
  // create point light
  PointLightPtr light2 = _scene->CreatePointLight();
  light2->SetDiffuseColor(0.5, 0.5, 0.5);
  light2->SetSpecularColor(0.5, 0.5, 0.5);
  light2->SetLocalPosition(3, 5, 5);
  root->AddChild(light2);

  ////////////////////// Materials ///////////////////////
  // create green material
  MaterialPtr green = _scene->CreateMaterial();
  green->SetAmbient(0.0, 0.5, 0.0);
  green->SetDiffuse(0.0, 1.0, 0.0);
  green->SetSpecular(0.5, 0.5, 0.5);
  green->SetShininess(50);
  green->SetReflectivity(0);
  // create red material
  MaterialPtr red = _scene->CreateMaterial();
  red->SetAmbient(0.0, 0.0, 0.5);
  red->SetDiffuse(0.0, 0.0, 1.0);
  red->SetSpecular(0.5, 0.5, 0.5);
  red->SetShininess(50);
  red->SetReflectivity(0);
  // create yellow material
  MaterialPtr yellow = _scene->CreateMaterial();
  yellow->SetAmbient(0.0, 0.5, 0.5);
  yellow->SetDiffuse(0.0, 1.0, 1.0);
  yellow->SetShininess(50);
  yellow->SetReflectivity(0);

  ////////////////////// Visuals ///////////////////////
  // create plane visual
  VisualPtr plane = _scene->CreateVisual("plane");
  plane->AddGeometry(_scene->CreatePlane());
  plane->SetLocalScale(5, 8, 1);
  plane->SetLocalPosition(3, 0, -0.5);
  root->AddChild(plane);

  // create a mesh
  VisualPtr mesh = _scene->CreateVisual();
  mesh->SetLocalPosition(3, 0, 0);
  mesh->SetLocalRotation(1.5708, 0, 2.0);
  MeshDescriptor descriptor;
  descriptor.meshName = common::joinPaths(RESOURCE_PATH, "duck.dae");
  common::MeshManager *meshManager = common::MeshManager::Instance();
  descriptor.mesh = meshManager->Load(descriptor.meshName);
  MeshPtr meshGeom = _scene->CreateMesh(descriptor);
  mesh->AddGeometry(meshGeom);
  mesh->SetMaterial(yellow);
  mesh->SetUserData("label", 5);
  root->AddChild(mesh);

  // create a box
  VisualPtr box = _scene->CreateVisual("box");
  box->SetLocalPosition(3, 1.5, 0);
  box->SetLocalRotation(0, 0, 0.7);
  GeometryPtr boxGeom = _scene->CreateBox();
  box->AddGeometry(boxGeom);
  box->SetMaterial(green);
  box->SetUserData("label", 2);
  root->AddChild(box);

  // create a box
  VisualPtr box2 = _scene->CreateVisual("box2");
  box2->SetLocalPosition(2, -1, 1);
  box2->SetLocalRotation(0, 0.3, 0.7);
  GeometryPtr boxGeom2 = _scene->CreateBox();
  box2->AddGeometry(boxGeom2);
  box2->SetMaterial(green);
  box2->SetUserData("label", 2);
  root->AddChild(box2);

  // create a sphere
  VisualPtr sphere = _scene->CreateVisual("sphere");
  sphere->SetLocalPosition(3, -1.5, 0);
  GeometryPtr sphereGeom = _scene->CreateSphere();
  sphere->AddGeometry(sphereGeom);
  sphere->SetMaterial(red);
  sphere->SetUserData("label", 3);
  root->AddChild(sphere);

  // create a sphere2
  VisualPtr sphere2 = _scene->CreateVisual("sphere2");
  sphere2->SetLocalPosition(5, 4, 2);
  GeometryPtr sphereGeom2 = _scene->CreateSphere();
  sphere2->AddGeometry(sphereGeom2);
  sphere2->SetMaterial(red);
  sphere2->SetUserData("label", 3);
  root->AddChild(sphere2);

  // create camera
  SegmentationCameraPtr camera = _scene->CreateSegmentationCamera("camera");
  camera->SetLocalPosition(0.0, 0.0, 0.5);
  camera->SetLocalRotation(0.0, 0.0, 0.0);
  camera->SetImageWidth(800);
  camera->SetImageHeight(600);
  camera->SetImageFormat(PixelFormat::PF_R8G8B8);
  camera->SetAspectRatio(1.333);
  camera->SetHFOV(IGN_PI / 2);
  camera->EnableColoredMap(true);
  camera->SetSegmentationType(SegmentationType::Semantic);
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
  std::string ogreEngineName("ogre2");
  if (_argc > 1)
  {
    ogreEngineName = _argv[1];
  }

  common::Console::SetVerbosity(4);
  std::vector<std::string> engineNames;
  std::vector<CameraPtr> cameras;

  engineNames.push_back(ogreEngineName);

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
    catch (...)
    {
      std::cerr << "Error starting up: " << engineName << std::endl;
    }
  }
  run(cameras);
  return 0;
}
