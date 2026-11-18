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
#ifndef GZ_RENDERING_FILAMENT_FILAMENTTHERMALCAMERA_HH_
#define GZ_RENDERING_FILAMENT_FILAMENTTHERMALCAMERA_HH_

#include <map>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

#include <gz/common/Event.hh>

#include "gz/rendering/base/BaseThermalCamera.hh"
#include "gz/rendering/filament/FilamentCamera.hh"
#include "gz/rendering/filament/FilamentRenderTypes.hh"
#include "gz/rendering/filament/Export.hh"

namespace filament
{
  class MaterialInstance;
  class Texture;
}

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    //
    /// \brief Thermal camera. Submeshes are drawn with a flat id color
    /// (textured ones unlit as diffuse x texture); the temperature of each
    /// id is computed on the CPU with the same formula and 8 bit
    /// intermediate quantization as ogre2's thermal shader, so outputs are
    /// comparable. Heat signature textures (a string "temperature" with
    /// "minTemp" / "maxTemp") are mapped on the GPU like ogre2's
    /// heat_signature_fs.glsl.
    class GZ_RENDERING_FILAMENT_VISIBLE FilamentThermalCamera :
      public BaseThermalCamera<FilamentCamera>
    {
      /// \brief Constructor
      protected: FilamentThermalCamera() = default;

      /// \brief Destructor
      public: virtual ~FilamentThermalCamera() = default;

      /// \brief Forwards to FilamentCamera::SetProjectionMatrix, which
      /// BaseCamera<FilamentCamera> would otherwise override.
      /// \param[in] _matrix Custom projection matrix.
      public: virtual void SetProjectionMatrix(const math::Matrix4d &_matrix)
          override { FilamentCamera::SetProjectionMatrix(_matrix); }

      /// \brief Forwards to FilamentCamera::ProjectionMatrix, which
      /// BaseCamera<FilamentCamera> would otherwise override.
      /// \return The projection matrix.
      public: virtual math::Matrix4d ProjectionMatrix() const override
          { return FilamentCamera::ProjectionMatrix(); }

      /// \brief Draws every submesh with a flat id color, an unlit textured
      /// color or a heat signature material.
      public: virtual void Render() override;

      /// \brief Reads the id image back, converts every pixel to a
      /// temperature value and sends the image to the new frame subscribers.
      /// Does nothing without subscribers.
      public: virtual void PostRender() override;

      // Documentation inherited
      public: virtual common::ConnectionPtr ConnectNewThermalFrame(
          std::function<void(const uint16_t *, unsigned int, unsigned int,
          unsigned int, const std::string &)> _subscriber) override;

      // Documentation inherited
      public: virtual void Destroy() override;

      /// \brief Output format, L16 (default) or L8. The render target
      /// itself holds RGB8 ids.
      /// \return The output format.
      public: virtual PixelFormat ImageFormat() const override;

      /// \brief Selects the output format: L8 for PF_L8, L16 for any other
      /// format, as ogre2 only has 8 and 16 bit outputs.
      /// \param[in] _format Requested output format.
      /// \param[in] _reinterpretable Ignored.
      public: virtual void SetImageFormat(PixelFormat _format,
                  bool _reinterpretable = false) override;

      // Documentation inherited
      protected: virtual void Init() override;

      // Documentation inherited
      protected: virtual RenderTargetPtr RenderTarget() const override;

      /// \brief Output value of a pixel whose color pass value in ogre2
      /// would be _rgba (8 bit), see thermal_camera_fs.glsl
      /// \param[in] _rgba Color pass value as red, green, blue, alpha, each
      /// in [0, 1]. If green, blue and alpha are all 0, red is the
      /// normalized temperature of a heat source; otherwise the luminance of
      /// the color selects a temperature within the ambient range.
      /// \return Output value of the pixel, in [0, 255] for an L8 image and
      /// [0, 65535] for an L16 image.
      private: uint16_t Encode(const float _rgba[4]) const;

      /// \brief Instance drawing diffuse x _texture, unlit, alpha 0
      /// \param[in] _texture Diffuse texture of the material.
      /// \param[in] _diffuse Diffuse color, multiplied with the texture.
      /// \return The instance, cached per texture and diffuse color. Null
      /// if the gz_unlit material can not be loaded.
      private: filament::MaterialInstance *TexturedInstance(
          filament::Texture *_texture, const math::Color &_diffuse);

      /// \brief Heat signature instance for _visual, whose "temperature"
      /// user data is the texture _texture; nullptr if it cannot be loaded
      /// \param[in] _visual Visual whose "minTemp" and "maxTemp" user data
      /// give the temperature range, when both are floats.
      /// \param[in] _texture Heat signature texture, as given by the
      /// visual's "temperature" user data.
      /// \return The instance, cached per texture, temperature range, bit
      /// depth and resolution. Null if the texture or the gz_heat_signature
      /// material can not be loaded.
      private: filament::MaterialInstance *HeatSignatureInstance(
          const VisualPtr &_visual, const std::string &_texture);

      /// \brief gz_flat instances by id (0 is the background), the id as
      /// the colour 0xRRGGBB
      private: std::unordered_map<uint32_t, filament::MaterialInstance *>
          idInstances;

      /// \brief Heat signature textures (R8, one level, linear) by
      /// "temperature" user data string
      private: std::map<std::string, filament::Texture *> heatTextures;

      /// \brief Meshes Render overrode this frame, reused
      private: std::vector<FilamentMesh *> overridden;

      /// \brief Heat signature instances keyed by (path, min, max,
      /// bit depth, resolution)
      private: std::map<std::tuple<std::string, float, float, int, float>,
          filament::MaterialInstance *> heatInstances;

      /// \brief Textured objects that are not heat sources: unlit
      /// diffuse x texture with alpha 0, keyed by (texture, packed color)
      private: std::map<std::pair<const void *, uint32_t>,
          filament::MaterialInstance *> texturedInstances;

      /// \brief RGBA readback buffer, reused
      private: std::vector<uint8_t> rgbaRaw;

      /// \brief Output value per id, index 0 is the background
      private: std::vector<uint16_t> idValue;

      /// \brief Thermal image sent to subscribers, one value per pixel.
      private: std::vector<uint16_t> thermalImage;

      /// \brief True once Render has set the format and size of the render
      /// target.
      private: bool textureCreated{false};

      /// \brief Output format, PF_L16 or PF_L8.
      private: PixelFormat format{PF_L16};

      /// \brief Event triggered with every new thermal image.
      private: common::EventT<void(const uint16_t *, unsigned int,
          unsigned int, unsigned int, const std::string &)> newThermalFrame;

      /// \brief Only the scene can create this class.
      private: friend class FilamentScene;
    };
    }
  }
}
#endif
