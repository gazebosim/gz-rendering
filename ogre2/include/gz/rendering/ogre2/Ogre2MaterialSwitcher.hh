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

#ifndef IGNITION_RENDERING_OGRE2_OGRE2MATERIALSWITCHER_HH_
#define IGNITION_RENDERING_OGRE2_OGRE2MATERIALSWITCHER_HH_

#include <map>
#include <string>

#include <ignition/math/Color.hh>
#include "ignition/rendering/config.hh"
#include "ignition/rendering/ogre2/Export.hh"
#include "ignition/rendering/ogre2/Ogre2Includes.hh"
#include "ignition/rendering/ogre2/Ogre2RenderTypes.hh"

namespace ignition
{
  namespace rendering
  {
    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
    //
    // forward declarations
    class Ogre2SelectionBuffer;

    /// \brief Helper class to assign unique colors to renderables
    class IGNITION_RENDERING_OGRE2_VISIBLE Ogre2MaterialSwitcher :
      public Ogre::RenderTargetListener
    {
      /// \brief Constructor
      public: explicit Ogre2MaterialSwitcher(Ogre2ScenePtr _scene);

      /// \brief Destructor
      public: ~Ogre2MaterialSwitcher();

      /// \brief Get the entity with a specific color
      /// \param[in] _color The entity's color.
      public: std::string EntityName(
              const ignition::math::Color &_color) const;

      /// \brief Reset the color value incrementor
      public: void Reset();

      /// \brief Ogre's pre render update callback
      /// \param[in] _evt Ogre render target event containing information about
      /// the source render target.
      public: virtual void preRenderTargetUpdate(
                  const Ogre::RenderTargetEvent &_evt);

      /// \brief Ogre's post render update callback
      /// \param[in] _evt Ogre render target event containing information about
      /// the source render target.
      public: virtual void postRenderTargetUpdate(
                  const Ogre::RenderTargetEvent &_evt);

      /// \brief Current unique color value
      private: ignition::math::Color currentColor;

      /// \brief Color dictionary that maps the unique color value to
      /// renderable name
      private: std::map<unsigned int, std::string> colorDict;

      /// \brief A map of ogre sub item pointer to their original hlms material
      private: std::map<Ogre::SubItem *, Ogre::HlmsDatablock *> datablockMap;

      /// \brief Ogre v1 material consisting of a shader that changes the
      /// appearance of item to use a unique color for mouse picking
      private: Ogre::MaterialPtr plainMaterial;

      /// \brief Ogre v1 material consisting of a shader that changes the
      /// appearance of item to use a unique color for mouse picking. In
      /// addition, the depth check and depth write properties disabled.
      private: Ogre::MaterialPtr plainOverlayMaterial;

      /// \brief Increment unique color value that will be assigned to the
      /// next renderable
      private: void NextColor();

      /// \brief Selection Buffer class that make use of this class for
      /// selecting entitiies
      public: friend class Ogre2SelectionBuffer;

      /// \brief Plain material technique
      private: Ogre2ScenePtr scene;
    };
    }
  }
}
#endif
