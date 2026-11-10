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
#ifndef GZ_RENDERING_FILAMENT_FILAMENTCAMERA_HH_
#define GZ_RENDERING_FILAMENT_FILAMENTCAMERA_HH_

#include <utils/Entity.h>

#include "gz/rendering/config.hh"
#include "gz/rendering/base/BaseCamera.hh"
#include "gz/rendering/filament/FilamentRenderTarget.hh"
#include "gz/rendering/filament/FilamentRenderTypes.hh"
#include "gz/rendering/filament/FilamentSensor.hh"
#include "gz/rendering/filament/Export.hh"

namespace filament
{
  class Camera;
  class ColorGrading;
  class View;
}

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    //
    class GZ_RENDERING_FILAMENT_VISIBLE FilamentCamera :
      public BaseCamera<FilamentSensor>
    {
      protected: FilamentCamera();

      public: virtual ~FilamentCamera();

      public: virtual void Render() override;

      public: virtual void Destroy() override;

      public: filament::View *View() const;

      /// \brief Picking through filament::View::pick (renders one frame)
      public: virtual VisualPtr VisualAt(const math::Vector2i &_mousePos)
          override;

      public: filament::Camera *Native() const;

      /// \brief The colour target's GL name in the caller's context when the
      /// engine shares it (FilamentRenderEngine::SharesCallerGLContext),
      /// else 0: the caller can't sample it, GUIs fall back to Copy().
      public: virtual unsigned int RenderTextureGLId() const override;

      /// \brief Like ogre2: the custom matrix as set, else the frustum
      /// that is rendered
      public: virtual math::Matrix4d ProjectionMatrix() const override;

      /// \brief Rendered from the next frame on, see ogre2's
      /// setCustomProjectionMatrix: rows 0, 1 and 3 are used as given,
      /// the depth row encodes the camera's clip planes
      public: virtual void SetProjectionMatrix(const math::Matrix4d &_matrix)
          override;

      /// \brief RGB only, like ogre2: sensor subclasses resolve
      /// SetProjectionType to BaseCamera<FilamentCamera> (type stored,
      /// rendering stays perspective). Clears a custom matrix.
      public: virtual void SetProjectionType(CameraProjectionType _type)
          override;

      protected: virtual RenderTargetPtr RenderTarget() const override;

      protected: virtual void Init() override;

      protected: virtual void CreateRenderTexture();

      /// \brief Render with [_renderNear, _renderFar] but cull against the
      /// camera's own clip planes. With a closer near plane this emulates
      /// ogre2's depth clamp: geometry crossing the near plane is drawn,
      /// objects entirely in front of it are culled.
      protected: void SetClampedProjection(double _renderNear,
                     double _renderFar);

      /// \brief Push HFOV / aspect / near / far (or the custom /
      /// orthographic projection) into the filament camera
      protected: void UpdateProjection();

      /// \brief _matrix with the depth rows replaced to encode [_near, _far],
      /// perspective or orthographic depending on _matrix's last row
      protected: static math::Matrix4d WithDepthRows(math::Matrix4d _matrix,
                     double _near, double _far);

      /// \brief Set to CPT_ORTHOGRAPHIC (RGB camera only, see
      /// SetProjectionType)
      private: bool Orthographic() const;

      /// \brief Quality mode (default; GZ_FILAMENT_QUALITY=0 keeps ogre2's
      /// look), decided at the first render. Every RGB view (GUI, RGB camera
      /// sensors, the depth camera's RGBD colour pass) gets Filament's stock
      /// diffuse. The GUI camera (user data "user-camera", set by gz-gui
      /// before its first render) also gets post-processing: tone mapping,
      /// GTAO, MSAA or FXAA, dithering. Sensors stay without post-processing
      /// (render target MSAA, plain values): its cost is per camera. Depth,
      /// thermal, segmentation, bounding box, wide angle and lidar cameras
      /// never call it.
      protected: void UpdateQualityMode();

      /// \brief UpdateQualityMode ran
      private: bool qualityChecked{false};

      /// \brief Tone mapping LUT of the quality mode, null when off
      private: filament::ColorGrading *colorGrading{nullptr};

      /// \brief SetProjectionMatrix() was called (ogre2's
      /// isCustomProjectionMatrixEnabled)
      protected: bool customProjection{false};

      protected: filament::View *view{nullptr};

      protected: filament::Camera *camera{nullptr};

      /// \brief Child of the node entity, rotated into filament's camera
      /// convention (-Z forward, Y up)
      protected: ::utils::Entity cameraEntity;

      protected: FilamentRenderTexturePtr renderTexture;

      private: friend class FilamentScene;
    };
    }
  }
}
#endif
