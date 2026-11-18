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

// Built with -fno-rtti like filament, whose classes have no typeinfo to
// derive from (see CMakeLists.txt).

#include <algorithm>

#include <filament/ColorGrading.h>
#include <filament/ToneMapper.h>

namespace
{
/// \brief Khronos PBR Neutral without its toe: the identity up to kStart,
/// then the same hue-preserving highlight roll-off towards 1. The toe
/// (subtract up to 0.04, for an F0 = 0.04 specular that gz materials mostly
/// do not show) crushes ambient-lit sides several times darker than ogre2.
struct NeutralHighlightToneMapper final : public filament::ToneMapper
{
  filament::math::float3 operator()(filament::math::float3 _c)
      const noexcept override
  {
    constexpr float kStart = 0.8f;
    constexpr float kDesaturation = 0.15f;
    const float peak = std::max(_c.r, std::max(_c.g, _c.b));
    if (peak < kStart)
      return _c;
    constexpr float d = 1.0f - kStart;
    const float newPeak = 1.0f - d * d / (peak + d - kStart);
    _c *= newPeak / peak;
    const float g = 1.0f - 1.0f / (kDesaturation * (peak - newPeak) + 1.0f);
    return _c * (1.0f - g) + filament::math::float3(newPeak) * g;
  }
};
}

/// \brief Used by FilamentCamera's quality mode: tone mapping, sRGB
/// output (the camera's target is plain RGBA8 then)
filament::ColorGrading *CreateQualityColorGrading(filament::Engine &_engine)
{
  NeutralHighlightToneMapper toneMapper;
  return filament::ColorGrading::Builder()
      .toneMapper(&toneMapper)
      .build(_engine);
}
