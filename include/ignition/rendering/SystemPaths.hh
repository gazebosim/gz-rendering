/*
 * Copyright (C) 2017 Open Source Robotics Foundation
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
#ifndef IGNITION_RENDERING_SYSTEMPATHS_HH_
#define IGNITION_RENDERING_SYSTEMPATHS_HH_

#define LINUX
#ifdef _WIN32
  #include <direct.h>
  #define GetCurrentDir _getcwd
#else
  #include <unistd.h>
  #define GetCurrentDir getcwd
#endif
#include <stdio.h>

#include <list>
#include <memory>
#include <string>

#include <ignition/common/SingletonT.hh>
#include "ignition/rendering/Util.hh"

namespace ignition
{
  namespace rendering
  {
    // Forward declare private data class
    class SystemPathsPrivate;

    /// \class SystemPaths SystemPaths.hh
    /// \brief Functions to handle getting system paths, keeps track of:
    ///        \li SystemPaths#ogrePaths - ogre library paths.
    ///            Should point to Ogre RenderSystem_GL.so et. al.
    ///        \li SystemPaths#pluginPaths - plugin library paths
    class IGNITION_VISIBLE SystemPaths : public common::SingletonT<SystemPaths>
    {
      /// Constructor for SystemPaths
      private: SystemPaths();

      /// Destructor for SystemPaths
      private: ~SystemPaths();

      /// \brief Get the log path
      /// \return the path
      public: std::string LogPath() const;

      /// \brief Get the ogre install paths
      /// \return a list of paths
      public: const std::list<std::string> &OgrePaths();

      /// \brief Get the plugin paths
      /// \return a list of paths
      public: const std::list<std::string> &PluginPaths();

      /// \brief Find a file or path using a URI
      /// \param[in] _uri the uniform resource identifier
      /// \return Returns full path name to file
      public: std::string FindFileURI(const std::string &_uri);

      /// \brief Find a file in the gazebo paths
      /// \param[in] _filename Name of the file to find.
      /// \param[in] _searchLocalPath True to search in the current working
      /// directory.
      /// \return Returns full path name to file
      public: std::string FindFile(const std::string &_filename,
                                   bool _searchLocalPath = true);

      /// \brief Add colon delimited paths to ogre install
      /// \param[in] _path of the directory to add
      public: void AddOgrePaths(const std::string &_path);

      /// \brief Add colon delimited paths to plugins
      /// \param[in] _path of the directory to add
      public: void AddPluginPaths(const std::string &_path);

      /// \brief Add colon delimited paths to resources
      /// \param[in] _path of the directory to add
      public: void AddResourcePaths(const std::string &_path);

      /// \brief clear out ogre paths
      public: void ClearOgrePaths();

      /// \brief clear out plugin paths
      public: void ClearPluginPaths();

      /// \brief clear out resource paths
      public: void ClearResourcePaths();

      /// \brief add _suffix to the list of path search suffixes
      /// \param[in] _suffix The suffix to add
      public: void AddSearchPathSuffix(const std::string &_suffix);


      private: friend class SingletonT<SystemPaths>;

      /// \brief Private data pointer
      private: std::unique_ptr<SystemPathsPrivate> dataPtr;
    };
  }
}
#endif
