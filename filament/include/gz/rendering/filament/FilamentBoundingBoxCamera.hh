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
    /// encodes (mesh index, label), then computes the boxes on the CPU.
    class GZ_RENDERING_FILAMENT_VISIBLE FilamentBoundingBoxCamera :
      public BaseBoundingBoxCamera<FilamentCamera>
    {
      /// \brief Constructor
      protected: FilamentBoundingBoxCamera() = default;

      /// \brief Destructor
      public: virtual ~FilamentBoundingBoxCamera() = default;

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

      /// \brief Renders the id image: every mesh with a flat color that
      /// encodes its index and label.
      public: virtual void Render() override;

      /// \brief Reads the id image back, computes the boxes of the camera's
      /// type and sends them to the new frame subscribers. Does nothing
      /// without subscribers.
      public: virtual void PostRender() override;

      // Documentation inherited
      public: virtual common::ConnectionPtr ConnectNewBoundingBoxes(
          std::function<void(const std::vector<BoundingBox> &)> _subscriber)
          override;

      // Documentation inherited
      public: virtual void DrawBoundingBox(unsigned char *_data,
          const math::Color &_color, const BoundingBox &_box) const override;

      // Documentation inherited
      public: virtual void Destroy() override;

      // Documentation inherited
      protected: virtual void Init() override;

      // Documentation inherited
      protected: virtual RenderTargetPtr RenderTarget() const override;

      /// \brief One rendered mesh: what ogre2 calls an item.
      private: struct Item
      {
        /// \brief Mesh drawn with this item's id.
        FilamentMesh *mesh{nullptr};
        /// \brief Visual that owns the mesh.
        VisualPtr visual;
        /// \brief Name of the top level visual (child of the scene root)
        /// that visual belongs to.
        std::string parentName;
      };

      /// \brief Fills boundingBoxes with the 2D box of the visible pixels
      /// of each top level model.
      private: void VisibleBoundingBoxes();

      /// \brief Fills boundingBoxes with the 2D box of each visible top
      /// level model, computed from all its vertices (occluded parts
      /// included).
      private: void FullBoundingBoxes();

      /// \brief Fills boundingBoxes with the oriented 3D box of each
      /// visible top level model, in the camera frame.
      private: void BoundingBoxes3D();

      /// \brief Name of the top level visual of an item, to merge the
      /// boxes of multi-link models
      private: std::string ParentName(uint32_t _itemId) const;

      /// \brief gz_flat instances by colour (0xRRGGBB)
      private: std::unordered_map<uint32_t, filament::MaterialInstance *>
          flatInstances;

      /// \brief Items of the last render, indexed by their encoded id
      private: std::vector<Item> items;

      /// \brief Last ID image, RGB8: R,G = item index (low, high), B = label
      private: Image image;

      /// \brief Event triggered with every new set of boxes.
      private: common::EventT<void(const std::vector<BoundingBox> &)>
          newBoundingBoxes;

      /// \brief Only the scene can create this class.
      private: friend class FilamentScene;
    };
    }
  }
}
#endif
