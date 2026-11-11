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
#ifndef GZ_RENDERING_FILAMENT_FILAMENTBOUNDINGBOXCAMERA_HH_
#define GZ_RENDERING_FILAMENT_FILAMENTBOUNDINGBOXCAMERA_HH_

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <gz/common/Event.hh>

#include "gz/rendering/base/BaseBoundingBoxCamera.hh"
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
    class FilamentMesh;

    /// \brief Bounding box camera. Renders every mesh with a flat color that
    /// encodes (mesh index, label), exactly like ogre2's material switcher,
    /// then computes the boxes on the CPU with ogre2's algorithms.
    class GZ_RENDERING_FILAMENT_VISIBLE FilamentBoundingBoxCamera :
      public BaseBoundingBoxCamera<FilamentCamera>
    {
      protected: FilamentBoundingBoxCamera();

      public: virtual ~FilamentBoundingBoxCamera();

      // BaseCamera<FilamentCamera> would otherwise win over FilamentCamera
      public: virtual void SetProjectionMatrix(const math::Matrix4d &_matrix)
          override { FilamentCamera::SetProjectionMatrix(_matrix); }

      public: virtual math::Matrix4d ProjectionMatrix() const override
          { return FilamentCamera::ProjectionMatrix(); }

      public: virtual void Render() override;

      public: virtual void PostRender() override;

      public: virtual common::ConnectionPtr ConnectNewBoundingBoxes(
          std::function<void(const std::vector<BoundingBox> &)> _subscriber)
          override;

      public: virtual void DrawBoundingBox(unsigned char *_data,
          const math::Color &_color, const BoundingBox &_box) const override;

      public: virtual void Destroy() override;

      protected: virtual void Init() override;

      protected: virtual RenderTargetPtr RenderTarget() const override;

      /// \brief One rendered mesh: what ogre2 calls an item.
      private: struct Item
      {
        FilamentMesh *mesh{nullptr};
        VisualPtr visual;
        std::string parentName;
      };

      private: void VisibleBoundingBoxes();

      private: void FullBoundingBoxes();

      private: void BoundingBoxes3D();

      /// \brief Label of every item with a visible pixel by item index,
      /// 0xffffffff for the others
      private: std::vector<uint32_t> MarkVisibleBoxes() const;

      /// \brief gz_flat instances by colour (0xRRGGBB)
      private: std::unordered_map<uint32_t, filament::MaterialInstance *>
          flatInstances;

      /// \brief Items of the last render, indexed by their encoded id
      private: std::vector<Item> items;

      /// \brief Last ID image, RGB8: R,G = item index (low, high), B = label
      private: std::vector<uint8_t> buffer;

      /// \brief Readback image of PostRender, reused
      private: Image image;

      private: common::EventT<void(const std::vector<BoundingBox> &)>
          newBoundingBoxes;

      private: friend class FilamentScene;
    };
    }
  }
}
#endif
