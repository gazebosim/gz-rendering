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

#include <map>

#include <ignition/common/Console.hh>
#include <ignition/common/SystemPaths.hh>

#include <ignition/plugin/Loader.hh>

#include "ignition/rendering/config.hh"
#include "ignition/rendering/RenderEngine.hh"
#include "ignition/rendering/RenderEngineManager.hh"
#include "ignition/rendering/RenderEnginePlugin.hh"

/// \brief Private implementation of the RenderEngineManager class
class ignition::rendering::RenderEngineManagerPrivate
{
  /// \brief EngineMap that maps engine name to an engine pointer
  typedef std::map<std::string, RenderEngine *> EngineMap;

  /// \brief EngineMap iterator
  typedef EngineMap::iterator EngineIter;

  /// \brief Get a pointer to the render engine from an EngineMap iterator
  /// \param[in] _iter EngineMap iterator
  /// \param[in] _path Another search path for rendering engine plugin.
  public: RenderEngine *Engine(EngineIter _iter,
      const std::map<std::string, std::string> &_params,
      const std::string &_path);

  /// \brief Unload the given render engine from an EngineMap iterator
  /// \param[in] _iter EngineMap iterator
  /// \return True if the engine is unloaded
  public: bool UnloadEngine(EngineIter _iter);

  /// \brief Register default engines supplied by ign-rendering
  public: void RegisterDefaultEngines();

  /// \brief Unregister an engine using an EngineMap iterator
  /// \param[in] _iter EngineMap iterator
  public: void UnregisterEngine(EngineIter _iter);

  /// \brief Load a render engine plugin
  /// \param[in] _filename Filename of plugin shared library
  /// \param[in] _path Another search path for rendering engine plugin.
  /// \return True if the plugin is loaded successfully
  public: bool LoadEnginePlugin(const std::string &_filename,
              const std::string &_path);

  /// \brief Unload a render engine plugin
  /// \param[in] _engineName Name of engine associated with this plugin
  /// \return True if the plugin is unloaded successfully
  public: bool UnloadEnginePlugin(const std::string &_engineName);

  /// \brief Engines that have been registered
  public: EngineMap engines;

  /// \brief A map of default engine name to its plugin library name
  public: std::map<std::string, std::string> defaultEngines;

  /// \brief A map of loaded engine plugins to its plugin name
  public: std::map<std::string, std::string> enginePlugins;

  /// \brief Plugin loader for managing render engine plugin libraries
  public: ignition::plugin::Loader pluginLoader;
};

using namespace ignition;
using namespace rendering;

//////////////////////////////////////////////////
// RenderEngineManager
//////////////////////////////////////////////////
RenderEngineManager::RenderEngineManager() :
  dataPtr(new RenderEngineManagerPrivate)
{
  this->dataPtr->RegisterDefaultEngines();
}

//////////////////////////////////////////////////
RenderEngineManager::~RenderEngineManager()
{
}

//////////////////////////////////////////////////
unsigned int RenderEngineManager::EngineCount() const
{
  return this->dataPtr->engines.size();
}

//////////////////////////////////////////////////
bool RenderEngineManager::HasEngine(const std::string &_name) const
{
  auto iter = this->dataPtr->engines.find(_name);
  return iter != this->dataPtr->engines.end();
}

//////////////////////////////////////////////////
RenderEngine *RenderEngineManager::Engine(const std::string &_name,
    const std::map<std::string, std::string> &_params,
    const std::string &_path)
{
  // check in the list of available engines
  auto iter = this->dataPtr->engines.find(_name);

  if (iter == this->dataPtr->engines.end())
  {
    ignerr << "No render-engine registered with name: " << _name << std::endl;
    return nullptr;
  }

  return this->dataPtr->Engine(iter, _params, _path);
}

//////////////////////////////////////////////////
RenderEngine *RenderEngineManager::EngineAt(unsigned int _index,
    const std::map<std::string, std::string> &_params,
    const std::string &_path)
{
  if (_index >= this->EngineCount())
  {
    ignerr << "Invalid render-engine index: " << _index << std::endl;
    return nullptr;
  }

  auto iter = this->dataPtr->engines.begin();
  std::advance(iter, _index);
  return this->dataPtr->Engine(iter, _params, _path);
}

