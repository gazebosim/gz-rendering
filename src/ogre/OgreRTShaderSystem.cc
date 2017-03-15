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
#ifdef _WIN32
  // Ensure that Winsock2.h is included before Windows.h, which can get
  // pulled in by anybody (e.g., Boost).
  #include <Winsock2.h>
#endif

#include <thread>
#include <sys/stat.h>

#include <boost/filesystem.hpp>

#include <ignition/common/Console.hh>
#include "ignition/rendering/ogre/OgreRenderEngine.hh"
#include "ignition/rendering/ogre/OgreScene.hh"
#include "ignition/rendering/ogre/OgreMaterial.hh"
#include "ignition/rendering/ogre/OgreMesh.hh"
#include "ignition/rendering/ogre/OgreRTShaderSystem.hh"

#define MINOR_VERSION 7

using namespace ignition;
using namespace rendering;

//////////////////////////////////////////////////
OgreRTShaderSystem::OgreRTShaderSystem()
{
  this->entityMutex = new std::mutex();
  this->initialized = false;
  this->shadowsApplied = false;
  this->pssmSetup.setNull();
}

//////////////////////////////////////////////////
OgreRTShaderSystem::~OgreRTShaderSystem()
{
  this->Fini();
  delete this->entityMutex;
}

//////////////////////////////////////////////////
void OgreRTShaderSystem::Init()
{
#if INCLUDE_RTSHADER && OGRE_VERSION_MAJOR >= 1 &&\
    OGRE_VERSION_MINOR >= MINOR_VERSION

  // Only initialize if using FORWARD rendering
  if (OgreRenderEngine::Instance()->RenderPathType() !=
      OgreRenderEngine::FORWARD)
  {
    return;
  }

  if (Ogre::RTShader::ShaderGenerator::initialize())
  {
    this->initialized = true;

    std::string coreLibsPath, cachePath;
    this->Paths(coreLibsPath, cachePath);

    // Get the shader generator pointer
    this->shaderGenerator = Ogre::RTShader::ShaderGenerator::getSingletonPtr();

    // Add the shader libs resource location
    Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
        coreLibsPath, "FileSystem");

    // Set shader cache path.
    this->shaderGenerator->setShaderCachePath(cachePath);

    this->shaderGenerator->setTargetLanguage("glsl");
  }
  else
    ignerr << "RT Shader system failed to initialize" << std::endl;

#endif
}

//////////////////////////////////////////////////
void OgreRTShaderSystem::Fini()
{
  if (!this->initialized)
    return;

  // Restore default scheme.
  Ogre::MaterialManager::getSingleton().setActiveScheme(
      Ogre::MaterialManager::DEFAULT_SCHEME_NAME);

  // Finalize RTShader system.
  if (this->shaderGenerator != nullptr)
  {
    // On Windows, we're using 1.9RC1, which doesn't have a bunch of changes.
#if (OGRE_VERSION < ((1 << 16) | (9 << 8) | 0)) || defined(_WIN32)
    Ogre::RTShader::ShaderGenerator::finalize();
#else
    Ogre::RTShader::ShaderGenerator::destroy();
#endif
    this->shaderGenerator = nullptr;
  }

  this->pssmSetup.setNull();
  this->entities.clear();
  this->scenes.clear();
  this->initialized = false;
}

//////////////////////////////////////////////////
#if INCLUDE_RTSHADER && OGRE_VERSION_MAJOR >= 1 &&\
    OGRE_VERSION_MINOR >= MINOR_VERSION
void OgreRTShaderSystem::AddScene(OgreScenePtr _scene)
{
  if (!this->initialized)
    return;

  // Set the scene manager
  this->shaderGenerator->addSceneManager(_scene->OgreSceneManager());
  this->shaderGenerator->createScheme(_scene->Name() +
      Ogre::RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME);
  this->scenes.push_back(_scene);
}
#else
void OgreRTShaderSystem::AddScene(OgreScenePtr /*_scene*/)
{
}
#endif

