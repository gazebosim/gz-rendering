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
    /// \brief Filament implementation of the camera class.
    class GZ_RENDERING_FILAMENT_VISIBLE FilamentCamera :
      public BaseCamera<FilamentSensor>
    {
      /// \brief Constructor
      protected: FilamentCamera() = default;

      /// \brief Destructor
      public: virtual ~FilamentCamera() = default;

      // Documentation inherited
      public: virtual void Render() override;

      // Documentation inherited
      public: virtual void Destroy() override;

      /// \brief Get the filament view that renders this camera.
      /// \return The filament view, null before Init and after Destroy.
      public: filament::View *View() const;

      /// \brief Pick the visual under a pixel through filament::View::pick.
      /// Renders one frame.
      /// \param[in] _mousePos Pixel position, origin at the top left.
      /// \return The visual whose mesh is under the pixel, or null if there
      /// is none or the position is outside the image.
      public: virtual VisualPtr VisualAt(const math::Vector2i &_mousePos)
          override;

      /// \brief Get the underlying filament camera.
      /// \return The filament camera, null before Init and after Destroy.
      public: filament::Camera *Native() const;

      /// \brief The colour target's GL name in the caller's context when the
      /// engine shares it (FilamentRenderEngine::SharesCallerGLContext),
      /// else 0: the caller can't sample it, GUIs fall back to Copy().
      /// \return The GL texture name, or 0.
      public: virtual unsigned int RenderTextureGLId() const override;

      /// \brief Like ogre2: the custom matrix as set, else the frustum
      /// that is rendered.
      /// \return The projection matrix.
      public: virtual math::Matrix4d ProjectionMatrix() const override;

      /// \brief Set a custom projection matrix, rendered from the next frame
      /// on. See ogre2's setCustomProjectionMatrix: rows 0, 1 and 3 are used
      /// as given, the depth row encodes the camera's clip planes.
      /// \param[in] _matrix The projection matrix.
      public: virtual void SetProjectionMatrix(const math::Matrix4d &_matrix)
          override;

      /// \brief Set the projection type. RGB only, like ogre2: sensor
      /// subclasses resolve SetProjectionType to BaseCamera<FilamentCamera>
      /// (type stored, rendering stays perspective). Clears a custom matrix.
      /// \param[in] _type Perspective or orthographic projection.
      public: virtual void SetProjectionType(CameraProjectionType _type)
          override;

      /// \brief Get the render target of this camera: its colour render
      /// texture.
      protected: virtual RenderTargetPtr RenderTarget() const override;

      /// \brief Create the filament camera and view, then the render texture.
      protected: virtual void Init() override;

      /// \brief Create the colour render texture (R8G8B8, image size).
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

      /// \brief Whether the projection type is CPT_ORTHOGRAPHIC (only the
      /// RGB camera can be set to it, see SetProjectionType).
      private: bool Orthographic() const;

      /// \brief Quality mode, decided at the first render: the GUI camera
      /// (user data "user-camera", set by gz-gui before its first render)
      /// gets post-processing: tone mapping, GTAO, MSAA or FXAA, dithering.
      /// Sensors stay without post-processing (render target MSAA, plain
      /// values): its cost is per camera.
      protected: void UpdateQualityMode();

      /// \brief UpdateQualityMode ran
      private: bool qualityChecked{false};

      /// \brief Tone mapping LUT of the quality mode, null when off
      private: filament::ColorGrading *colorGrading{nullptr};

      /// \brief SetProjectionMatrix() was called (ogre2's
      /// isCustomProjectionMatrixEnabled)
      protected: bool customProjection{false};

      /// \brief Filament view of this camera, owned (destroyed in Destroy).
      protected: filament::View *view{nullptr};

      /// \brief Filament camera component of cameraEntity.
      protected: filament::Camera *camera{nullptr};

      /// \brief Child of the node entity, rotated into filament's camera
      /// convention (-Z forward, Y up)
      protected: ::utils::Entity cameraEntity;

      /// \brief Colour render texture the view renders into.
      protected: FilamentRenderTexturePtr renderTexture;

      /// \brief Where the GUI camera's scene reflections are rendered from
      /// (FilamentScene::BeginReflections): 1 m short of what it looks at
      private: math::Vector3d ReflectionProbePosition();

      /// \brief Camera pose of the last ReflectionProbePosition
      private: math::Pose3d probeCameraPose{math::Pose3d::Zero};

      /// \brief Last ReflectionProbePosition
      private: math::Vector3d probePosition;

      private: friend class FilamentScene;
    };
    }
  }
}
#endif
