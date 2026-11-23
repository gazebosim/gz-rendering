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

#include "Ogre2MaterialSwitcherUtils.hh"

#include <cmath>

#include <gz/common/Console.hh>

#include "gz/rendering/ogre2/Ogre2Scene.hh"
#include "gz/rendering/Visual.hh"

#ifdef _MSC_VER
  #pragma warning(push, 0)
#endif
#include <OgreException.h>
#ifdef _MSC_VER
  #pragma warning(pop)
#endif

namespace gz
{
namespace rendering
{
inline namespace GZ_RENDERING_VERSION_NAMESPACE {

//////////////////////////////////////////////////
Ogre2FrustumPlanes CollectFrustumPlanes(const Ogre::Camera *_camera)
{
  Ogre2FrustumPlanes result;
  if (!_camera)
    return result;

  const Ogre::Plane *planes = _camera->getFrustumPlanes();
  for (int i = 0; i < 6; ++i)
  {
    if (i == Ogre::FRUSTUM_PLANE_FAR && !(_camera->getFarClipDistance() > 0))
      continue;
    result.planes[result.count++] = planes[i];
  }
  return result;
}

//////////////////////////////////////////////////
bool ItemInView(const Ogre::Item &_item, const Ogre2FrustumPlanes &_planes)
{
  if (!_item.isVisible())
    return false;

  const Ogre::Aabb aabb = _item.getWorldAabb();
  const Ogre::Vector3 &center = aabb.mCenter;
  const Ogre::Vector3 &halfSize = aabb.mHalfSize;
  // Infinite, empty or invalid boxes: let Ogre decide.
  if (!std::isfinite(halfSize.x) || !std::isfinite(halfSize.y) ||
      !std::isfinite(halfSize.z) || !std::isfinite(center.x) ||
      !std::isfinite(center.y) || !std::isfinite(center.z) ||
      halfSize.x < 0 || halfSize.y < 0 || halfSize.z < 0)
  {
    return true;
  }

  // Grow the box, so that only items clearly out of view are skipped.
  const Ogre::Vector3 grown = halfSize * 1.01f + Ogre::Vector3(1e-3f);
  // Same test as Ogre::Frustum::isVisible, without rebuilding the frustum
  // and the box for every item.
  for (std::size_t i = 0; i < _planes.count; ++i)
  {
    const Ogre::Plane &plane = _planes.planes[i];
    if (plane.normal.dotProduct(center) + plane.d <
        -plane.normal.absDotProduct(grown))
    {
      return false;
    }
  }
  return true;
}

//////////////////////////////////////////////////
bool ItemVisualId(const Ogre::Item &_item, unsigned int &_id)
{
  const Ogre::Any &userAny = _item.getUserObjectBindings().getUserAny();
  if (userAny.isEmpty() || userAny.getType() != typeid(unsigned int))
    return false;
  _id = Ogre::any_cast<unsigned int>(userAny);
  return true;
}

//////////////////////////////////////////////////
VisualPtr Ogre2ItemVisualCache::Lookup(Ogre2Scene &_scene,
    const Ogre::Item &_item, unsigned int _visualId)
{
  Entry &entry = this->entries[_item.getId()];
  entry.lastSeen = this->frame;
  if (entry.visualId == _visualId)
  {
    VisualPtr visual = entry.visual.lock();
    if (visual)
      return visual;
  }

  VisualPtr visual;
  try
  {
    visual = _scene.VisualById(_visualId);
  }
  catch (Ogre::Exception &e)
  {
    gzerr << "Ogre Error:" << e.getFullDescription() << "\n";
  }
  entry.visualId = _visualId;
  entry.visual = visual;
  return visual;
}

//////////////////////////////////////////////////
void Ogre2ItemVisualCache::EndFrame()
{
  ++this->frame;
  if (this->frame % kPruneInterval != 0)
    return;

  for (auto it = this->entries.begin(); it != this->entries.end();)
  {
    if (this->frame - it->second.lastSeen > kPruneInterval)
      it = this->entries.erase(it);
    else
      ++it;
  }
}

//////////////////////////////////////////////////
std::size_t Ogre2ItemVisualCache::Size() const
{
  return this->entries.size();
}
}
}  // namespace rendering
}  // namespace gz
