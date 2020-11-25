/*
 * Copyright (C) 2020 Open Source Robotics Foundation
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
#ifndef IGNITION_RENDERING_HEIGHTMAPDESCRIPTOR_HH_
#define IGNITION_RENDERING_HEIGHTMAPDESCRIPTOR_HH_

#include <string>
#include <ignition/common/HeightmapData.hh>

#include "ignition/rendering/config.hh"
#include "ignition/rendering/Export.hh"

namespace ignition
{
  namespace rendering
  {
    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
    //
    /// \struct HeightmapDescriptor HeightmapDescriptor.hh
    /// ignition/rendering/HeightmapDescriptor.hh
    /// \brief Describes how a Heightmap should be loaded
    struct IGNITION_RENDERING_VISIBLE HeightmapDescriptor
    {
      struct IGNITION_RENDERING_VISIBLE Texture
      {
        /// \brief Texture size.
        public: double size{1.0};

        /// \brief Path to diffuse texture file.
        public: std::string diffuse;

        /// \brief Path to normal map file.
        public: std::string normal;
      };

      struct IGNITION_RENDERING_VISIBLE Blend
      {
        /// \brief Minimum height to blend from.
        public: double minHeight{0.0};

        /// \brief Distance to blend.
        public: double fadeDistance{0.0};
      };

      /// \brief Contains heightfield data.
      public: std::shared_ptr<common::HeightmapData> data{nullptr};

      /// \brief Heightmap XYZ size in meters.
      public: math::Vector3d size{1.0, 1.0, 1.0};

      /// \brief Heightmap XYZ origin in meters.
      public: math::Vector3d position{0.0, 0.0, 0.0};

      /// \brief Flag that enables/disables the terrain paging
      public: bool useTerrainPaging{false};

      /// \brief Number of samples per heightmap datum.
      public: unsigned int sampling{2u};

      /// \brief Textures in this heightmap, in height order.
      public: std::vector<Texture> textures;

      /// \brief Blends in this heightmap, in height order. There should be one
      /// less than textures.
      public: std::vector<Blend> blends;
    };
    }
  }
}
#endif
