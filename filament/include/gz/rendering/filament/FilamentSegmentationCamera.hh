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
#ifndef GZ_RENDERING_FILAMENT_FILAMENTSEGMENTATIONCAMERA_HH_
#define GZ_RENDERING_FILAMENT_FILAMENTSEGMENTATIONCAMERA_HH_

#include <map>
#include <random>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include <gz/common/Event.hh>

#include "gz/rendering/base/BaseSegmentationCamera.hh"
#include "gz/rendering/filament/FilamentCamera.hh"
#include "gz/rendering/filament/FilamentRenderTypes.hh"
#include "gz/rendering/filament/Export.hh"

namespace filament
{
  class MaterialInstance;
}

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    //
    /// \brief Segmentation camera: every mesh is drawn with a flat label
    /// color into a linear RGB8 target. Labels, instance counting and colors
    /// follow Ogre2SegmentationMaterialSwitcher so outputs are comparable.
    class GZ_RENDERING_FILAMENT_VISIBLE FilamentSegmentationCamera :
      public BaseSegmentationCamera<FilamentCamera>
    {
      protected: FilamentSegmentationCamera();

      public: virtual ~FilamentSegmentationCamera();

      // BaseCamera<FilamentCamera> would otherwise win over FilamentCamera
      public: virtual void SetProjectionMatrix(const math::Matrix4d &_matrix)
          override { FilamentCamera::SetProjectionMatrix(_matrix); }

      public: virtual math::Matrix4d ProjectionMatrix() const override
          { return FilamentCamera::ProjectionMatrix(); }

      public: virtual void CreateSegmentationTexture() override;

      public: virtual void Render() override;

      public: virtual void PostRender() override;

      public: virtual uint8_t *SegmentationData() const override;

      public: virtual common::ConnectionPtr ConnectNewSegmentationFrame(
          std::function<void(const uint8_t *, unsigned int, unsigned int,
          unsigned int, const std::string &)> _subscriber) override;

      public: virtual void SetBackgroundLabel(int _label) override;

      public: virtual void LabelMapFromColoredBuffer(
          uint8_t *_labelBuffer) const override;

      public: virtual void Destroy() override;

      protected: virtual void Init() override;

      protected: virtual RenderTargetPtr RenderTarget() const override;

      /// \brief Label color of a visual, see Ogre2's ColorForVisual
      private: math::Color ColorForVisual(const VisualPtr &_visual,
          std::string &_prevParentName);

      /// \brief Seeded random color of a label, see Ogre2's LabelToColor
      private: math::Color LabelToColor(int64_t _label,
          bool _isMultiLink = false);

      /// \brief Shared material instance for a flat color
      private: filament::MaterialInstance *FlatInstance(
          const math::Color &_color);

      private: std::unordered_map<uint32_t, filament::MaterialInstance *>
          flatInstances;

      private: bool textureCreated{false};

      private: std::vector<uint8_t> buffer;

      private: std::default_random_engine generator;

      private: std::map<int64_t, int64_t> colorToLabel;

      private: std::set<int64_t> takenColors;

      private: std::set<int64_t> coloredLabel;

      private: std::map<int, int> instancesCount;

      private: common::EventT<void(const uint8_t *, unsigned int,
          unsigned int, unsigned int, const std::string &)>
          newSegmentationFrame;

      private: friend class FilamentScene;
    };
    }
  }
}
#endif
