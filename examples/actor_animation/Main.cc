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
#include <ignition/common/MeshManager.hh>
#include <ignition/common/Mesh.hh>
#include <ignition/common/SubMesh.hh>

#include <ignition/rendering.hh>
#include <ignition/rendering/ogre/OgreScene.hh>
#include <ignition/rendering/ogre/OgreMesh.hh>
#include <ignition/rendering/ogre/OgreMeshFactory.hh>

#include <ignition/common/Skeleton.hh>
#include <ignition/common/SkeletonAnimation.hh>

#include "example_config.hh"
#include "GlutWindow.hh"

const std::string RESOURCE_PATH =
    ic::joinPaths(std::string(PROJECT_BINARY_PATH), "media");

using namespace ignition;

//////////////////////////////////////////////////
void buildScene(ir::ScenePtr _scene, ic::SkeletonPtr &_skel)
{
  // initialize _scene
  _scene->SetAmbientLight(0.3, 0.3, 0.3);
  _scene->SetBackgroundColor(0.3, 0.3, 0.3);
  ir::VisualPtr root = _scene->RootVisual();

  // create directional light
  ir::DirectionalLightPtr light0 = _scene->CreateDirectionalLight();
  light0->SetDirection(0.5, 0.5, -1);
  light0->SetDiffuseColor(0.8, 0.8, 0.8);
  light0->SetSpecularColor(0.5, 0.5, 0.5);
  root->AddChild(light0);

  // create a mesh
  ir::VisualPtr actorVisual = _scene->CreateVisual("actor");
  actorVisual->SetLocalPosition(3, 0, 0);
  actorVisual->SetLocalRotation(0, 0, 0);

  ir::MeshDescriptor descriptor;
  descriptor.meshName = ic::joinPaths(RESOURCE_PATH, "walk.dae");
  ic::MeshManager *meshManager = ic::MeshManager::Instance();
  std::cout << "loading" << std::endl;
  descriptor.mesh = meshManager->Load(descriptor.meshName);
  std::cout << "loading done" << std::endl;

  ir::MeshPtr meshGeom = _scene->CreateMesh(descriptor);
  actorVisual->AddGeometry(meshGeom);
  root->AddChild(actorVisual);

  if (meshGeom && descriptor.mesh->HasSkeleton())
  {
    _skel = descriptor.mesh->MeshSkeleton();

    if (!_skel || _skel->AnimationCount() == 0)
    {
      std::cout << "Failed to load animation." << std::endl;
      return;
    }
  }

  // create gray material
  ir::MaterialPtr gray = _scene->CreateMaterial();
  gray->SetAmbient(0.7, 0.7, 0.7);
  gray->SetDiffuse(0.7, 0.7, 0.7);
  gray->SetSpecular(0.7, 0.7, 0.7);

  // create grid visual
  ir::VisualPtr grid = _scene->CreateVisual("grid");
  ir::GridPtr gridGeom = _scene->CreateGrid();
  gridGeom->SetCellCount(20);
  gridGeom->SetCellLength(1);
  gridGeom->SetVerticalCellCount(0);
  grid->AddGeometry(gridGeom);
  grid->SetLocalPosition(3, 0, 0.0);
  grid->SetMaterial(gray);
  root->AddChild(grid);

  // create camera
  ir::CameraPtr camera = _scene->CreateCamera("camera");
  camera->SetLocalPosition(0.0, 0.0, 0.5);
  camera->SetLocalRotation(0.0, 0.0, 0.0);
  camera->SetImageWidth(800);
  camera->SetImageHeight(600);
  camera->SetAntiAliasing(2);
  camera->SetAspectRatio(1.333);
  camera->SetHFOV(IGN_PI / 2);
  root->AddChild(camera);

  run(meshGeom, camera, _skel);
}

//////////////////////////////////////////////////
int main(int _argc, char** _argv)
{
  glutInit(&_argc, _argv);

  ic::Console::SetVerbosity(4);
  ir::CameraPtr camera;

  ic::SkeletonPtr skel;

  std::chrono::nanoseconds nanosec(1);

  using toSeconds = std::chrono::duration<float, std::ratio<1, 1>>;
  std::cout << "Seconds: " << toSeconds(nanosec).count() << std::endl;

  try
  {
    // create and populate scene
    ir::RenderEngine *engine = ir::engine("ogre");
    ir::ScenePtr scene = engine->CreateScene("scene");

    buildScene(scene, skel);
  }
  catch (...)
  {
    // std::cout << ex.what() << std::endl;
    std::cerr << "Error starting up: ogre" << std::endl;
  }

  return 0;
}