//////////////////////////////////////////////////
bool RenderEngineManager::UnloadEngine(const std::string &_name)
{
  // check in the list of available engines
  auto iter = this->dataPtr->engines.find(_name);

  if (iter == this->dataPtr->engines.end())
  {
    ignerr << "No render-engine registered with name: " << _name << std::endl;
    return false;
  }

  return this->dataPtr->UnloadEngine(iter);
}

//////////////////////////////////////////////////
bool RenderEngineManager::UnloadEngineAt(unsigned int _index)
{
  if (_index >= this->EngineCount())
  {
    ignerr << "Invalid render-engine index: " << _index << std::endl;
    return false;
  }

  auto iter = this->dataPtr->engines.begin();
  std::advance(iter, _index);
  return this->dataPtr->UnloadEngine(iter);
}

//////////////////////////////////////////////////
void RenderEngineManager::RegisterEngine(const std::string &_name,
    RenderEngine *_engine)
{
  if (!_engine)
  {
    ignerr << "Render-engine cannot be null" << std::endl;
    return;
  }

  if (this->HasEngine(_name))
  {
    ignerr << "Render-engine already registered with name: "
          << _name << std::endl;

    return;
  }

  this->dataPtr->engines[_name] = _engine;
}

//////////////////////////////////////////////////
void RenderEngineManager::UnregisterEngine(const std::string &_name)
{
  auto iter = this->dataPtr->engines.find(_name);

  if (iter != this->dataPtr->engines.end())
  {
    this->dataPtr->UnregisterEngine(iter);
  }
}

//////////////////////////////////////////////////
void RenderEngineManager::UnregisterEngine(RenderEngine *_engine)
{
  if (!_engine)
    return;

  auto begin = this->dataPtr->engines.begin();
  auto end = this->dataPtr->engines.end();

  for (auto iter = begin; iter != end; ++iter)
  {
    if (iter->second == _engine)
    {
      this->dataPtr->UnregisterEngine(iter);
      return;
    }
  }
}

//////////////////////////////////////////////////
void RenderEngineManager::UnregisterEngineAt(unsigned int _index)
{
  if (_index >= this->EngineCount())
  {
    ignerr << "Invalid render-engine index: " << _index << std::endl;
    return;
  }

  auto iter = this->dataPtr->engines.begin();
  std::advance(iter, _index);
  this->dataPtr->UnregisterEngine(iter);
}

//////////////////////////////////////////////////
// RenderEngineManagerPrivate
//////////////////////////////////////////////////
RenderEngine *RenderEngineManagerPrivate::Engine(EngineIter _iter,
    const std::map<std::string, std::string> &_params,
    const std::string &_path)
{
  RenderEngine *engine = _iter->second;

  if (!engine)
  {
    // check if it's an engine in the list of default engines provided by
    // ign-rendering. If so, load it
    auto defaultIt = this->defaultEngines.find(_iter->first);
    if (defaultIt != this->defaultEngines.end())
    {
      std::string libName = defaultIt->second;
      if (this->LoadEnginePlugin(libName, _path))
      {
        auto engineIt = this->engines.find(_iter->first);
        if (engineIt != this->engines.end())
          engine = engineIt->second;
      }
    }
  }

  if (!engine)
    return nullptr;

  if (!engine->IsInitialized())
  {
    engine->Load(_params);
    engine->Init();
  }

  return engine;
}

//////////////////////////////////////////////////
bool RenderEngineManagerPrivate::UnloadEngine(EngineIter _iter)
{
  RenderEngine *engine = _iter->second;

  if (!engine)
    return false;

  engine->Destroy();

  return this->UnloadEnginePlugin(engine->Name());
}

