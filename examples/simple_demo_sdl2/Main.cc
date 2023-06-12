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
#include <vector>

#include <gz/common/Console.hh>
#include <gz/rendering.hh>

#include <SDL.h>
#include <SDL_vulkan.h>

#include <vulkan/vulkan.h>

using namespace gz;
using namespace rendering;

//////////////////////////////////////////////////
void buildScene(ScenePtr _scene)
{
  // initialize _scene
  _scene->SetAmbientLight(0.3, 0.3, 0.3);
  VisualPtr root = _scene->RootVisual();

  // create directional light
  DirectionalLightPtr light0 = _scene->CreateDirectionalLight();
  light0->SetDirection(-0.5, 0.5, -1);
  light0->SetDiffuseColor(0.5, 0.5, 0.5);
  light0->SetSpecularColor(0.5, 0.5, 0.5);
  root->AddChild(light0);

  // // create point light
  // PointLightPtr light1 = _scene->CreatePointLight();
  // light1->SetDiffuseColor(0.5, 0.5, 0.5);
  // light1->SetSpecularColor(0.5, 0.5, 0.5);
  // light1->SetLocalPosition(5, -5, 10);
  // root->AddChild(light1);

  // create point light
  PointLightPtr light2 = _scene->CreatePointLight();
  light2->SetDiffuseColor(0.5, 0.5, 0.5);
  light2->SetSpecularColor(0.5, 0.5, 0.5);
  light2->SetLocalPosition(3, 5, 5);
  root->AddChild(light2);

  // create green material
  MaterialPtr green = _scene->CreateMaterial();
  green->SetAmbient(0.0, 0.5, 0.0);
  green->SetDiffuse(0.0, 0.7, 0.0);
  green->SetSpecular(0.5, 0.5, 0.5);
  green->SetShininess(50);
  green->SetReflectivity(0);

  // create center visual
  VisualPtr center = _scene->CreateVisual();
  center->AddGeometry(_scene->CreateSphere());
  center->SetLocalPosition(3, 0, 0);
  center->SetLocalScale(0.1, 0.1, 0.1);
  center->SetMaterial(green);
  root->AddChild(center);

//! [red material]
  // create red material
  MaterialPtr red = _scene->CreateMaterial();
  red->SetAmbient(0.5, 0.0, 0.0);
  red->SetDiffuse(1.0, 0.0, 0.0);
  red->SetSpecular(0.5, 0.5, 0.5);
  red->SetShininess(50);
  red->SetReflectivity(0);
  red->SetRenderOrder(3);
//! [red material]

  // create sphere visual
  VisualPtr sphere = _scene->CreateVisual();
  sphere->AddGeometry(_scene->CreateSphere());
  sphere->SetOrigin(0.0, -0.5, 0.0);
  sphere->SetLocalPosition(3, 0, 0);
  sphere->SetLocalRotation(0, 0, 0);
  sphere->SetLocalScale(1, 1, 1);
  sphere->SetMaterial(red);
  root->AddChild(sphere);

  // create blue material
  MaterialPtr blue = _scene->CreateMaterial();
  blue->SetAmbient(0.0, 0.0, 0.3);
  blue->SetDiffuse(0.0, 0.0, 0.8);
  blue->SetSpecular(0.5, 0.5, 0.5);
  blue->SetShininess(50);
  blue->SetReflectivity(0);

  // create box visual
  VisualPtr box = _scene->CreateVisual();
  box->AddGeometry(_scene->CreateBox());
  box->SetOrigin(0.0, 0.5, 0.0);
  box->SetLocalPosition(3, 0, 0);
  box->SetLocalRotation(GZ_PI / 4, 0, GZ_PI / 3);
  box->SetLocalScale(1, 2.5, 1);
  box->SetMaterial(blue);
  root->AddChild(box);

  // create ellipsoid visual
  VisualPtr ellipsoidVisual = _scene->CreateVisual();
  auto ellipsoid = _scene->CreateSphere();
  ellipsoidVisual->SetLocalScale(1.2, 0.7, 0.5);
  ellipsoidVisual->AddGeometry(ellipsoid);
  ellipsoidVisual->SetLocalPosition(3, -1, 0);
  ellipsoidVisual->SetMaterial(green);
  root->AddChild(ellipsoidVisual);

//! [white material]
  // create white material
  MaterialPtr white = _scene->CreateMaterial();
  white->SetAmbient(0.5, 0.5, 0.5);
  white->SetDiffuse(0.8, 0.8, 0.8);
  white->SetReceiveShadows(true);
  white->SetReflectivity(0);
  white->SetRenderOrder(0);
//! [white material]

  VisualPtr capsuleVisual = _scene->CreateVisual();
  CapsulePtr capsule = _scene->CreateCapsule();
  capsule->SetLength(0.2);
  capsule->SetRadius(0.2);
  capsuleVisual->AddGeometry(capsule);
  capsuleVisual->SetOrigin(0.0, 0.0, 0.0);
  capsuleVisual->SetLocalPosition(4, 2, 0);
  capsuleVisual->SetLocalScale(1, 1, 1);
  capsuleVisual->SetMaterial(red);
  root->AddChild(capsuleVisual);

  // create plane visual
  VisualPtr plane = _scene->CreateVisual();
  plane->AddGeometry(_scene->CreatePlane());
  plane->SetLocalScale(5, 8, 1);
  plane->SetLocalPosition(3, 0, -0.5);
  plane->SetMaterial(white);
  root->AddChild(plane);

  // create plane visual
  VisualPtr plane2 = _scene->CreateVisual();
  plane2->AddGeometry(_scene->CreatePlane());
  plane2->SetLocalScale(5, 8, 1);
  plane2->SetLocalPosition(4, 0.5, -0.5);
  plane2->Scale(0.1, 0.1, 1);
  plane2->SetMaterial(red);
  root->AddChild(plane2);

  // create axis visual
  VisualPtr axis = _scene->CreateAxisVisual();
  axis->SetLocalPosition(4.0, 0.5, -0.4);
  root->AddChild(axis);

  // create camera
  CameraPtr camera = _scene->CreateCamera("camera");
  camera->SetLocalPosition(0.0, 0.0, 0.0);
  camera->SetLocalRotation(0.0, 0.0, 0.0);
  camera->SetImageWidth(800);
  camera->SetImageHeight(600);
  camera->SetAntiAliasing(2);
  camera->SetAspectRatio(1.333);
  camera->SetHFOV(GZ_PI / 2);
  root->AddChild(camera);

  // track target
  camera->SetTrackTarget(box);
}

//////////////////////////////////////////////////
CameraPtr createCamera(const std::string &_engineName,
    const std::map<std::string, std::string>& _params)
{
  // create and populate scene
  RenderEngine *engine = rendering::engine(_engineName, _params);
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

void SetupVulkan(std::vector<const char*> _instance_extensions)
{
  VkResult err;
  VkInstanceCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

  std::vector<VkExtensionProperties> properties;
  uint32_t properties_count;
  vkEnumerateInstanceExtensionProperties(nullptr, &properties_count, nullptr);
  properties.resize(properties_count);
  err = vkEnumerateInstanceExtensionProperties(nullptr, &properties_count, properties.data());

  for (auto property: properties)
  {
    std::cout << "Extension property found: " << property.extensionName << std::endl;
  }
}

//////////////////////////////////////////////////
int main(int _argc, char** _argv)
{
  common::Console::SetVerbosity(4);

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER)  != 0)
  {
    std::cerr << "could not init sdl" << std::endl;
    return -1;
  }

  auto window_flags = static_cast<SDL_WindowFlags>(SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
  SDL_Window* window = SDL_CreateWindow("simple_demo_sdl2", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
}
