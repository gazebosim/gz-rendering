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
#include "ignition/rendering/config.hh"
#include "ignition/rendering/Geometry.hh"
#include "ignition/rendering/Object.hh"
#include "ignition/rendering/Material.hh"

namespace ignition
{
  namespace rendering
  {
    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
    //
    /// \class Marker Marker.hh ignition/rendering/Marker
    /// \brief TODO update description of this and all below funcs. 
    class IGNITION_RENDERING_VISIBLE Marker :
      public virtual Geometry
    {
      /// \brief Destructor
      public: virtual ~Marker() { }
      
      public: virtual void setLifetime(common::Time _lifetime) = 0;

      public: virtual common::Time Lifetime() const = 0;

      public: virtual void setLoaded(bool _loaded) = 0;

      public: virtual bool isLoaded() const = 0;

      public: virtual void setNS(std::string _ns) = 0;

      public: virtual std::string NS() const = 0;

      public: virtual void setID(uint64_t _id) const = 0;

      public: virtual uint64_t ID() const = 0;

      public: virtual void setLayer(int32_t _layer) = 0;

      public: virtual int32_t Layer() const = 0;

      public: virtual void setPose(math::Pose _pose) = 0;

      public: virtual math::Pose Pose() const = 0;

      public: virtual void setScale(math::Vector3d _scale) = 0;

      public: virtual math::Vector3d Scale() const = 0;

      public: virtual void setMaterial(Material _material) = 0;

      public: virtual Material Material() const = 0;

      public: virtual void setParent(std::string _parent) = 0;

      public: virtual std::string Parent() const = 0;
    };
    }
  }
}

#endif