void OgreRTShaderSystem::RemoveScene(OgreScenePtr _scene)
{
  if (!this->initialized)
    return;

  std::vector<OgreScenePtr>::iterator iter;
  for (iter = this->scenes.begin(); iter != scenes.end(); ++iter)
    if ((*iter) == _scene)
      break;

  if (iter != this->scenes.end())
  {
    this->scenes.erase(iter);
    this->shaderGenerator->invalidateScheme(_scene->Name() +
        Ogre::RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME);
    this->shaderGenerator->removeSceneManager(_scene->OgreSceneManager());
    this->shaderGenerator->removeAllShaderBasedTechniques();
    this->shaderGenerator->flushShaderCache();
    // this->UpdateShaders();
  }
}

//////////////////////////////////////////////////
void OgreRTShaderSystem::RemoveScene(const std::string &_scene)
{
  if (!this->initialized)
    return;

  for (auto iter : this->scenes)
  {
    if (iter->Name() == _scene)
    {
      this->RemoveScene(iter);
      return;
    }
  }
}

//////////////////////////////////////////////////
void OgreRTShaderSystem::AttachEntity(OgreSubMesh *subMesh)
{
  if (!this->initialized)
    return;

  this->entityMutex->lock();
  this->entities.push_back(subMesh);
  this->entityMutex->unlock();
}

//////////////////////////////////////////////////
void OgreRTShaderSystem::DetachEntity(OgreSubMesh *_vis)
{
  if (!this->initialized)
    return;

  std::lock_guard<std::mutex> lock(*this->entityMutex);
  this->entities.remove(_vis);
}

//////////////////////////////////////////////////
void OgreRTShaderSystem::Clear()
{
  std::lock_guard<std::mutex> lock(*this->entityMutex);
  this->entities.clear();
}

//////////////////////////////////////////////////
void OgreRTShaderSystem::AttachViewport(Ogre::Viewport *_viewport,
    OgreScenePtr _scene)
{
  if (!OgreRTShaderSystem::Instance()->initialized)
    return;

#if OGRE_VERSION_MAJOR == 1 && OGRE_VERSION_MINOR >= 7
  _viewport->setMaterialScheme(_scene->Name() +
      Ogre::RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME);
#endif
}

//////////////////////////////////////////////////
void OgreRTShaderSystem::DetachViewport(Ogre::Viewport *_viewport,
    OgreScenePtr _scene)
{
  if (!OgreRTShaderSystem::Instance()->initialized)
    return;

#if OGRE_VERSION_MAJOR == 1 && OGRE_VERSION_MINOR >= 7
  if (_viewport && _scene && _scene->IsInitialized())
    _viewport->setMaterialScheme(_scene->Name());
#endif
}

//////////////////////////////////////////////////
void OgreRTShaderSystem::UpdateShaders()
{
  if (!this->initialized)
    return;

  std::list<OgreSubMesh*>::iterator iter;

  std::lock_guard<std::mutex> lock(*this->entityMutex);

  // Update all the shaders
  for (iter = this->entities.begin(); iter != this->entities.end(); ++iter)
    this->GenerateShaders(*iter);
}

