/*
 * Copyright (C) 2018 Open Source Robotics Foundation
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
  _scene->SetAmbientLight(0.8, 0.8, 0.8);
  _scene->SetBackgroundColor(0.1, 0.1, 0.1);
  VisualPtr root = _scene->RootVisual();

  std::vector<std::string> meshes;
  meshes.push_back("drill");
  meshes.push_back("extinguisher");
  meshes.push_back("rescue_randy");
  meshes.push_back("valve");
  meshes.push_back("pump");
  meshes.push_back("PBRTest");

  std::map<std::string, VisualPtr> visuals;

  // create PBR material
  double x = -1.0 * meshes.size() * 0.5;
  double y = 0;
  std::string environmentMap =
      common::joinPaths(RESOURCE_PATH, "fort_point.dds");
  for (auto mesh : meshes)
  {
    MaterialPtr matPBR = _scene->CreateMaterial(mesh);
    std::string textureMap = common::joinPaths(RESOURCE_PATH, mesh,
        mesh + "_albedo.png");
    matPBR->SetDiffuse(1.0, 1.0, 1.0);
    matPBR->SetTexture(textureMap);
//    matPBR->SetRoughness(0.5);

    if (mesh == "pump" || mesh == "extinguisher" || mesh == "valve" || mesh == "PBRTest")
    {
      matPBR->SetMetalness(0.9);
      matPBR->SetRoughness(0.5);
    }
    matPBR->SetSpecular(1.0, 1.0, 1.0);
    matPBR->SetCastShadows(true);

      // create mesh for PBR
    VisualPtr meshPBR = _scene->CreateVisual(mesh);
    meshPBR->SetLocalPosition(x, y, 0.0);
    if (mesh == "pump")
      meshPBR->SetLocalRotation(0, 0, 1.57);
    else
      meshPBR->SetLocalRotation(0, 0, 0);
    MeshDescriptor descriptorPBR;
    std::string extension = ".dae";
    if (mesh == "PBRTest")
    {
      extension = ".obj";
      meshPBR->SetLocalRotation(1.57, 0, 0);
      meshPBR->SetLocalPosition(x, y, 1.0);
    }
    descriptorPBR.meshName = common::joinPaths(RESOURCE_PATH, mesh,
        mesh + extension);
    common::MeshManager *meshManager = common::MeshManager::Instance();
    descriptorPBR.mesh = meshManager->Load(descriptorPBR.meshName);
    MeshPtr meshPBRGeom = _scene->CreateMesh(descriptorPBR);
    meshPBRGeom->SetMaterial(matPBR);
    meshPBR->AddGeometry(meshPBRGeom);
    root->AddChild(meshPBR);
    x += 1.0;

    visuals[mesh] = meshPBR;
  }

  // manually position and scale the meshes
//  VisualPtr extinguisher = visuals["extinguisher"];
//  matPBR->SetEnvironmentMap(environmentMap);
/*  {
    std::string mesh = "extinguisher";
    MaterialPtr matPBR = _scene->CreateMaterial();
    std::string textureMap = common::joinPaths(RESOURCE_PATH, mesh,
        mesh + "_albedo.png");
    std::string normalMap = common::joinPaths(RESOURCE_PATH, mesh,
        mesh + "_normal.png");
    std::string roughnessMap = common::joinPaths(RESOURCE_PATH, mesh,
        mesh + "_roughness.png");
    std::string metalnessMap = common::joinPaths(RESOURCE_PATH, mesh,
       mesh + "_metalness.png");
    matPBR->SetDiffuse(1.0, 1.0, 1.0);
    matPBR->SetTexture(textureMap);
    matPBR->SetNormalMap(normalMap);
    matPBR->SetRoughness(1.07);
    matPBR->SetRoughnessMap(roughnessMap);
    matPBR->SetMetalness(1.0);
    matPBR->SetMetalnessMap(metalnessMap);
    matPBR->SetEnvironmentMap(environmentMap);
    matPBR->SetSpecular(1.0, 1.0, 1.0);

    // create mesh for PBR
    VisualPtr meshPBR = _scene->CreateVisual(mesh);
    meshPBR->SetLocalPosition(0, 0, 0.0);
    meshPBR->SetLocalRotation(0, 0, 0);
    MeshDescriptor descriptorPBR;
    descriptorPBR.meshName = common::joinPaths(RESOURCE_PATH, mesh,
        mesh + ".dae");
    common::MeshManager *meshManager = common::MeshManager::Instance();
    descriptorPBR.mesh = meshManager->Load(descriptorPBR.meshName);
    MeshPtr meshPBRGeom = _scene->CreateMesh(descriptorPBR);
    meshPBRGeom->SetMaterial(matPBR);
    meshPBR->AddGeometry(meshPBRGeom);
    root->AddChild(meshPBR);
  }
*/

  //---------------------------- non PBR meshes
  /*
  std::vector<std::string> meshes2;
  meshes2.push_back("walking_nurse");
  meshes2.push_back("trolley_bed");
  meshes2.push_back("instrument_cart_1");
  meshes2.push_back("truck");


  // create PBR material
  double x2 = -1.0 * meshes2.size() * 2 * 0.5;
  double y2 = -5;
  for (auto mesh : meshes2)
  {
    MaterialPtr matPBR = _scene->CreateMaterial(mesh);
    std::string textureMap = common::joinPaths(RESOURCE_PATH, mesh,
        mesh + ".png");
    matPBR->SetDiffuse(1.0, 1.0, 1.0);
    matPBR->SetTexture(textureMap);
    matPBR->SetSpecular(1.0, 1.0, 1.0);
    matPBR->SetCastShadows(true);

      // create mesh for PBR
    VisualPtr meshPBR = _scene->CreateVisual(mesh);
    meshPBR->SetLocalPosition(x2, y2, 0.0);
    if (mesh == "instrument_cart_1")
      meshPBR->SetLocalRotation(0, 0, -1.57);
    else
      meshPBR->SetLocalRotation(0, 0, 3.14);
    MeshDescriptor descriptorPBR;
    if (mesh == "walking_nurse")
      descriptorPBR.meshName = common::joinPaths(RESOURCE_PATH, mesh,
          mesh + ".dae");
    else
      descriptorPBR.meshName = common::joinPaths(RESOURCE_PATH, mesh,
          mesh + ".obj");
    common::MeshManager *meshManager = common::MeshManager::Instance();
    descriptorPBR.mesh = meshManager->Load(descriptorPBR.meshName);
    MeshPtr meshPBRGeom = _scene->CreateMesh(descriptorPBR);
    meshPBRGeom->SetMaterial(matPBR);
    meshPBR->AddGeometry(meshPBRGeom);
    if (mesh == "walking_nurse")
      meshPBR->SetLocalScale(0.01, 0.01, 0.01);
    root->AddChild(meshPBR);
    x2 += 2.0;
  }
  */

  //---------------------------- non PBR meshes end


  // create white material
  MaterialPtr white = _scene->CreateMaterial();
  white->SetDiffuse(1.0, 1.0, 1.0);
  white->SetSpecular(1.0, 1.0, 1.0);

  // create plane visual
  VisualPtr plane = _scene->CreateVisual("plane");
  plane->AddGeometry(_scene->CreatePlane());
  plane->SetLocalScale(20, 20, 1);
  plane->SetLocalPosition(0, 0, 0.0);
  plane->SetMaterial(white);
  root->AddChild(plane);

  VisualPtr planeL = _scene->CreateVisual("planeL");
  planeL->AddGeometry(_scene->CreatePlane());
  planeL->SetLocalScale(20, 20, 1);
  planeL->SetLocalPosition(-8, 0, 0.0);
  planeL->SetLocalRotation(0, 1.57, 0.0);
  planeL->SetMaterial(white);
  root->AddChild(planeL);

  VisualPtr planeR = _scene->CreateVisual("planeR");
  planeR->AddGeometry(_scene->CreatePlane());
  planeR->SetLocalScale(20, 20, 1);
  planeR->SetLocalPosition(8, 0, 0.0);
  planeR->SetLocalRotation(0, -1.57, 0.0);
  planeR->SetMaterial(white);
  root->AddChild(planeR);

  VisualPtr planeB = _scene->CreateVisual("planeB");
  planeB->AddGeometry(_scene->CreatePlane());
  planeB->SetLocalScale(20, 20, 1);
  planeB->SetLocalPosition(0, 3, 0.0);
  planeB->SetLocalRotation(1.57, 0, 0.0);
  planeB->SetMaterial(white);
  root->AddChild(planeB);

  VisualPtr planeF = _scene->CreateVisual("planeF");
  planeF->AddGeometry(_scene->CreatePlane());
  planeF->SetLocalScale(20, 20, 1);
  planeF->SetLocalPosition(0, -8, 0.0);
  planeF->SetLocalRotation(-1.57, 0, 0.0);
  planeF->SetMaterial(white);
  root->AddChild(planeF);





  // create directional light
