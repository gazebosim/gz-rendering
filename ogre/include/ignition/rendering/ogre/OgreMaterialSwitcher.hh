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

#ifndef IGNITION_RENDERING_SELECTIONBUFFER_MATERIALSWITCHER_HH_
#define IGNITION_RENDERING_SELECTIONBUFFER_MATERIALSWITCHER_HH_

#include <map>
#include <string>

#include <ignition/math/Color.hh>
#include "ignition/rendering/ogre/OgreIncludes.hh"
#include "ignition/rendering/ogre/OgreRenderTypes.hh"
#include "ignition/rendering/ogre/OgreObject.hh"

namespace ignition
{
  namespace rendering
  {
    class OgreSelectionBuffer;
    class IGNITION_RENDERING_OGRE_VISIBLE OgreMaterialSwitcher :
      public Ogre::MaterialManager::Listener
    {
      /// \brief Constructor
      public: OgreMaterialSwitcher();

      /// \brief Destructor
      public: ~OgreMaterialSwitcher();

      /// \brief Get the entity with a specific color
      /// \param[in] _color The entity's color.
      public: const std::string &GetEntityName(
              const ignition::math::Color &_color) const;

      /// \brief Reset the color value incrementor
      public: void Reset();

      /// \brief Ogre callback that assigns colors to new renderables
      public: virtual Ogre::Technique *handleSchemeNotFound(
                  uint16_t _schemeIndex, const Ogre::String &_schemeName,
                  Ogre::Material *_originalMaterial, uint16_t _lodIndex,
                  const Ogre::Renderable *_rend);

      // private: typedef std::map<unsigned int, std::string, cmp_color>
      // ColorMap;
      private: typedef std::map<unsigned int, std::string> ColorMap;
      private: typedef ColorMap::const_iterator ColorMapConstIter;
      private: std::string emptyString;
      private: ignition::math::Color currentColor;
      private: std::string lastEntity;
      private: Ogre::Technique *lastTechnique;
      private: OgreMaterialSwitcher::ColorMap colorDict;

      private: void GetNextColor();

      public: friend class OgreSelectionBuffer;

      /// \brief Plain material technique
      private: Ogre::Technique *plainTechnique = nullptr;

      /// \brief Overlay material technique
      private: Ogre::Technique *overlayTechnique = nullptr;
    };
  }
}
#endif
