/*
 * Copyright (C) 2026 cellumation GmbH
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
#ifndef GZ_RENDERING_OGRE2_OGRE2TEXT_HH_
#define GZ_RENDERING_OGRE2_OGRE2TEXT_HH_

#include "gz/rendering/config.hh"

#ifdef GZ_RENDERING_HAVE_OGRE2_OVERLAY

#include <memory>
#include "gz/rendering/base/BaseText.hh"
#include "gz/rendering/ogre2/Ogre2Geometry.hh"

namespace Ogre
{
  class MovableObject;
}

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    //
    // forward declaration
    class Ogre2TextPrivate;

    /// \brief Ogre2.x implementation of the Text class
    class GZ_RENDERING_OGRE2_VISIBLE Ogre2Text
      : public BaseText<Ogre2Geometry>
    {
      /// \brief Constructor
      protected: Ogre2Text();

      /// \brief Destructor
      public: virtual ~Ogre2Text();

      // Documentation inherited
      public: virtual void Init() override;

      // Documentation inherited
      public: virtual void PreRender() override;

      // Documentation inherited
      public: virtual void Destroy() override;

      // Documentation inherited
      public: virtual Ogre::MovableObject *OgreObject() const override;

      // Documentation inherited.
      public: virtual MaterialPtr Material() const override;

      // Documentation inherited.
      public: virtual void SetMaterial(MaterialPtr _material, bool _unique)
          override;

      // Documentation inherited.
      public: virtual void SetFontName(const std::string &_font) override;

      // Documentation inherited.
      public: virtual void SetTextString(const std::string &_text) override;

      // Documentation inherited.
      public: virtual void SetColor(const gz::math::Color &_color)
          override;

      // Documentation inherited.
      public: virtual void SetCharHeight(const float _height) override;

      // Documentation inherited.
      public: virtual void SetSpaceWidth(const float _width) override;

      // Documentation inherited.
      public: virtual void SetTextAlignment(
                  const TextHorizontalAlign &_horizAlign,
                  const TextVerticalAlign &_vertAlign) override;

      // Documentation inherited.
      public: virtual void SetBaseline(const float _baseline) override;

      // Documentation inherited.
      public: virtual void SetShowOnTop(const bool _onTop) override;

      // Documentation inherited.
      public: virtual gz::math::AxisAlignedBox AABB() const override;

      /// \brief Set material to text geometry.
      /// \param[in] _material Ogre2 material.
      protected: virtual void SetMaterialImpl(Ogre2MaterialPtr _material);

      /// \brief Text should only be created by scene.
      private: friend class Ogre2Scene;

      /// \internal
      /// \brief Private data pointer
      private: std::unique_ptr<Ogre2TextPrivate> dataPtr;
    };
    }
  }
}
#endif  // GZ_RENDERING_HAVE_OGRE2_OVERLAY
#endif  // GZ_RENDERING_OGRE2_OGRE2TEXT_HH_