//////////////////////////////////////////////////
void OgreRTShaderSystem::GenerateShaders(OgreSubMesh *subMesh)
{
  if (!this->initialized)
  {
    return;
  }

  Ogre::SubEntity* curSubEntity = subMesh->OgreSubEntity();

  OgreMaterialPtr material =
      std::dynamic_pointer_cast<OgreMaterial>(subMesh->Material());

  if (!material)
  {
    return;
  }

  std::string shaderTypeName = ShaderUtil::Name(material->ShaderType());
  std::string normalMapName = material->NormalMap();

  const Ogre::String& curMaterialName = curSubEntity->getMaterialName();
  bool success = false;

  for (unsigned int s = 0; s < this->scenes.size(); s++)
  {
    try
    {
      success = this->shaderGenerator->createShaderBasedTechnique(
          curMaterialName,
          Ogre::MaterialManager::DEFAULT_SCHEME_NAME,
          this->scenes[s]->Name() +
          Ogre::RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME);
    }
    catch(Ogre::Exception &e)
    {
      ignerr << "Unable to create shader technique for material["
        << curMaterialName << "]\n";
      success = false;
    }

    // Setup custom shader sub render states according to current setup.
    if (success)
    {
      // Grab the first pass render state.
      // NOTE:For more complicated samples iterate over the passes and build
      // each one of them as desired.
      Ogre::RTShader::RenderState* renderState =
        this->shaderGenerator->getRenderState(
            this->scenes[s]->Name() +
            Ogre::RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME,
            curMaterialName, 0);

      // Remove all sub render states.
      renderState->reset();

                if (shaderTypeName == "normal_map_object_space")
      {
        Ogre::RTShader::SubRenderState* subRenderState =
          this->shaderGenerator->createSubRenderState(
              Ogre::RTShader::NormalMapLighting::Type);

        Ogre::RTShader::NormalMapLighting* normalMapSubRS =
          static_cast<Ogre::RTShader::NormalMapLighting*>(subRenderState);

        normalMapSubRS->setNormalMapSpace(
            Ogre::RTShader::NormalMapLighting::NMS_OBJECT);

        normalMapSubRS->setNormalMapTextureName(normalMapName);
        renderState->addTemplateSubRenderState(normalMapSubRS);
      }
      else if (shaderTypeName == "normal_map_tangent_space")
      {
        Ogre::RTShader::SubRenderState* subRenderState =
          this->shaderGenerator->createSubRenderState(
              Ogre::RTShader::NormalMapLighting::Type);

        Ogre::RTShader::NormalMapLighting* normalMapSubRS =
          static_cast<Ogre::RTShader::NormalMapLighting*>(subRenderState);

        normalMapSubRS->setNormalMapSpace(
            Ogre::RTShader::NormalMapLighting::NMS_TANGENT);

        normalMapSubRS->setNormalMapTextureName(normalMapName);

        renderState->addTemplateSubRenderState(normalMapSubRS);
      }
      else if (shaderTypeName == "vertex")
      {
        Ogre::RTShader::SubRenderState *perPerVertexLightModel =
          this->shaderGenerator->createSubRenderState(
              Ogre::RTShader::FFPLighting::Type);

        renderState->addTemplateSubRenderState(perPerVertexLightModel);
      }
      else
      {
        Ogre::RTShader::SubRenderState *perPixelLightModel =
          this->shaderGenerator->createSubRenderState(
              Ogre::RTShader::PerPixelLighting::Type);

        renderState->addTemplateSubRenderState(perPixelLightModel);
      }


      // Invalidate this material in order to re-generate its shaders.
      this->shaderGenerator->invalidateMaterial(
          this->scenes[s]->Name() +
          Ogre::RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME,
          curMaterialName);
    }
  }
}

