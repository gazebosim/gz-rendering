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

#include "Ogre2SharedFullscreenRect.hh"

#include "gz/rendering/ogre2/Ogre2RenderEngine.hh"

#ifdef _MSC_VER
  #pragma warning(push, 0)
#endif
#include <Compositor/OgreCompositorManager2.h>
#include <OgreRectangle2D.h>
#include <OgreRoot.h>
#ifdef _MSC_VER
  #pragma warning(pop)
#endif

namespace gz
{
namespace rendering
{
inline namespace GZ_RENDERING_VERSION_NAMESPACE {

//////////////////////////////////////////////////
void ReleaseFromSharedFullscreenRects(const Ogre::MaterialPtr &_material,
    const Ogre::MaterialPtr &_replacement)
{
  if (_material.isNull() || _replacement.isNull())
    return;

  Ogre::Root *root = Ogre2RenderEngine::Instance()->OgreRoot();
  if (!root)
    return;
  Ogre::CompositorManager2 *compositorManager = root->getCompositorManager2();
  if (!compositorManager)
    return;

  for (Ogre::v1::Rectangle2D *rect :
       {compositorManager->getSharedFullscreenTriangle(),
        compositorManager->getSharedFullscreenQuad()})
  {
    if (rect && rect->getMaterial() == _material)
    {
      // _material is still referenced here, so this doesn't destroy it
      _replacement->load();
      rect->setMaterial(_replacement);
    }
  }
}
}
}  // namespace rendering
}  // namespace gz
