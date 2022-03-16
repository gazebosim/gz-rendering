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

#ifndef IGNITION_RENDERING_OGRE2_OGREMAPVISUAL_HH_
#define IGNITION_RENDERING_OGRE2_OGREMAPVISUAL_HH_

#include <memory>
#include <vector>
#include "ignition/rendering/base/BaseMapVisual.hh"
#include "ignition/rendering/ogre2/Ogre2Visual.hh"
#include "ignition/rendering/ogre2/Ogre2Scene.hh"

namespace ignition
{
  namespace rendering
  {
    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
    //
    // Forward declaration
    class Ogre2MapVisualPrivate;

    /// \brief Ogre 2.x implementation of a map Visual.
    class IGNITION_RENDERING_OGRE2_VISIBLE Ogre2MapVisual
      : public BaseMapVisual<Ogre2Visual>
    {
      /// \brief Constructor
      protected: Ogre2MapVisual();

      /// \brief Destructor
      public: virtual ~Ogre2MapVisual();

      // Documentation inherited.
      public: virtual void Init() override;

      // Documentation inherited.
      public: virtual void PreRender() override;

      // Documentation inherited.
      public: virtual void Destroy() override;

      /// \brief Create the map Visual in ogre
      private: void Create();

      /// \brief Clear data stored by dynamiclines
      private: void ClearVisualData();

      // Documentation inherited
      public: virtual void SetVisible(bool _visible) override;

      /// \brief Map Visual should only be created by scene.
      private: friend class Ogre2Scene;

      /// \brief Private data class
      private: std::unique_ptr<Ogre2MapVisualPrivate> dataPtr;
    };
    }
  }
}
#endif
