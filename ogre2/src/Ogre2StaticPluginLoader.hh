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
#ifndef GZ_RENDERING_OGRE2_OGRE2STATICPLUGINLOADER_HH_
#define GZ_RENDERING_OGRE2_OGRE2STATICPLUGINLOADER_HH_

#include "gz/rendering/ogre2/Export.hh"

#include <gz/utils/ImplPtr.hh>

#include <vector>

namespace Ogre
{
class Root;
}  // namespace Ogre

namespace gz::rendering
{

class GZ_RENDERING_OGRE2_VISIBLE Ogre2StaticPluginLoader
{
public: Ogre2StaticPluginLoader();

public: void Install(Ogre::Root *_root);

GZ_UTILS_UNIQUE_IMPL_PTR(dataPtr);
};
}  // namespace gz::rendering

#endif  // GZ_RENDERING_OGRE2_OGRE2STATICPLUGINLOADER_HH_
