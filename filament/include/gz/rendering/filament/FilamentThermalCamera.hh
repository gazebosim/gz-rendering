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
    /// \brief Thermal camera. Every submesh is drawn with a flat id color;
    /// the temperature of each id is computed on the CPU with the same
    /// formula and 8 bit intermediate quantization as ogre2's thermal
    /// shader, so outputs are comparable. Heat signature textures (a string
    /// "temperature" with "minTemp" / "maxTemp") are mapped on the GPU like
    /// ogre2's heat_signature_fs.glsl.
    class GZ_RENDERING_FILAMENT_VISIBLE FilamentThermalCamera :
      public BaseThermalCamera<FilamentCamera>
    {
      protected: FilamentThermalCamera();

      public: virtual ~FilamentThermalCamera();

      // BaseCamera<FilamentCamera> would otherwise win over FilamentCamera
      public: virtual void SetProjectionMatrix(const math::Matrix4d &_matrix)
          override { FilamentCamera::SetProjectionMatrix(_matrix); }

      public: virtual math::Matrix4d ProjectionMatrix() const override
          { return FilamentCamera::ProjectionMatrix(); }

      public: virtual void Render() override;

      public: virtual void PostRender() override;

      public: virtual common::ConnectionPtr ConnectNewThermalFrame(
          std::function<void(const uint16_t *, unsigned int, unsigned int,
          unsigned int, const std::string &)> _subscriber) override;

      public: virtual void Destroy() override;

      /// \brief Output format, L16 (default) or L8. The render target
      /// itself holds RGB8 ids.
      public: virtual PixelFormat ImageFormat() const override;

      public: virtual void SetImageFormat(PixelFormat _format,
                  bool _reinterpretable = false) override;

      protected: virtual void Init() override;

      protected: virtual RenderTargetPtr RenderTarget() const override;

      /// \brief Output value of a pixel whose color pass value in ogre2
      /// would be _rgba (8 bit), see thermal_camera_fs.glsl
      private: uint16_t Encode(const float _rgba[4]) const;

      /// \brief Instance drawing diffuse x _texture, unlit, alpha 0
      private: filament::MaterialInstance *TexturedInstance(
          filament::Texture *_texture, const math::Color &_diffuse);

      /// \brief Heat signature instance for _visual, whose "temperature"
      /// user data is the texture _texture; nullptr if it cannot be loaded
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

      private: std::vector<uint16_t> thermalImage;

      private: bool textureCreated{false};

      private: PixelFormat format{PF_L16};

      private: common::EventT<void(const uint16_t *, unsigned int,
          unsigned int, unsigned int, const std::string &)> newThermalFrame;

      private: friend class FilamentScene;
    };
    }
  }
}
#endif
