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
#ifndef GZ_RENDERING_FILAMENT_FILAMENTCUBEFACES_HH_
#define GZ_RENDERING_FILAMENT_FILAMENTCUBEFACES_HH_

#include <array>

#include <math/mat4.h>
#include <math/vec3.h>

// Cube faces shared by the sensors that render the full sphere into an atlas
// of 90 degree faces (GPU lidar, wide-angle camera). gz_wide_angle.mat
// hardcodes the same table.
namespace gz::rendering::filament_cube
{
  using filament::math::float3;

  /// \brief Cube face axes in the sensor frame (X forward, Y left, Z up):
  /// forward and up. Right is forward x up. Order: +X -X +Y -Y +Z -Z.
  struct Face { float3 fwd; float3 up; };
  inline const std::array<Face, 6> kFaces = {{
    {{1, 0, 0}, {0, 0, 1}}, {{-1, 0, 0}, {0, 0, 1}},
    {{0, 1, 0}, {0, 0, 1}}, {{0, -1, 0}, {0, 0, 1}},
    {{0, 0, 1}, {-1, 0, 0}}, {{0, 0, -1}, {1, 0, 0}}}};

  /// \brief Filament camera transform for a face: camera looks down -Z with
  /// Y up, so its basis columns are (right, up, -forward).
  inline filament::math::mat4f FaceTransform(const Face &_f)
  {
    const float3 r = cross(_f.fwd, _f.up);
    return filament::math::mat4f(
        filament::math::float4{r, 0.0f},
        filament::math::float4{_f.up, 0.0f},
        filament::math::float4{-_f.fwd, 0.0f},
        filament::math::float4{0.0f, 0.0f, 0.0f, 1.0f});
  }
}

#endif
