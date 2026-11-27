/*
 * Copyright (C) 2026 Open Source Robotics Foundation
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

#ifndef GZ_RENDERING_OGRE2_OGRE2MEMORYARCHIVE_HH_
#define GZ_RENDERING_OGRE2_OGRE2MEMORYARCHIVE_HH_

#include <map>
#include <string>

#include "gz/rendering/config.hh"
#include "gz/rendering/ogre2/Export.hh"

#ifdef _MSC_VER
  #pragma warning(push, 0)
#endif
#include <OgreArchive.h>
#ifdef _MSC_VER
  #pragma warning(pop)
#endif

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE
    {
    /// \brief A read only Ogre archive whose files are strings in memory,
    /// e.g. Hlms pieces generated at run time, so nothing is written to disk.
    /// It has no folders: every file is at its root.
    /// \internal
    class GZ_RENDERING_OGRE2_HIDDEN Ogre2MemoryArchive : public Ogre::Archive
    {
      /// \brief Constructor
      /// \param[in] _name Name of the archive, as Ogre reports it
      public: explicit Ogre2MemoryArchive(const std::string &_name);

      /// \brief Add a file, or replace its contents
      /// \param[in] _filename Name of the file
      /// \param[in] _contents Its contents
      public: void SetFile(const std::string &_filename,
                           const std::string &_contents);

      /// \brief Contents of a file
      /// \param[in] _filename Name of the file
      /// \return The contents, empty if there is no such file
      public: std::string File(const std::string &_filename) const;

      // Documentation inherited
      public: bool isCaseSensitive() const override;

      // Documentation inherited
      public: void load() override;

      // Documentation inherited
      public: void unload() override;

      // Documentation inherited
      public: Ogre::DataStreamPtr open(const Ogre::String &_filename,
                                       bool _readOnly = true) override;

      // Documentation inherited
      public: Ogre::StringVectorPtr list(bool _recursive = true,
                                         bool _dirs = false) override;

      // Documentation inherited
      public: Ogre::FileInfoListPtr listFileInfo(bool _recursive = true,
                                                 bool _dirs = false) override;

      // Documentation inherited
      public: Ogre::StringVectorPtr find(const Ogre::String &_pattern,
                                         bool _recursive = true,
                                         bool _dirs = false) override;

      // Documentation inherited
      public: bool exists(const Ogre::String &_filename) override;

      // Documentation inherited
      public: time_t getModifiedTime(const Ogre::String &_filename) override;

      // Documentation inherited
      public: Ogre::FileInfoListPtr findFileInfo(const Ogre::String &_pattern,
                                                 bool _recursive = true,
                                                 bool _dirs = false) override;

      /// \brief The files, by name
      private: std::map<std::string, std::string> files;
    };
    }
  }
}
#endif
