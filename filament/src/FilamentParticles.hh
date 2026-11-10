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
#ifndef GZ_RENDERING_FILAMENT_FILAMENTPARTICLES_HH_
#define GZ_RENDERING_FILAMENT_FILAMENTPARTICLES_HH_

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

#include <gz/math/Helpers.hh>
#include <gz/math/Vector3.hh>

#include "gz/rendering/filament/FilamentParticleEmitter.hh"
#include "gz/rendering/filament/FilamentScene.hh"

// Particle effects of depth cameras and GPU lidars, after ogre2. ogre2 renders
// the particles alone into a half resolution mask + depth pass, and where a
// particle is in front of the scene, a random fraction (the scatter ratio) of
// the pixels returns the particle depth plus noise (depth_camera_fs.glsl,
// gpu_rays_1st_pass_fs.glsl, Ogre2ParticleNoiseListener). Here the same
// half resolution particle depth is rasterized on the CPU: billboards face
// the camera, so each one is an axis-aligned rectangle at constant depth.
namespace gz::rendering::filament_particles
{
  /// \brief Noise of the particle returns
  struct Noise
  {
    /// \brief Std dev: a quarter of the emitter box X size
    double stddev{0.0};

    /// \brief Fraction of the particle pixels that return the particle
    double ratio{0.0};
  };

  /// \brief Noise of the first emitter with live particles, like ogre2's
  /// (which takes the first one in view). False when there are none.
  /// ponytail: one emitter's noise for all particles, as in ogre2
  inline bool FindNoise(const FilamentScene &_scene, Noise &_noise)
  {
    for (const FilamentParticleEmitter *e : _scene.ParticleEmitters())
    {
      if (e->Particles().empty())
        continue;
      const math::AxisAlignedBox box = e->ParticleBounds();
      _noise.stddev = (box.Max().X() - box.Min().X()) * 0.25;
      _noise.ratio = e->ParticleScatterRatio();
      return true;
    }
    return false;
  }

  /// \brief Rasterize the particles of the scene seen by a pinhole camera
  /// at _origin looking along _fwd (_left, _up complete the frame) into a
  /// _w x _h grid, top row first: the forward distance of the nearest
  /// particle whose billboard covers the texel center, +inf elsewhere.
  /// _fx, _fy: focal lengths in texels. Returns false if no texel is hit.
  inline bool Raster(const FilamentScene &_scene,
      const math::Vector3d &_origin, const math::Vector3d &_fwd,
      const math::Vector3d &_left, const math::Vector3d &_up,
      double _fx, double _fy, unsigned int _w, unsigned int _h,
      double _near, float *_depth)
  {
    std::fill(_depth, _depth + static_cast<size_t>(_w) * _h,
        std::numeric_limits<float>::infinity());
    bool hit = false;
    const double cx = _w * 0.5;
    const double cy = _h * 0.5;
    for (const FilamentParticleEmitter *e : _scene.ParticleEmitters())
    {
      for (const auto &p : e->Particles())
      {
        const math::Vector3d q = p.pos - _origin;
        const double f = q.Dot(_fwd);
        if (f <= _near)
          continue;
        const double l = q.Dot(_left);
        const double u = q.Dot(_up);
        // texels whose center (i + 0.5) lies inside the rectangle
        auto range = [](double _a, double _b, unsigned int _n,
            int &_lo, int &_hi)
        {
          _lo = static_cast<int>(std::ceil(std::clamp(_a - 0.5, -1.0,
              static_cast<double>(_n))));
          _hi = static_cast<int>(std::floor(std::clamp(_b - 0.5, -1.0,
              static_cast<double>(_n - 1))));
        };
        int i0, i1, j0, j1;
        range(cx - (l + 0.5 * p.width) / f * _fx,
            cx - (l - 0.5 * p.width) / f * _fx, _w, i0, i1);
        range(cy - (u + 0.5 * p.height) / f * _fy,
            cy - (u - 0.5 * p.height) / f * _fy, _h, j0, j1);
        const float fd = static_cast<float>(f);
        for (int j = std::max(j0, 0); j <= j1; ++j)
        {
          float *row = _depth + static_cast<size_t>(j) * _w;
          for (int i = std::max(i0, 0); i <= i1; ++i)
          {
            row[i] = std::min(row[i], fd);
            hit = true;
          }
        }
      }
    }
    return hit;
  }

  /// \brief ogre2's scatter rule for a pixel whose particle (range
  /// _particleRange) is in front of the scene (range _range): with _r
  /// uniform in [0, 1), the new range. ogre2 draws one random value per
  /// pixel and derives the Box-Muller inputs from it as well.
  inline double Scatter(double _r, const Noise &_noise,
      double _particleRange, double _range)
  {
    if (_r >= _noise.ratio)
      return _range;
    const double r = std::max(_r, 1e-12);
    const double rr = r - 0.5;
    double z = std::sqrt(-2.0 * std::log(r)) *
        (r < 0.5 ? std::sin(2.0 * GZ_PI * r) : std::cos(2.0 * GZ_PI * r));
    z = z * _noise.stddev + rr * rr * _noise.stddev * 0.5;
    // the noise vector is (z, z, z)
    const double newRange = _particleRange + std::sqrt(3.0) * std::abs(z);
    return std::min(newRange, _range);
  }
}

#endif
