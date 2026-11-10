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
#ifndef GZ_RENDERING_FILAMENT_FILAMENTPARTICLEEMITTER_HH_
#define GZ_RENDERING_FILAMENT_FILAMENTPARTICLEEMITTER_HH_

#include <chrono>
#include <string>
#include <vector>

#include <utils/Entity.h>

#include <gz/math/AxisAlignedBox.hh>
#include <gz/math/Color.hh>
#include <gz/math/Vector3.hh>

#include "gz/rendering/config.hh"
#include "gz/rendering/base/BaseParticleEmitter.hh"
#include "gz/rendering/filament/FilamentVisual.hh"
#include "gz/rendering/filament/Export.hh"

namespace filament
{
  class IndexBuffer;
  class MaterialInstance;
  class VertexBuffer;
}

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    //
    /// \brief Particle emitter simulated on the CPU like ogre2's ParticleFX
    /// (world space particles, constant emission direction, linear growth,
    /// colour image over the lifetime) and drawn as camera-facing billboards.
    /// Depth cameras and GPU lidars do not render the billboards: they read
    /// Particles() and apply ogre2's scatter / noise rule (see
    /// FilamentParticles.hh).
    class GZ_RENDERING_FILAMENT_VISIBLE FilamentParticleEmitter :
      public BaseParticleEmitter<FilamentVisual>
    {
      /// \brief One live particle, in the world frame
      public: struct Particle
      {
        math::Vector3d pos;
        math::Vector3d vel;
        double ttl;
        /// \brief Lifetime at emission
        double life;
        /// \brief Colour (rgba) from the colour image at the current age
        math::Color color;
        double width;
        double height;
      };

      protected: FilamentParticleEmitter();

      public: virtual ~FilamentParticleEmitter();

      public: virtual void Destroy() override;

      public: virtual void PreRender() override;

      /// \brief Like ogre2, where this is disabled (gz-rendering#902): warns
      /// and does nothing
      public: virtual void SetColorRange(const math::Color &_colorStart,
                  const math::Color &_colorEnd) override;

      /// \brief Colours the particles from the first row of the image over
      /// their lifetime, left to right, with the image alpha as particle
      /// alpha (ogre2's ColourImage affector). Particles are white without
      /// an image.
      public: virtual void SetColorRangeImage(const std::string &_image)
                  override;

      /// \brief Draw the particles back to front along _dir, a camera's
      /// forward vector, like ogre2's per camera sort. Re-uploads the
      /// billboards when _dir changed; cameras call it before rendering.
      /// ponytail: one direction for all cameras, the one that rendered
      /// last; cameras that don't call this (wide angle faces) draw in the
      /// order of the last call
      public: void SortFor(const math::Vector3d &_dir);

      /// \brief Live particles; empty when the emitter is hidden
      public: const std::vector<Particle> &Particles() const;

      /// \brief World box of the particles padded by half their size,
      /// round-tripped through the emitter frame like ogre2's local AABB
      public: math::AxisAlignedBox ParticleBounds() const;

      protected: virtual void Init() override;

      /// \brief Advance the simulation by _dt seconds (ogre2's
      /// ParticleSystem::_update order: expire, grow, move, emit)
      private: void Step(double _dt);

      /// \brief Set the renderable's layers and material, upload the
      /// billboards
      private: void UpdateRenderable();

      /// \brief Write the billboards, sorted along sortDir, to the vertex
      /// buffer
      private: void Upload();

      private: std::vector<Particle> particles;

      /// \brief First row of the colour image; empty: no colour affector
      private: std::vector<math::Color> colorRow;

      /// \brief Direction the billboards are sorted along
      private: math::Vector3d sortDir;

      /// \brief Fractional particles left over by the emission rate
      private: double remainder{0.0};

      /// \brief Emitting time, for the duration limit
      private: double emitTime{0.0};

      /// \brief Scene time of the last step
      private: std::chrono::steady_clock::duration lastTime{0};

      private: bool stepped{false};

      private: ::utils::Entity renderable;

      private: filament::VertexBuffer *vb{nullptr};

      private: filament::IndexBuffer *ib{nullptr};

      private: filament::MaterialInstance *mi{nullptr};

      private: friend class FilamentScene;
    };
    }
  }
}
#endif
