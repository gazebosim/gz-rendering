/*
 * Copyright (C) 2023 Open Source Robotics Foundation
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

#include "Ogre2StaticPluginLoader.hh"

#include "gz/rendering/RenderPassSystem.hh"
#include "gz/rendering/ogre2/Ogre2GaussianNoisePass.hh"

#include "OgreRoot.h"

#if OGRE2_STATIC_LIB
  #ifdef OGRE2_HAVE_OPENGL
    #include "OgreGL3PlusPlugin.h"
  #endif
  #ifdef OGRE2_HAVE_VULKAN
    #include "OgreVulkanPlugin.h"
  #endif
  #include "OgreParticleFXPlugin.h"
#endif

#if OGRE2_STATIC_LIB
class GaussianNoiseFactory: public gz::rendering::RenderPassFactory
{
  public: GaussianNoiseFactory() {
    gz::rendering::RenderPassSystem::Register(typeid(gz::rendering::GaussianNoisePass).name(), this);
  }

  public: gz::rendering::RenderPass *New() const override {
      return new gz::rendering::Ogre2GaussianNoisePass();
  }
};
#endif  // OGRE2_STATIC_LIB

namespace gz::rendering
{

class Ogre2StaticPluginLoader::Implementation
{
#ifdef OGRE2_STATIC_LIB
#ifdef OGRE2_HAVE_OPENGL
  public: Ogre::GL3PlusPlugin* gl3_plugin = {nullptr};
#endif
#ifdef OGRE2_HAVE_VULKAN
  public: Ogre::VulkanPlugin * vulkan_plugin = {nullptr};
#endif

  public: Ogre::ParticleFXPlugin * particle_plugin = {nullptr};

  public: GaussianNoiseFactory gaussian_noise_factory;

#endif

  public: ~Implementation()
  {
#ifdef OGRE2_STATIC_LIB
#ifdef OGRE2_HAVE_OPENGL
  delete this->gl3_plugin;
#endif
#ifdef OGRE2_HAVE_VULKAN
  delete this->vulkan_plugin;
#endif
  delete this->particle_plugin;
#endif
  }
};

Ogre2StaticPluginLoader::Ogre2StaticPluginLoader()
  : dataPtr(gz::utils::MakeUniqueImpl<Implementation>())
{
}

void Ogre2StaticPluginLoader::Install(Ogre::Root *_root)
{
#ifdef OGRE2_STATIC_LIB
#ifdef OGRE2_HAVE_OPENGL
  if (!this->dataPtr->gl3_plugin)
  {
    this->dataPtr->gl3_plugin = new Ogre::GL3PlusPlugin();
  }
  _root->installPlugin(this->dataPtr->gl3_plugin, nullptr);
#endif
#ifdef OGRE2_HAVE_VULKAN
  if (!this->dataPtr->vulkan_plugin)
  {
    this->dataPtr->vulkan_plugin = new Ogre::VulkanPlugin();
  }
  _root->installPlugin(this->dataPtr->vulkan_plugin, nullptr);
#endif
  if (!this->dataPtr->particle_plugin)
  {
    this->dataPtr->particle_plugin = new Ogre::ParticleFXPlugin();
  }
  _root->installPlugin(this->dataPtr->particle_plugin, nullptr);
#endif
}

}  // namespace gz::rendering
