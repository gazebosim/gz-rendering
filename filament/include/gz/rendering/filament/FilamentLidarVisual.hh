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
    /// \brief Lidar rays as lines, triangle strips or points.
    class GZ_RENDERING_FILAMENT_VISIBLE FilamentLidarVisual :
      public BaseLidarVisual<FilamentVisual>
    {
      /// \brief Constructor
      protected: FilamentLidarVisual();

      /// \brief Destructor
      public: virtual ~FilamentLidarVisual();

      // Documentation inherited
      public: virtual void Init() override;

      // Documentation inherited
      public: virtual void Destroy() override;

      // Documentation inherited
      public: virtual void Update() override;

      // Documentation inherited
      public: virtual void SetPoints(
                  const std::vector<double> &_points) override;

      // Documentation inherited
      public: virtual void ClearPoints() override;

      // Documentation inherited
      public: virtual unsigned int PointCount() const override;

      // Documentation inherited
      public: virtual std::vector<double> Points() const override;

      /// \brief Destroys the meshes of all the visual types.
      private: void ClearVisualData();

      /// \brief A mesh of the given type, attached to this visual
      private: std::shared_ptr<FilamentDynamicMesh> NewMesh(MarkerType _type,
                  const std::string &_material);

      /// \brief Light blue triangle strips, one per scan row, that also span
      /// the rays without a hit, up to the max range, when those are
      /// displayed (triangle strips only).
      private: std::vector<std::shared_ptr<FilamentDynamicMesh>> noHitRayStrips;

      /// \brief Blue triangle strips, one per scan row, from the min range
      /// to the hit point (triangle strips only).
      private: std::vector<std::shared_ptr<FilamentDynamicMesh>> rayStrips;

      /// \brief Triangle fans, one per scan row, from the sensor origin to
      /// the min range: the dead zone (triangle strips only).
      private: std::vector<std::shared_ptr<FilamentDynamicMesh>>
          deadZoneRayFans;

      /// \brief Line lists, one per scan row, with a segment per ray
      /// (triangle strips and ray lines).
      private: std::vector<std::shared_ptr<FilamentDynamicMesh>> rayLines;

      /// \brief Point meshes, one per scan row (points only).
      private: std::vector<std::shared_ptr<FilamentDynamicMesh>> points;

      /// \brief Visual type the current meshes were built for.
      private: LidarVisualType lidarVisType{
          LidarVisualType::LVT_TRIANGLE_STRIPS};

      /// \brief Whether the current meshes show the rays without a hit.
      private: bool currentDisplayNonHitting{true};

      /// \brief Ranges received by SetPoints, row by row.
      private: std::vector<double> lidarPoints;

      /// \brief True while lidarPoints holds data Update has not drawn yet.
      private: bool receivedData{false};

      /// \brief Only the scene can create this class.
      private: friend class FilamentScene;
    };
    }
  }
}
#endif
