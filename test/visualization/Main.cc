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

#include "gazebo/common/Console.hh"
#include "ignition/rendering/rendering.hh"
#include "GlutWindow.hh"

#include <GL/glut.h>
#include <GL/gl.h>

using namespace ignition;
using namespace rendering;

int main(int, char**)
{
  gazebo::common::Console::SetQuiet(false);

  // create new scene
  RenderEngine *engine = rendering::get_engine("ogre");
  ScenePtr scene = engine->CreateScene("scene");
  VisualPtr root = scene->GetRootVisual();

  // create point light
  DirectionalLightPtr light = scene->CreateDirectionalLight();
  light->SetDirection(0.0, 0.5, -1.0);
  light->SetDiffuseColor(0.5, 0.5, 0.5);
  light->SetSpecularColor(0.5, 0.5, 0.5);
  light->SetLocalPosition(2, -5, 10);
  root->AddChild(light);

  // create green material
  MaterialPtr green = scene->CreateMaterial();
  green->SetAmbient(0.0, 0.5, 0.0);
  green->SetDiffuse(0.0, 0.7, 0.0);
  green->SetSpecular(0.5, 0.5, 0.5);
  green->SetShininess(50);

  // create center visual
  VisualPtr center = scene->CreateVisual();
  center->AddGeometry(scene->CreateSphere());
  center->SetLocalPosition(3, 0, 0);
  center->SetScale(0.1, 0.1, 0.1);
  center->SetMaterial(green);
  root->AddChild(center);

  // create red material
  MaterialPtr red = scene->CreateMaterial();
  red->SetAmbient(0.5, 0.0, 0.0);
  red->SetDiffuse(0.7, 0.0, 0.0);
  red->SetSpecular(0.5, 0.5, 0.5);
  red->SetShininess(50);

  // create sphere visual
  VisualPtr sphere = scene->CreateVisual();
  sphere->AddGeometry(scene->CreateSphere());
  sphere->SetOrigin(0.0, -0.5, 0.0);
  sphere->SetLocalPosition(3, 0, 0);
  sphere->SetLocalRotation(0, 0, 0);
  sphere->SetScale(1, 2.5, 1);
  sphere->SetMaterial(red);
  root->AddChild(sphere);

  // create blue material
  MaterialPtr blue = scene->CreateMaterial();
  blue->SetAmbient(0.0, 0.0, 0.3);
  blue->SetDiffuse(0.0, 0.0, 0.8);
  blue->SetSpecular(0.5, 0.5, 0.5);
  blue->SetShininess(50);

  // create box visual
  VisualPtr box = scene->CreateVisual();
  box->AddGeometry(scene->CreateBox());
  box->SetOrigin(0.0, 0.5, 0.0);
  box->SetLocalPosition(3, 0, 0);
  box->SetLocalRotation(M_PI / 4, 0, M_PI / 3);
  box->SetScale(1, 2.5, 1);
  box->SetMaterial(blue);
  root->AddChild(box);

  // create white material
  MaterialPtr white = scene->CreateMaterial();
  white->SetAmbient(0.5, 0.5, 0.5);
  white->SetDiffuse(0.8, 0.8, 0.8);
  white->SetReceiveShadows(true);

  // create sphere visual
  VisualPtr plane = scene->CreateVisual();
  plane->AddGeometry(scene->CreatePlane());
  plane->SetScale(3, 5, 1);
  plane->SetLocalPosition(3, 0, -0.5);
  plane->SetMaterial(white);
  root->AddChild(plane);

  // create camera
  CameraPtr camera = scene->CreateCamera();
  camera->SetLocalPosition(0.0, 0.0, 1.0);
  camera->SetLocalRotation(0.0, 0.3, 0.0);
  camera->SetImageSize(800, 800);
  camera->SetAntiAliasing(4);
  root->AddChild(camera);

  GlutRun(camera);
  return 0;
}