//////////////////////////////////////////////////
void RenderEngineManagerPrivate::RegisterDefaultEngines()
{
  // TODO(anyone): Find a cleaner way to get the default engine library name

  // cppcheck-suppress unreadVariable
  std::string libName = "ignition-rendering" +
    std::to_string(IGNITION_RENDERING_MAJOR_VERSION) + "-";

  // cppcheck-suppress unusedVariable
  std::string engineName;

#if HAVE_OGRE
  engineName = "ogre";
  this->defaultEngines[engineName] = libName + engineName;
  if (this->engines.find(engineName) == this->engines.end())
    this->engines[engineName] = nullptr;
#endif
#if HAVE_OGRE2
  engineName = "ogre2";
  this->defaultEngines[engineName] = libName + engineName;
  if (this->engines.find(engineName) == this->engines.end())
    this->engines[engineName] = nullptr;
#endif
#if HAVE_OPTIX
  engineName = "optix";
  this->defaultEngines[engineName] = libName + engineName;
  if (this->engines.find(engineName) == this->engines.end())
    this->engines[engineName] = nullptr;
#endif
}

//////////////////////////////////////////////////
bool RenderEngineManagerPrivate::LoadEnginePlugin(
    const std::string &_filename, const std::string &_path)
{
  ignmsg << "Loading plugin [" << _filename << "]" << std::endl;

  ignition::common::SystemPaths systemPaths;

  // Add default install folder.
  systemPaths.AddPluginPaths(std::string(IGN_RENDERING_PLUGIN_PATH));
  // Add extra search path.
  systemPaths.AddPluginPaths(_path);

  auto pathToLib = systemPaths.FindSharedLibrary(_filename);
  if (pathToLib.empty())
  {
    ignerr << "Failed to load plugin [" << _filename <<
              "] : couldn't find shared library." << std::endl;
    return false;
  }

  // Load plugin
  auto pluginNames = this->pluginLoader.LoadLibrary(pathToLib);
  if (pluginNames.empty())
  {
    ignerr << "Failed to load plugin [" << _filename <<
              "] : couldn't load library on path [" << pathToLib <<
              "]." << std::endl;
    return false;
  }

  std::string pluginName = *pluginNames.begin();
  if (pluginName.empty())
  {
    ignerr << "Failed to load plugin [" << _filename <<
              "] : couldn't load library on path [" << pathToLib <<
              "]." << std::endl;
    return false;
  }

  auto commonPlugin = this->pluginLoader.Instantiate(pluginName);
  if (!commonPlugin)
  {
    ignerr << "Failed to load plugin [" << _filename <<
              "] : couldn't instantiate plugin on path [" << pathToLib <<
              "]." << std::endl;
    return false;
  }

  auto plugin =
      commonPlugin->QueryInterface<ignition::rendering::RenderEnginePlugin>();
  if (!plugin)
  {
    ignerr << "Failed to load plugin [" << _filename <<
              "] : couldn't get interface [" << pluginName <<
              "]." << std::endl;
    return false;
  }

  // this triggers the engine to be instantiated
  std::string engineName = plugin->Name();
  this->engines[engineName] = plugin->Engine();

  // store engine plugin data so plugin can be unloaded later
  this->enginePlugins[engineName] = pluginName;

  return true;
}

//////////////////////////////////////////////////
bool RenderEngineManagerPrivate::UnloadEnginePlugin(
    const std::string &_engineName)
{
  auto it = this->enginePlugins.find(_engineName);
  if (it == this->enginePlugins.end())
  {
    ignmsg << "Skip unloading engine plugin. [" << _engineName << "] "
           << "not loaded from plugin." << std::endl;
    return false;
  }

  std::string pluginName = it->second;
  this->enginePlugins.erase(it);

  if (!this->pluginLoader.ForgetLibraryOfPlugin(pluginName))
  {
    ignerr << "Failed to unload plugin: " << pluginName << std::endl;
  }

  auto engineIt = this->engines.find(_engineName);
  if (engineIt == this->engines.end())
    return false;

  // set to null - this means engine is still registered but not loaded
  this->engines[_engineName] = nullptr;

  return true;
}

//////////////////////////////////////////////////
void RenderEngineManagerPrivate::UnregisterEngine(EngineIter _iter)
{
  _iter->second->Destroy();
  this->engines.erase(_iter);
}
