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

#include "Ogre2MemoryArchive.hh"

#include <cstring>

#ifdef _MSC_VER
  #pragma warning(push, 0)
#endif
#include <OgreDataStream.h>
#include <OgreString.h>
#ifdef _MSC_VER
  #pragma warning(pop)
#endif

using namespace gz;
using namespace rendering;

//////////////////////////////////////////////////
Ogre2MemoryArchive::Ogre2MemoryArchive(const std::string &_name)
  : Ogre::Archive(_name, "GzMemory")
{
}

//////////////////////////////////////////////////
void Ogre2MemoryArchive::SetFile(const std::string &_filename,
                                 const std::string &_contents)
{
  this->files[_filename] = _contents;
}

//////////////////////////////////////////////////
std::string Ogre2MemoryArchive::File(const std::string &_filename) const
{
  const auto it = this->files.find(_filename);
  return it == this->files.end() ? std::string() : it->second;
}

//////////////////////////////////////////////////
bool Ogre2MemoryArchive::isCaseSensitive() const
{
  return true;
}

//////////////////////////////////////////////////
void Ogre2MemoryArchive::load()
{
}

//////////////////////////////////////////////////
void Ogre2MemoryArchive::unload()
{
}

//////////////////////////////////////////////////
Ogre::DataStreamPtr Ogre2MemoryArchive::open(const Ogre::String &_filename,
                                             bool /*_readOnly*/)
{
  const auto it = this->files.find(_filename);
  if (it == this->files.end())
    return Ogre::DataStreamPtr();

  // A copy: the stream may outlive a change of the file
  auto *stream = OGRE_NEW Ogre::MemoryDataStream(_filename, it->second.size(),
      true, true);
  std::memcpy(stream->getPtr(), it->second.data(), it->second.size());
  return Ogre::DataStreamPtr(stream);
}

//////////////////////////////////////////////////
Ogre::StringVectorPtr Ogre2MemoryArchive::list(bool /*_recursive*/,
                                               bool _dirs)
{
  return this->find("*", true, _dirs);
}

//////////////////////////////////////////////////
Ogre::FileInfoListPtr Ogre2MemoryArchive::listFileInfo(bool /*_recursive*/,
                                                       bool _dirs)
{
  return this->findFileInfo("*", true, _dirs);
}

//////////////////////////////////////////////////
Ogre::StringVectorPtr Ogre2MemoryArchive::find(const Ogre::String &_pattern,
                                               bool /*_recursive*/,
                                               bool _dirs)
{
  Ogre::StringVectorPtr names(OGRE_NEW_T(Ogre::StringVector,
      Ogre::MEMCATEGORY_GENERAL)(), Ogre::SPFM_DELETE_T);
  if (_dirs)
    return names;
  for (const auto &file : this->files)
  {
    if (Ogre::StringUtil::match(file.first, _pattern, true))
      names->push_back(file.first);
  }
  return names;
}

//////////////////////////////////////////////////
bool Ogre2MemoryArchive::exists(const Ogre::String &_filename)
{
  return this->files.count(_filename) > 0;
}

//////////////////////////////////////////////////
time_t Ogre2MemoryArchive::getModifiedTime(const Ogre::String &/*_filename*/)
{
  return 0;
}

//////////////////////////////////////////////////
Ogre::FileInfoListPtr Ogre2MemoryArchive::findFileInfo(
    const Ogre::String &_pattern, bool /*_recursive*/, bool _dirs)
{
  Ogre::FileInfoListPtr infos(OGRE_NEW_T(Ogre::FileInfoList,
      Ogre::MEMCATEGORY_GENERAL)(), Ogre::SPFM_DELETE_T);
  if (_dirs)
    return infos;
  for (const auto &file : this->files)
  {
    if (!Ogre::StringUtil::match(file.first, _pattern, true))
      continue;
    Ogre::FileInfo info;
    info.archive = this;
    info.filename = file.first;
    info.basename = file.first;
    info.compressedSize = file.second.size();
    info.uncompressedSize = file.second.size();
    infos->push_back(info);
  }
  return infos;
}
