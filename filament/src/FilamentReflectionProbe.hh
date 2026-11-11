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
#ifndef GZ_RENDERING_FILAMENT_FILAMENTREFLECTIONPROBE_HH_
#define GZ_RENDERING_FILAMENT_FILAMENTREFLECTIONPROBE_HH_

#include <array>
#include <cstdint>
#include <memory>

#include <filament-iblprefilter/IBLPrefilterContext.h>
#include <math/vec3.h>
#include <math/vec4.h>
#include <utils/Entity.h>

#include "gz/rendering/config.hh"

namespace filament
{
  class Camera;
  class Engine;
  class IndirectLight;
  class RenderTarget;
  class Scene;
  class Texture;
  class View;
}

namespace gz::rendering
{
inline namespace GZ_RENDERING_VERSION_NAMESPACE
{
namespace filament_probe
{
  /// \brief Reflections of the scene around a point, for the quality-mode
  /// GUI camera: a cube map of the scene rendered from that point (linear
  /// radiance, no post-processing), GGX-prefiltered like the sky's. It
  /// replaces the sky as the image based lighting's reflections, so glossy
  /// surfaces and metals reflect the room they are in instead of a sky the
  /// walls hide. The diffuse term (the SH irradiance) stays the scene's.
  /// The cube is refreshed a few faces per frame, so moving objects and a
  /// moving camera catch up within a few frames.
  class ReflectionProbe
  {
    /// \param[in] _scene Scene to render
    /// \param[in] _size Cube face size in pixels
    /// \param[in] _samples Prefilter samples per texel
    public: ReflectionProbe(filament::Engine &_engine,
        filament::Scene *_scene, uint32_t _size, uint16_t _samples);

    public: ~ReflectionProbe();

    /// \brief Render the next _faces cube faces from _pos (round robin),
    /// while faces rendered before the last change of _signature (a hash
    /// of the scene state the cube depends on, _pos included) remain;
    /// prefilter the cube once 6 fresh faces are in. Static scenes cost
    /// nothing.
    /// \param[in] _layers Visible layers (the GUI camera's)
    /// \param[in] _clear Background colour (no sky)
    /// \return True when the reflections were updated
    public: bool Render(const filament::math::double3 &_pos, uint8_t _layers,
        float _near, float _far, const filament::math::float4 &_clear,
        unsigned int _faces, uint64_t _signature);

    /// \brief The IBL to draw with: _sh (the scene IBL's irradiance) and
    /// these reflections. Null until the first full cube. Rebuilt when _sh
    /// changes.
    public: filament::IndirectLight *Light(
        const filament::math::float3 *_sh, uint8_t _bands);

    private: filament::Engine &engine;

    private: uint32_t size;

    /// \brief Cube of linear radiance, with mipmaps for the prefilter
    private: filament::Texture *cube{nullptr};

    private: filament::Texture *depth{nullptr};

    /// \brief One target per cube face
    private: std::array<filament::RenderTarget *, 6> targets{};

    private: filament::View *view{nullptr};

    private: ::utils::Entity cameraEntity;

    private: filament::Camera *camera{nullptr};

    private: std::unique_ptr<IBLPrefilterContext> context;

    private: std::unique_ptr<IBLPrefilterContext::SpecularFilter> filter;

    /// \brief Prefiltered cube (the light's reflections)
    private: filament::Texture *reflections{nullptr};

    private: filament::IndirectLight *light{nullptr};

    /// \brief Irradiance the light was built with
    private: std::array<filament::math::float3, 9> lightSh{};

    private: uint8_t lightBands{0u};

    /// \brief Next face to render
    private: unsigned int nextFace{0u};

    /// \brief Faces to render before the cube is up to date
    private: unsigned int staleFaces{6u};

    /// \brief _signature of the last Render
    private: uint64_t signature{0u};
  };
}
}
}

#endif
