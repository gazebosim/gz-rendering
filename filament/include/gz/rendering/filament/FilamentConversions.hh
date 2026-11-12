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
#ifndef GZ_RENDERING_FILAMENT_FILAMENTCONVERSIONS_HH_
#define GZ_RENDERING_FILAMENT_FILAMENTCONVERSIONS_HH_

#include <array>
#include <cmath>

#include <math/mat4.h>
#include <math/quat.h>
#include <math/vec3.h>
#include <math/vec4.h>

#include <gz/math/Color.hh>
#include <gz/math/Matrix4.hh>
#include <gz/math/Pose3.hh>
#include <gz/math/Vector3.hh>
#include <gz/math/Vector4.hh>

#include "gz/rendering/config.hh"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    //
    /// \brief Conversion helpers between gz-math and filament::math types.
    namespace FilamentConversions
    {
      /// \brief Convert a gz-math vector to a filament float3.
      /// \param[in] _v Vector to convert.
      /// \return The vector, narrowed to float.
      inline filament::math::float3 Convert(const math::Vector3d &_v)
      {
        return {static_cast<float>(_v.X()), static_cast<float>(_v.Y()),
                static_cast<float>(_v.Z())};
      }

      /// \brief Convert a gz-math quaternion to a filament quatf.
      /// \param[in] _q Quaternion to convert.
      /// \return The quaternion, narrowed to float.
      inline filament::math::quatf Convert(const math::Quaterniond &_q)
      {
        return {static_cast<float>(_q.W()), static_cast<float>(_q.X()),
                static_cast<float>(_q.Y()), static_cast<float>(_q.Z())};
      }

      /// \brief Convert a gz-math matrix to a filament mat4. gz-math
      /// matrices index (row, column), filament's [column][row].
      /// \param[in] _m Matrix to convert.
      /// \return The same transform as a filament matrix.
      inline filament::math::mat4 Convert(const math::Matrix4d &_m)
      {
        filament::math::mat4 out;
        for (int c = 0; c < 4; ++c)
          for (int r = 0; r < 4; ++r)
            out[c][r] = _m(r, c);
        return out;
      }

      /// \brief Compute _m * (_v, 1), with the homogeneous w that gz-math's
      /// Matrix4 * Vector3 drops.
      /// \param[in] _m Transform matrix.
      /// \param[in] _v Point to transform.
      /// \return The transformed homogeneous point (x, y, z, w).
      inline math::Vector4d Transform(const math::Matrix4d &_m,
          const math::Vector3d &_v)
      {
        math::Vector4d r;
        for (int i = 0; i < 4; ++i)
        {
          r[i] = _m(i, 0) * _v.X() + _m(i, 1) * _v.Y() + _m(i, 2) * _v.Z() +
              _m(i, 3);
        }
        return r;
      }

      /// \brief Local transform matrix T * R * S.
      /// \param[in] _pose Translation (T) and rotation (R).
      /// \param[in] _scale Scale (S).
      /// \return The combined local transform as a filament matrix.
      inline filament::math::mat4f Convert(const math::Pose3d &_pose,
          const math::Vector3d &_scale)
      {
        return filament::math::mat4f::translation(Convert(_pose.Pos())) *
               filament::math::mat4f(Convert(_pose.Rot())) *
               filament::math::mat4f::scaling(Convert(_scale));
      }

      /// \brief Convert a gz color to a filament float3 (RGB). gz colors are
      /// linear: ogre2 feeds them to the shaders as-is and renders into an
      /// sRGB target, so do the same here.
      /// \param[in] _c Color to convert; alpha is dropped.
      /// \return The RGB components, unchanged.
      inline filament::math::float3 ToFloat3(const math::Color &_c)
      {
        return {_c.R(), _c.G(), _c.B()};
      }

      /// \brief Convert a gz color to a filament float4 (RGBA), unchanged.
      /// \param[in] _c Color to convert.
      /// \return The RGBA components.
      inline filament::math::float4 ToFloat4(const math::Color &_c)
      {
        return {_c.R(), _c.G(), _c.B(), _c.A()};
      }

      /// \brief Filament layer of particle emitters. It stands for gz bit
      /// 0x00100000, ogre2's Ogre2ParticleEmitter::kParticleVisibilityFlags.
      constexpr uint8_t kParticleLayer = 0x80u;

      /// \brief gz bit of particle emitters (ogre2's kParticleVisibilityFlags)
      constexpr uint32_t kParticleFlag = 0x00100000u;

      /// \brief Distinct visual flag values in use; entry i is the value
      /// drawn on filament layer i. Entry 7 is the particle value.
      /// Process-wide and first come; a value past the 7th distinct
      /// one shares layer 6 (its entry becomes the OR of the values), so
      /// scenes with more than 7 distinct visual flag values can
      /// over-render. Upgrade to a per-scene table with re-layering if that
      /// ever matters.
      /// \return The process-wide table of visual flag values.
      inline std::array<uint32_t, 8> &LayerValues()
      {
        static std::array<uint32_t, 8> values = [] {
          std::array<uint32_t, 8> v{};
          v[7] = kParticleFlag;
          return v;
        }();
        return values;
      }

      /// \brief Filament layer of a visual with gz visibility flags _flags.
      /// gz draws a visual when (flags & cameraMask) != 0; giving each
      /// distinct flag value its own layer keeps that test exact for any
      /// mask, including masks with most bits set such as ~GZ_VISIBILITY_GUI.
      /// \param[in] _flags gz visibility flags of the visual.
      /// \return The filament layer bit of the visual.
      inline uint8_t ToLayers(uint32_t _flags)
      {
        if (_flags == 0u)
          return 0u;
        if (_flags == kParticleFlag)
          return kParticleLayer;
        auto &values = LayerValues();
        for (int i = 0; i < 7; ++i)
        {
          if (values[i] == _flags)
            return static_cast<uint8_t>(1u << i);
          if (values[i] == 0u)
          {
            values[i] = _flags;
            return static_cast<uint8_t>(1u << i);
          }
        }
        values[6] |= _flags;
        return 1u << 6;
      }

      /// \brief Filament layers a camera with gz visibility mask _mask sees:
      /// every layer whose visual flag value intersects the mask. Call it
      /// per render, after visuals have registered their flags.
      /// \param[in] _mask gz visibility mask of the camera.
      /// \return The filament layer mask of the camera.
      inline uint8_t MaskToLayers(uint32_t _mask)
      {
        const auto &values = LayerValues();
        uint8_t layers = 0u;
        for (int i = 0; i < 8; ++i)
          if (values[i] & _mask)
            layers |= static_cast<uint8_t>(1u << i);
        return layers;
      }

      /// \brief Layers seen by sensors that do not render particles as
      /// geometry (depth, lidar, thermal, segmentation, bounding boxes).
      /// \param[in] _mask gz visibility mask of the sensor.
      /// \return The filament layer mask, without the particle layer.
      inline uint8_t ToSensorLayers(uint32_t _mask)
      {
        return MaskToLayers(_mask) & static_cast<uint8_t>(~kParticleLayer);
      }

      /// \brief sRGB transfer function, for values written outside of a
      /// filament render pass (e.g. background-colored point cloud entries).
      /// \param[in] _x Linear value, normally in [0, 1].
      /// \return The sRGB-encoded value.
      inline double LinearToSrgb(double _x)
      {
        return _x < 0.0031308 ? _x * 12.92
                              : 1.055 * std::pow(_x, 1.0 / 2.4) - 0.055;
      }

      /// \brief Rotation from the gz camera frame (X forward, Z up) to the
      /// filament camera frame (-Z forward, Y up). Same matrix as the one
      /// used by BaseCamera::ViewMatrix().
      /// \return The camera frame rotation as a 4x4 matrix.
      inline filament::math::mat4f CameraFix()
      {
        return filament::math::mat4f(
            filament::math::float4{0.0f, -1.0f, 0.0f, 0.0f},
            filament::math::float4{0.0f, 0.0f, 1.0f, 0.0f},
            filament::math::float4{-1.0f, 0.0f, 0.0f, 0.0f},
            filament::math::float4{0.0f, 0.0f, 0.0f, 1.0f});
      }
    }
    }
  }
}
#endif