//////////////////////////////////////////////////
bool OgreRTShaderSystem::Paths(std::string &coreLibsPath,
    std::string &cachePath)
{
  if (!this->initialized)
    return false;

  Ogre::StringVector groupVector;

  // Setup the core libraries and shader cache path
  groupVector = Ogre::ResourceGroupManager::getSingleton().getResourceGroups();
  Ogre::StringVector::iterator itGroup = groupVector.begin();
  Ogre::StringVector::iterator itGroupEnd = groupVector.end();
  Ogre::String shaderCoreLibsPath;

  for (; itGroup != itGroupEnd; ++itGroup)
  {
    Ogre::ResourceGroupManager::LocationList resLocationsList;
    Ogre::ResourceGroupManager::LocationList::iterator it;
    Ogre::ResourceGroupManager::LocationList::iterator itEnd;
    bool coreLibsFound = false;

    resLocationsList =
      Ogre::ResourceGroupManager::getSingleton().getResourceLocationList(
          *itGroup);
    it = resLocationsList.begin();
    itEnd = resLocationsList.end();
    // Try to find the location of the core shader lib functions and use it
    // as shader cache path as well - this will reduce the number of
    // generated files when running from different directories.

    for (; it != itEnd; ++it)
    {
      struct stat st;
      if (stat((*it)->archive->getName().c_str(), &st) == 0)
      {
        if ((*it)->archive->getName().find("rtshaderlib") != Ogre::String::npos)
        {
          coreLibsPath = (*it)->archive->getName() + "/";

          // setup patch name for rt shader cache in tmp
          char *tmpdir;
          char *user;
          std::ostringstream stream;
          std::ostringstream errStream;
          // Get the tmp dir
          tmpdir = getenv("TMP");
          if (!tmpdir)
          {
            tmpdir = const_cast<char *>(
                boost::filesystem::temp_directory_path().string().c_str());
          }
          // Get the user
          user = getenv("USER");
          if (!user)
            user = const_cast<char*>("nobody");
          stream << tmpdir << "/ign-rendering-" << user
              << "-rtshaderlibcache" << "/";
          cachePath = stream.str();
          // Create the directory
#ifdef _WIN32
          if (mkdir(cachePath.c_str()) != 0)
#else
          if (mkdir(cachePath.c_str(), S_IRUSR | S_IWUSR | S_IXUSR) != 0)
#endif
          {
            if (errno != EEXIST)
            {
              errStream << "failed to create [" << cachePath << "] : ["
                <<  strerror(errno) << "]";
              throw(errStream.str());
            }
          }

          coreLibsFound = true;
          break;
        }
      }
    }

    // Core libs path found in the current group.
    if (coreLibsFound)
      break;
  }

  // Core shader lib not found -> shader generating will fail.
  if (coreLibsPath.empty())
  {
    ignerr << "Unable to find shader lib. Shader generating will fail."
      << std::endl;
    return false;
  }

  return true;
}

//////////////////////////////////////////////////
void OgreRTShaderSystem::RemoveShadows(OgreScenePtr _scene)
{
  if (!this->initialized || !this->shadowsApplied)
    return;

  _scene->OgreSceneManager()->setShadowTechnique(Ogre::SHADOWTYPE_NONE);

  _scene->OgreSceneManager()->setShadowCameraSetup(
      Ogre::ShadowCameraSetupPtr());

  Ogre::RTShader::RenderState* schemeRenderState =
    this->shaderGenerator->getRenderState(
        _scene->Name() +
        Ogre::RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME);

  schemeRenderState->removeTemplateSubRenderState(this->shadowRenderState);

  this->shaderGenerator->invalidateScheme(_scene->Name() +
      Ogre::RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME);
  this->UpdateShaders();

  this->shadowsApplied = false;
}

