/*
 * Copyright (C) 2026 Open Source Robotics Foundation
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
#ifndef GZ_RENDERING_FILAMENT_FILAMENTLIDARVISUAL_HH_
#define GZ_RENDERING_FILAMENT_FILAMENTLIDARVISUAL_HH_

#include <memory>
#include <string>
#include <vector>

#include "gz/rendering/base/BaseLidarVisual.hh"
#include "gz/rendering/filament/FilamentDynamicMesh.hh"
#include "gz/rendering/filament/FilamentVisual.hh"
#include "gz/rendering/filament/Export.hh"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    /// \brief Lidar rays as lines, triangle strips or points
    class GZ_RENDERING_FILAMENT_VISIBLE FilamentLidarVisual :
      public BaseLidarVisual<FilamentVisual>
    {
      protected: FilamentLidarVisual();

      public: virtual ~FilamentLidarVisual();

      public: virtual void Init() override;

      public: virtual void Destroy() override;

      public: virtual void Update() override;

      public: virtual void SetPoints(
                  const std::vector<double> &_points) override;

      public: virtual void ClearPoints() override;

      public: virtual unsigned int PointCount() const override;

      public: virtual std::vector<double> Points() const override;

      private: void ClearVisualData();

      /// \brief A mesh of the given type, attached to this visual
      private: std::shared_ptr<FilamentDynamicMesh> NewMesh(MarkerType _type,
                  const std::string &_material);

      private: std::vector<std::shared_ptr<FilamentDynamicMesh>> noHitRayStrips;

      private: std::vector<std::shared_ptr<FilamentDynamicMesh>> rayStrips;

      private: std::vector<std::shared_ptr<FilamentDynamicMesh>>
          deadZoneRayFans;

      private: std::vector<std::shared_ptr<FilamentDynamicMesh>> rayLines;

      private: std::vector<std::shared_ptr<FilamentDynamicMesh>> points;

      private: LidarVisualType lidarVisType{
          LidarVisualType::LVT_TRIANGLE_STRIPS};

      private: bool currentDisplayNonHitting{true};

      private: std::vector<double> lidarPoints;

      private: bool receivedData{false};

      private: friend class FilamentScene;
    };
    }
  }
}
#endif
