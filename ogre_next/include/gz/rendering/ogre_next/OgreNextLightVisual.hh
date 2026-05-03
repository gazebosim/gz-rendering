/*
 * Copyright (C) 2021 Open Source Robotics Foundation
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
#ifndef GZ_RENDERING_OGRE_NEXT_OGRE_NEXTLIGHTVISUAL_HH_
#define GZ_RENDERING_OGRE_NEXT_OGRE_NEXTLIGHTVISUAL_HH_

#include <memory>

#include "gz/rendering/base/BaseLightVisual.hh"
#include "gz/rendering/ogre_next/OgreNextVisual.hh"

namespace Ogre
{
  class MovableObject;
}

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {

    // Forward declaration
    class OgreNextLightVisualPrivate;

    /// \brief OgreNext.x implementation of the light visual class
    class GZ_RENDERING_OGRE_NEXT_VISIBLE OgreNextLightVisual :
      public BaseLightVisual<OgreNextVisual>
    {
      /// \brief Constructor
      protected: OgreNextLightVisual();

      /// \brief Destructor
      public: virtual ~OgreNextLightVisual();

      // Documentation inherited.
      public: virtual void Init() override;

      // Documentation inherited.
      public: virtual void PreRender() override;

      // Documentation inherited.
      public: Ogre::MovableObject *OgreObject() const;

      /// \brief Create the Light Visual in OgreNext
      public: void CreateVisual();

      // Documentation inherited.
      public: virtual MaterialPtr Material() const override;

      // Documentation inherited.
      public: virtual void SetMaterial(
        MaterialPtr _material, bool _unique) override;

      /// \brief Set material to grid geometry.
      /// \param[in] _material Ogre material.
      protected: virtual void SetMaterialImpl(OgreNextMaterialPtr _material);

      /// \brief Only scene can instantiate an arrow visual
      private: friend class OgreNextScene;

      /// \brief Private data class
      private: std::unique_ptr<OgreNextLightVisualPrivate> dataPtr;
    };
    }
  }
}
#endif
