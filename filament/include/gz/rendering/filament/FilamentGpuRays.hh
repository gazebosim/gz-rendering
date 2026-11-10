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
#ifndef GZ_RENDERING_FILAMENT_FILAMENTGPURAYS_HH_
#define GZ_RENDERING_FILAMENT_FILAMENTGPURAYS_HH_

#include <array>
#include <string>
#include <vector>

#include <gz/common/Event.hh>
#include <utils/Entity.h>

#include "gz/rendering/config.hh"
#include "gz/rendering/base/BaseGpuRays.hh"
#include "gz/rendering/filament/FilamentRenderTypes.hh"
#include "gz/rendering/filament/FilamentSensor.hh"
#include "gz/rendering/filament/Export.hh"

namespace filament
{
  class Camera;
  class IndexBuffer;
  class MaterialInstance;
  class Scene;
  class Texture;
  class VertexBuffer;
  class View;
}

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    //
    /// \brief GPU lidar: renders the cube faces the rays need with the
    /// linear-depth material into one atlas, then a second pass samples one
    /// texel per ray on the GPU so only the scan is read back (like ogre2).
    /// Channels: range, retro (always 0), 0.
    class GZ_RENDERING_FILAMENT_VISIBLE FilamentGpuRays :
      public BaseGpuRays<FilamentSensor>
    {
      protected: FilamentGpuRays();

      public: virtual ~FilamentGpuRays();

      public: virtual void Render() override;

      public: virtual void PostRender() override;

      public: virtual void Destroy() override;

      public: virtual const float *Data() const override;

      public: virtual void Copy(float *_dataDest) override;

      public: virtual common::ConnectionPtr ConnectNewGpuRaysFrame(
                  std::function<void(const float *_frame, unsigned int _width,
                  unsigned int _height, unsigned int _depth,
                  const std::string &_format)> _subscriber) override;

      public: virtual RenderTargetPtr RenderTarget() const override;

      protected: virtual void Init() override;

      /// \brief Recompute ray directions and the faces they hit when the
      /// scan configuration changed
      private: void UpdateRays();

      /// \brief Per ray: cube face, texel index in the atlas, 1 / cos(angle
      /// between the texel center direction and the face axis): depth to
      /// range
      private: struct Ray { int face; unsigned int texel; float invCos; };

      private: std::vector<Ray> rays;

      /// \brief Configuration the rays were computed for
      private: std::array<double, 8> rayKey{};

      private: unsigned int faceRes{0u};

      private: std::array<bool, 6> faceUsed{};

      /// \brief Create the second-pass objects (scene, view, fullscreen
      /// triangle, material)
      private: void InitSamplePass();

      /// \brief Upload the per-ray atlas texel and 1/cos to rayTexture
      private: void UploadRays();

      /// \brief Range per ray, read back from the second pass
      private: std::vector<float> ranges;

      /// \brief Half resolution particle depth of the six faces, reused
      private: std::vector<float> particleDepth;

      /// \brief Second pass: its own scene holding a fullscreen triangle
      private: filament::Scene *sampleScene{nullptr};

      private: filament::View *sampleView{nullptr};

      private: ::utils::Entity sampleCameraEntity;

      private: ::utils::Entity triangle;

      private: filament::VertexBuffer *triangleVb{nullptr};

      private: filament::IndexBuffer *triangleIb{nullptr};

      private: filament::MaterialInstance *sampleInstance{nullptr};

      /// \brief RGBA32F, one texel per ray: atlas x, atlas y, 1/cos
      private: filament::Texture *rayTexture{nullptr};

      /// \brief R32F, one texel per ray: the range
      private: FilamentRenderTexturePtr scanTexture;

      private: unsigned int faceCount{0u};

      private: std::vector<float> scan;

      private: ::utils::Entity cameraEntity;

      private: filament::Camera *camera{nullptr};

      private: filament::View *view{nullptr};

      private: filament::MaterialInstance *depthInstance{nullptr};

      private: FilamentRenderTexturePtr renderTexture;

      private: common::EventT<void(const float *, unsigned int, unsigned int,
          unsigned int, const std::string &)> newFrame;

      private: friend class FilamentScene;
    };
    }
  }
}
#endif