//////////////////////////////////////////////////
void OgreRTShaderSystem::ApplyShadows(OgreScenePtr _scene)
{
  if (!this->initialized || this->shadowsApplied)
    return;

  Ogre::SceneManager *sceneMgr = _scene->OgreSceneManager();

  // Grab the scheme render state.
  Ogre::RTShader::RenderState* schemRenderState =
    this->shaderGenerator->getRenderState(_scene->Name() +
        Ogre::RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME);

  sceneMgr->setShadowTechnique(Ogre::SHADOWTYPE_TEXTURE_ADDITIVE_INTEGRATED);

  // 3 textures per directional light
  sceneMgr->setShadowTextureCountPerLightType(Ogre::Light::LT_DIRECTIONAL, 3);
  sceneMgr->setShadowTextureCountPerLightType(Ogre::Light::LT_POINT, 0);
  sceneMgr->setShadowTextureCountPerLightType(Ogre::Light::LT_SPOTLIGHT, 0);
  sceneMgr->setShadowTextureCount(3);
  sceneMgr->setShadowTextureConfig(0, 1024, 1024, Ogre::PF_FLOAT32_R);
  sceneMgr->setShadowTextureConfig(1, 512, 512, Ogre::PF_FLOAT32_R);
  sceneMgr->setShadowTextureConfig(2, 512, 512, Ogre::PF_FLOAT32_R);
  sceneMgr->setShadowTextureSelfShadow(false);
  sceneMgr->setShadowCasterRenderBackFaces(true);

  // TODO: We have two different shadow caster materials, both taken from
  // OGRE samples. They should be compared and tested.
  // Set up caster material - this is just a standard depth/shadow map caster
  // sceneMgr->setShadowTextureCasterMaterial("PSSM/shadow_caster");
  sceneMgr->setShadowTextureCasterMaterial("Gazebo/shadow_caster");

  // Disable fog on the caster pass.
  //  Ogre::MaterialPtr passCaterMaterial =
  //   Ogre::MaterialManager::getSingleton().getByName("PSSM/shadow_caster");
  // Ogre::Pass* pssmCasterPass =
  // passCaterMaterial->getTechnique(0)->getPass(0);
  // pssmCasterPass->setFog(true);

  // shadow camera setup
  if (this->pssmSetup.isNull())
  {
    this->pssmSetup =
        Ogre::ShadowCameraSetupPtr(new Ogre::PSSMShadowCameraSetup());
  }

  double shadowFarDistance = 500;
  double cameraNearClip = 0.01;
  sceneMgr->setShadowFarDistance(shadowFarDistance);

  Ogre::PSSMShadowCameraSetup *cameraSetup =
      dynamic_cast<Ogre::PSSMShadowCameraSetup*>(this->pssmSetup.get());

  cameraSetup->calculateSplitPoints(3, cameraNearClip, shadowFarDistance);
  cameraSetup->setSplitPadding(4);
  cameraSetup->setOptimalAdjustFactor(0, 2);
  cameraSetup->setOptimalAdjustFactor(1, 1);
  cameraSetup->setOptimalAdjustFactor(2, .5);

  sceneMgr->setShadowCameraSetup(this->pssmSetup);

  // These values do not seem to help at all. Leaving here until I have time
  // to properly fix shadow z-fighting.
  // cameraSetup->setOptimalAdjustFactor(0, 4);
  // cameraSetup->setOptimalAdjustFactor(1, 1);
  // cameraSetup->setOptimalAdjustFactor(2, 0.5);

  this->shadowRenderState = this->shaderGenerator->createSubRenderState(
      Ogre::RTShader::IntegratedPSSM3::Type);
  Ogre::RTShader::IntegratedPSSM3 *pssm3SubRenderState =
    static_cast<Ogre::RTShader::IntegratedPSSM3*>(this->shadowRenderState);

  const Ogre::PSSMShadowCameraSetup::SplitPointList &srcSplitPoints =
    cameraSetup->getSplitPoints();

  Ogre::RTShader::IntegratedPSSM3::SplitPointList dstSplitPoints;

  for (unsigned int i = 0; i < srcSplitPoints.size(); ++i)
  {
    dstSplitPoints.push_back(srcSplitPoints[i]);
  }

  pssm3SubRenderState->setSplitPoints(dstSplitPoints);
  schemRenderState->addTemplateSubRenderState(this->shadowRenderState);

  this->shaderGenerator->invalidateScheme(_scene->Name() +
      Ogre::RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME);

  this->UpdateShaders();

  this->shadowsApplied = true;
}

//////////////////////////////////////////////////
Ogre::PSSMShadowCameraSetup
    *OgreRTShaderSystem::PSSMShadowCameraSetup() const
{
  return dynamic_cast<Ogre::PSSMShadowCameraSetup *>(this->pssmSetup.get());
}
