/*
 * Copyright (C) 2018 Open Source Robotics Foundation
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
#ifndef IGNITION_RENDERING_OGRE2_OGRE2VISUAL_HH_
#define IGNITION_RENDERING_OGRE2_OGRE2VISUAL_HH_

#include "ignition/rendering/base/BaseVisual.hh"
#include "ignition/rendering/ogre2/Ogre2Node.hh"
#include "ignition/rendering/ogre2/Ogre2RenderTypes.hh"

namespace ignition
{
  namespace rendering
  {
    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
    //
    class IGNITION_RENDERING_OGRE2_VISIBLE Ogre2Visual :
      public BaseVisual<Ogre2Node>
    {
      protected: Ogre2Visual();

      public: virtual ~Ogre2Visual();

      public: virtual math::Vector3d LocalScale() const;

      public: virtual bool InheritScale() const;

      public: virtual void SetInheritScale(bool _inherit);

      protected: virtual NodeStorePtr Children() const;

      protected: virtual GeometryStorePtr Geometries() const;

      protected: virtual bool AttachChild(NodePtr _child);

      protected: virtual bool DetachChild(NodePtr _child);

      protected: virtual bool AttachGeometry(GeometryPtr _geometry);

      protected: virtual bool DetachGeometry(GeometryPtr _geometry);

      protected: virtual void SetLocalScaleImpl(
                     const math::Vector3d &_scale);

      protected: virtual void Init();

      protected: Ogre2NodeStorePtr children;

      protected: Ogre2GeometryStorePtr geometries;

      private: Ogre2VisualPtr SharedThis();

      private: friend class Ogre2Scene;
    };
    }
  }
}
#endif