/*  DirectionalLightPtr light0 = _scene->CreateDirectionalLight("dir");
  light0->SetDirection(0.5, 0.5, -1);
  light0->SetDiffuseColor(1.0, 1.0, 1.0);
  light0->SetSpecularColor(1.0, 1.0, 1.0);
  light0->SetCastShadows(true);
  root->AddChild(light0);
*/

  // create spot light
  SpotLightPtr light1 = _scene->CreateSpotLight();
  light1->SetDiffuseColor(0.7, 0.7, 0.7);
  light1->SetSpecularColor(0.5, 0.5, 0.5);
  light1->SetLocalPosition(-5, -2.5, 8);
  light1->SetDirection(0, 0, -1);
  light1->SetCastShadows(true);
  // light1->SetInnerAngle(1.4);
  light1->SetOuterAngle(2.8);
  light1->SetAttenuationRange(25);
  light1->SetAttenuationLinear(0.020);
  light1->SetAttenuationConstant(0);
  light1->SetAttenuationQuadratic(0.001);
  light1->SetFalloff(0.2);
  root->AddChild(light1);

  // create spot light that does not cast shadows
  SpotLightPtr light3 = _scene->CreateSpotLight();
  light3->SetDiffuseColor(0.7, 0.7, 0.7);
  light3->SetSpecularColor(0.5, 0.5, 0.5);
  light3->SetLocalPosition(5, -2.5, 8);
  light3->SetDirection(0, 0, -1);
  light3->SetCastShadows(true);
  // light3->SetInnerAngle(1.4);
  light3->SetOuterAngle(2.8);
  light3->SetAttenuationRange(25);
  light3->SetAttenuationLinear(0.020);
  light3->SetAttenuationConstant(0);
  light3->SetAttenuationQuadratic(0.001);
  light3->SetFalloff(0.2);

  root->AddChild(light3);

  // create point light
