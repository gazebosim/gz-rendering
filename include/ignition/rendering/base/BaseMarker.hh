/*
 * Copyright (C) 2019 Open Source Robotics Foundation
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
#ifndef IGNITION_RENDERING_MARKER_HH_
#define IGNITION_RENDERING_MARKER_HH_

#include <string>

#include "ignition/rendering/ogre/Export.hh"
#include "ignition/rendering/Visual.hh"

namespace ignition
{
  namespace rendering
  {
    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
    /// \cond
    /// \brief A marker visualization. The MarkerManager class should
    /// instantiate instances of this class.
    /// \sa MarkerManager
    class IGNITION_RENDERING_OGRE_VISIBLE BaseMarker
    {
      /*
      /// \brief Constructor.
      /// \param[in] _name Name of the visual.
      /// \param[in] _vis Pointer to the parent Visual.
      public: BaseMarkerVisual();

      /// \brief Destructor.
      public: virtual ~BaseMarkerVisual();
      */
      
      /// \brief Load the marker
      /// \param[in] _msg The marker message to load the visual from.
      public: void Load(const ignition::msgs::Marker &_msg) = 0;
      using Visual::Load;

      /// \brief Get the lifetime of the marker
      /// \return Life time of the marker in simulation time.
      public: common::Time Lifetime() const = 0;

      // Documentation inherited
      public: virtual void Fini() = 0;

      /// \brief Populate a marker message.
      /// \param[in] _msg The message to populate.
      public: void FillMsg(ignition::msgs::Marker &_msg) = 0;

      /// \brief Add or modify a marker
      /// \param[in] _msg The message that defines what to add or modify
      private: void AddModify(const ignition::msgs::Marker &_msg) = 0;

      /// \brief Add or modify a dynamic renderable.
      /// \param[in] _msg The message that defines what to add or modify
      private: void DynamicRenderable(const ignition::msgs::Marker &_msg) = 0;

      /// \brief Add or modify movable text.
      /// \param[in] _msg The message that defines what to add or modify
      private: void Text(const ignition::msgs::Marker &_msg) = 0;
    };
    
    /////////////////////////////////////////////////
    // BaseMarkerVisual
    /////////////////////////////////////////////////
    template <class T>
    BaseMarker<T>::BaseMarker()
    {
    }

    /////////////////////////////////////////////////
    template <class T>
    BaseMarker<T>::~BaseMarker()
    {
    }
    
    }
    /// \endcond
  }
}
#endif
