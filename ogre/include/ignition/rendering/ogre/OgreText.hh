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

#ifndef IGNITION_RENDERING_OGRE_OGRETEXT_HH_
#define IGNITION_RENDERING_OGRE_OGRETEXT_HH_

#include <memory>
#include <string>

#include <ignition/math/Box.hh>
#include <ignition/math/Color.hh>

#include "ignition/rendering/base/BaseText.hh"
#include "ignition/rendering/ogre/OgreGeometry.hh"
#include "ignition/rendering/ogre/OgreIncludes.hh"
#include "ignition/rendering/ogre/Export.hh"

namespace ignition
{
  namespace rendering
  {
    class OgreTextPrivate;

    /// \brief Ogre implementation of text geometry
    class IGNITION_RENDERING_OGRE_VISIBLE OgreText
      : public Ogre::MovableObject, public Ogre::Renderable,
        public BaseText<OgreGeometry>
    {
      /// \brief Constructor
      public: OgreText();

      /// \brief Destructor
      public: virtual ~OgreText();

      // Documentation inherited
      public: virtual void Init();

      // Documentation inherited
      public: virtual void PreRender();

      // Documentation inherited
      public: Ogre::MovableObject *OgreObject() const;

      // Documentation inherited.
      public: virtual MaterialPtr Material() const;

      // Documentation inherited.
      public: virtual void SetMaterial(MaterialPtr _material, bool _unique);

      /// \brief Set material to text geometry.
      /// \param[in] _material Ogre material.
      protected: virtual void SetMaterialImpl(OgreMaterialPtr _material);

      // Documentation inherited.
      public: void SetFontName(const std::string &_font);

      // Documentation inherited.
      public: void SetColor(const ignition::math::Color &_color);

      // Documentation inherited.
      public: void SetShowOnTop(const bool _onTop);

      // Documentation inherited.
      public: ignition::math::Box AABB();


      /// \internal
      /// \brief Method to allow a caller to abstractly iterate over the
      /// renderable instances.
      /// \param[in] _visitor Renderable instances to visit
      /// \param[in] _debug True if set to debug
      public: virtual void visitRenderables(Ogre::Renderable::Visitor *_visitor,
          bool _debug = false) override;

      /// \brief Setup the geometry.
      protected: void SetupGeometry();

      /// \brief Update colors.
      protected: void UpdateColors();

      /// \brief Update font.
      protected: void UpdateFont();

      /// \brief Set font name implementation.
      /// \param-in] _font Name of font
      protected: void SetFontNameImpl(const std::string &_font);

      /// \internal
      /// \brief Get the world transform (from MovableObject)
      protected: void getWorldTransforms(Ogre::Matrix4 *_xform) const override;

      /// \internal
      /// \brief Get the bounding radiu (from MovableObject)
      protected: float getBoundingRadius() const override;

      /// \internal
      /// \brief Get the squared view depth (from MovableObject)
      protected: float getSquaredViewDepth(const Ogre::Camera *_cam) const
          override;

      /// \internal
      /// \brief Get the render operation
      protected: void getRenderOperation(Ogre::RenderOperation &_op) override;

      /// \internal
      /// \brief Get the material
      protected: const Ogre::MaterialPtr &getMaterial() const override;

      /// \internal
      /// \brief Get the lights
      /// \deprecated Function has never returned meaningful values
      protected: const Ogre::LightList &getLights() const override;

      /// \internal
      private: const Ogre::AxisAlignedBox &getBoundingBox() const override;

      /// \internal
      private: const Ogre::String &getMovableType() const override;

      /// \internal
      private: void _notifyCurrentCamera(Ogre::Camera *_cam) override;

      /// \internal
      private: void _updateRenderQueue(Ogre::RenderQueue *_queue) override;

      /// \internal
      /// \brief Private data pointer
      private: std::unique_ptr<OgreTextPrivate> dataPtr;
    };
    /// \}
  }
}
#endif