/*  PointLightPtr pointLight = _scene->CreatePointLight("point");
  pointLight->SetDiffuseColor(0.5, 0.6, 0.8);
  pointLight->SetSpecularColor(1.0, 1.0, 1.0);
  pointLight->SetLocalPosition(0, 0, 5);
  pointLight->SetCastShadows(true);
  root->AddChild(pointLight);
*/

  // create camera
  CameraPtr camera = _scene->CreateCamera("camera");
  camera->SetLocalPosition(0.0, -3.0, 3.0);
  camera->SetLocalRotation(0.0, 0.5, IGN_PI * 0.5);
  camera->SetImageWidth(1280);
  camera->SetImageHeight(720);
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
    std::cout << "Engine '" << _engineName
              << "' is not supported" << std::endl;
    return CameraPtr();
  }
  ScenePtr scene = engine->CreateScene("scene");
  buildScene(scene);

  // return camera sensor
  SensorPtr sensor = scene->SensorByName("camera");
  // get render pass system
  CameraPtr camera = std::dynamic_pointer_cast<Camera>(sensor);
  RenderPassSystemPtr rpSystem = engine->RenderPassSystem();
  if (rpSystem)
  {
    // add gaussian noise pass
    RenderPassPtr pass = rpSystem->Create<GaussianNoisePass>();
    GaussianNoisePassPtr noisePass =
        std::dynamic_pointer_cast<GaussianNoisePass>(pass);
    noisePass->SetMean(0.1);
    noisePass->SetStdDev(0.08);
    noisePass->SetEnabled(false);
    camera->AddRenderPass(noisePass);
  }

  return camera;
}

//////////////////////////////////////////////////
int main(int _argc, char** _argv)
{
  glutInit(&_argc, _argv);

  common::Console::SetVerbosity(4);
  std::vector<std::string> engineNames;
  std::vector<CameraPtr> cameras;

  engineNames.push_back("ogre2");
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
      // std::cout << ex.what() << std::endl;
      std::cerr << "Error starting up: " << engineName << std::endl;
    }
  }
  run(cameras);
  return 0;
}
