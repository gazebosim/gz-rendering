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
#ifndef GZ_RENDERING_OGRE_OGRERESOURCEPATH_HH_
#define GZ_RENDERING_OGRE_OGRERESOURCEPATH_HH_

// Internal-only header (not installed).

#include <filesystem>
#include <string>
#include <system_error>

#include <gz/common/Filesystem.hh>

#include "gz/rendering/config.hh"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    /// \brief Name to pass to the OGRE resource system for a resource that
    /// can be given as a file path. A name that refers to an existing file,
    /// absolute or relative to the working directory, becomes an absolute
    /// path, which the AbsolutePathLoadingListener installed by
    /// OgreRenderEngine opens directly. Any other name is returned unchanged
    /// and is looked up in the resource groups. The path is not normalized:
    /// removing ".." without resolving the symlinks before it can name a
    /// different file, so ".." is left to the file system.
    /// \param[in] _name File path or resource name
    /// \return Absolute file path, or _name
    inline std::string OgreResourcePath(const std::string &_name)
    {
      if (_name.empty() || !common::isFile(_name))
        return _name;

      std::error_code ec;
      const std::filesystem::path path = std::filesystem::absolute(_name, ec);
      return ec ? _name : path.string();
    }
    }
  }
}
#endif
