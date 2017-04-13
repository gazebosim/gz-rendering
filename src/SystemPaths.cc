/*
 * Copyright (C) 2017 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <sys/stat.h>
#include <sys/types.h>

// See below for Windows dirent include. cpplint complains about system
// header order if "win_dirent.h" is in the wrong location.
#ifndef _WIN32
  #include <dirent.h>
#else
  #include <ignition/common/win_dirent.h>
#endif

#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>

#include "ignition/common/Console.hh"
#include "ignition/rendering/SystemPaths.hh"

using namespace ignition;
using namespace rendering;

#ifdef _WIN32
static const std::string PathDelimiter = ";";
#else
static const std::string PathDelimiter = ":";
#endif

// Private data class
class ignition::rendering::SystemPathsPrivate
{
  /// \brief re-read SystemPaths#pluginPaths from environment variable
  public: void UpdatePluginPaths();

  /// \brief re-read SystemPaths#ogrePaths from environment variable
  public: void UpdateOgrePaths();

  /// \brief adds a path to the provided list
  /// \param[in]_path the path
  /// \param[in]_list the list
  public: void AddPaths(const std::string &_path,
                        std::list<std::string> &_list);

   /// \brief adds a path to the list if not already present
   /// \param[in]_path the path
   /// \param[in]_list the list
   private: void InsertUnique(const std::string &_path,
                              std::list<std::string> &_list);

  /// \brief if true, call UpdatePluginPaths() within PluginPaths()
  public: bool pluginPathsFromEnv;

  /// \brief Path to ogre plugins
  public: std::list<std::string> ogrePaths;

  /// \brief Paths to plugins
  public: std::list<std::string> pluginPaths;

  /// \brief Paths to resources
  public: std::list<std::string> resourcePaths;

  /// \brief Suffix paths
  public: std::list<std::string> suffixPaths;

  /// \brief Log path
  public: std::string logPath;

  /// \brief if true, call UpdateOgrePaths() within OgrePaths()
  public: bool ogrePathsFromEnv;

  /// \brief Find file callback.
  public: std::function<std::string(const std::string &)> findFileCB;

  /// \brief Find file URI callback.
  public: std::function<std::string(const std::string &)> findFileURICB;
};


//////////////////////////////////////////////////
SystemPaths::SystemPaths()
  : dataPtr(new SystemPathsPrivate)
{
  this->dataPtr->ogrePaths.clear();
  this->dataPtr->pluginPaths.clear();

  char *homePath = getenv("HOME");
  std::string home;
  if (!homePath)
    home = "/tmp/ign-rendering";
  else
    home = homePath;

  char *path = getenv("IGN_RENDERING_LOG_PATH");
  std::string fullPath;
  if (!path)
  {
    if (home != "/tmp/ign-rendering")
      fullPath = home + "/.ign-rendering";
    else
      fullPath = home;
  }
  else
    fullPath = path;

  DIR *dir = opendir(fullPath.c_str());
  if (!dir)
  {
#ifdef _WIN32
    _mkdir(fullPath.c_str());
#else
    mkdir(fullPath.c_str(), S_IRWXU | S_IRGRP | S_IROTH);
#endif
  }
  else
    closedir(dir);

  this->dataPtr->logPath = fullPath;

  this->dataPtr->UpdatePluginPaths();
  this->dataPtr->UpdateOgrePaths();

  this->dataPtr->pluginPathsFromEnv = true;
  this->dataPtr->ogrePathsFromEnv = true;
}

/////////////////////////////////////////////////
SystemPaths::~SystemPaths()
{
}

/////////////////////////////////////////////////
std::string SystemPaths::LogPath() const
{
  return this->dataPtr->logPath;
}

/////////////////////////////////////////////////
const std::list<std::string> &SystemPaths::PluginPaths()
{
  if (this->dataPtr->pluginPathsFromEnv)
    this->dataPtr->UpdatePluginPaths();
  return this->dataPtr->pluginPaths;
}

/////////////////////////////////////////////////
const std::list<std::string> &SystemPaths::OgrePaths()
{
  if (this->dataPtr->ogrePathsFromEnv)
    this->dataPtr->UpdateOgrePaths();
  return this->dataPtr->ogrePaths;
}


//////////////////////////////////////////////////
std::string SystemPaths::FindFileURI(const std::string &_uri)
{
  int index = _uri.find("://");
  std::string prefix = _uri.substr(0, index);
  std::string suffix = _uri.substr(index + 3, _uri.size() - index - 3);
  std::string filename;

  if (prefix.empty() || prefix == "file")
  {
    // First try to find the file on the current system
    filename = this->FindFile(suffix);
  }
  else
  {
    filename = this->dataPtr->findFileURICB(_uri);
  }

  if (filename.empty())
    ignerr << "Unable to find file with URI [" << _uri << "]\n";

  return filename;
}

//////////////////////////////////////////////////
std::string SystemPaths::FindFile(const std::string &_filename,
                                  bool _searchLocalPath)
{
  std::string path;

  if (_filename.empty())
    return path;

  if (_filename.find("://") != std::string::npos)
  {
    path = this->FindFileURI(_filename);
  }
  else if (_filename[0] == '/')
  {
    path = _filename;
  }
  else
  {
    bool found = false;

    path = common::cwd() + "/" + _filename;

    if (_searchLocalPath && common::exists(path))
    {
      found = true;
    }
    else if ((_filename[0] == '/' || _filename[0] == '.' || _searchLocalPath)
             && common::exists(_filename))
    {
      path = _filename;
      found = true;
    }
    else
    {
      path = this->dataPtr->findFileCB(_filename);
      found = !path.empty();
    }

    if (!found)
      return std::string();
  }

  if (!common::exists(path))
  {
    ignerr << "File or path does not exist[" << path << "]\n";
    return std::string();
  }

  return path;
}


/////////////////////////////////////////////////
void SystemPaths::ClearOgrePaths()
{
  this->dataPtr->ogrePaths.clear();
}

/////////////////////////////////////////////////
void SystemPaths::ClearPluginPaths()
{
  this->dataPtr->pluginPaths.clear();
}

/////////////////////////////////////////////////
void SystemPaths::ClearResourcePaths()
{
  this->dataPtr->resourcePaths.clear();
}

/////////////////////////////////////////////////
void SystemPaths::AddOgrePaths(const std::string &_path)
{
  this->dataPtr->AddPaths(_path, this->dataPtr->ogrePaths);
}

/////////////////////////////////////////////////
void SystemPaths::AddPluginPaths(const std::string &_path)
{
  this->dataPtr->AddPaths(_path, this->dataPtr->pluginPaths);
}

/////////////////////////////////////////////////
void SystemPaths::AddResourcePaths(const std::string &_path)
{
  this->dataPtr->AddPaths(_path, this->dataPtr->resourcePaths);
}

/////////////////////////////////////////////////
void SystemPaths::AddSearchPathSuffix(const std::string &_suffix)
{
  std::string s;

  if (_suffix[0] != '/')
    s = std::string("/") + _suffix;
  else
    s = _suffix;

  if (_suffix[_suffix.size()-1] != '/')
    s += "/";

  this->dataPtr->suffixPaths.push_back(s);
}

//////////////////////////////////////////////////
void SystemPathsPrivate::UpdatePluginPaths()
{
  std::string path;

  char *pathCStr = getenv("IGN_RENDERING_PLUGIN_PATH");
  if (!pathCStr || *pathCStr == '\0')
  {
    // No env var; take the compile-time default.
    path = IGN_RENDERING_PLUGIN_PATH;
  }
  else
    path = pathCStr;

  size_t pos1 = 0;
  size_t pos2 = path.find(PathDelimiter);
  while (pos2 != std::string::npos)
  {
    this->InsertUnique(path.substr(pos1, pos2-pos1),
        this->pluginPaths);
    pos1 = pos2+1;
    pos2 = path.find(PathDelimiter, pos2+1);
  }
  this->InsertUnique(path.substr(pos1, path.size()-pos1),
      this->pluginPaths);
}

//////////////////////////////////////////////////
void SystemPathsPrivate::UpdateOgrePaths()
{
  std::string path;

  char *pathCStr = getenv("OGRE_RESOURCE_PATH");
  if (!pathCStr || *pathCStr == '\0')
  {
    // No env var; take the compile-time default.
    path = OGRE_RESOURCE_PATH;
  }
  else
    path = pathCStr;

  size_t pos1 = 0;
  size_t pos2 = path.find(PathDelimiter);
  while (pos2 != std::string::npos)
  {
    this->InsertUnique(path.substr(pos1, pos2-pos1), this->ogrePaths);
    pos1 = pos2+1;
    pos2 = path.find(PathDelimiter, pos2+1);
  }
  this->InsertUnique(path.substr(pos1, path.size()-pos1),
      this->ogrePaths);
}

/////////////////////////////////////////////////
void SystemPathsPrivate::AddPaths(const std::string &_path,
    std::list<std::string> &_list)
{
  size_t pos1 = 0;
  size_t pos2 = _path.find(PathDelimiter);
  while (pos2 != std::string::npos)
  {
    this->InsertUnique(_path.substr(pos1, pos2-pos1), _list);
    pos1 = pos2+1;
    pos2 = _path.find(PathDelimiter, pos2+1);
  }
  this->InsertUnique(_path.substr(pos1, _path.size()-pos1), _list);
}

/////////////////////////////////////////////////
void SystemPathsPrivate::InsertUnique(const std::string &_path,
                               std::list<std::string> &_list)
{
  if (std::find(_list.begin(), _list.end(), _path) == _list.end())
    _list.push_back(_path);
}


