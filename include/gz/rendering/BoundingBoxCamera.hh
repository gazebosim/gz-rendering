/*
 * Copyright (C) 2021 Open Source Robotics Foundation
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
#ifndef GZ_RENDERING_BOUNDINGBOXCAMERA_HH_
#define GZ_RENDERING_BOUNDINGBOXCAMERA_HH_

#include <cstdint>
#include <vector>

#include <gz/common/Event.hh>
#include <gz/math/Color.hh>
#include <gz/math/Vector3.hh>

#include "gz/rendering/BoundingBox.hh"
#include "gz/rendering/Camera.hh"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    //
    /// \brief BoundingBox types for Visible / Full 2D Boxes / 3D Boxes
    enum class BoundingBoxType
    {
      /// 2D box that shows the full box of occluded objects
      BBT_FULLBOX2D = 0,

      /// 2D box that shows the visible part of the
      /// occluded object
      BBT_VISIBLEBOX2D = 1,

      /// 3D oriented box
      BBT_BOX3D = 2
    };

    /// \class BoundingBoxCamera BoundingBoxCamera.hh
    /// gz/rendering/BoundingBoxCamera.hh
    /// \brief Poseable BoundingBox camera used for rendering bounding boxes of
    /// objects in the scene.
    class GZ_RENDERING_VISIBLE BoundingBoxCamera :
      public virtual Camera
    {
      /// \brief Destructor
      public: virtual ~BoundingBoxCamera() { }

      /// \brief Get the BoundingBox data
      /// \return Buffer of bounding boxes info (label, minX, minY, maxX, maxY)
      public: virtual const std::vector<BoundingBox> &BoundingBoxData()
              const = 0;

      /// \brief Connect to the new BoundingBox info
      /// \param[in] _subscriber Subscriber callback function
      /// \return Pointer to the new Connection. This must be kept in scope
      public: virtual gz::common::ConnectionPtr ConnectNewBoundingBoxes(
        std::function<void(const std::vector<BoundingBox> &)> _subscriber) = 0;

      /// \brief Set BoundingBox Type (Visible / Full)
      /// \param[in] _type BoundingBox Type (Visible / Full)
      public: virtual void SetBoundingBoxType(BoundingBoxType _type) = 0;

      /// \brief Get the BoundingBox Type (Visible / Full)
      /// \return BoundingBox Type (Visible / Full)
      public: virtual BoundingBoxType Type() const = 0;

      /// \brief Draw a bounding box on the given image
      /// \param[in] _data buffer containing the image data
      /// \param[in] _color Color of the bounding box to be drawn
      /// \param[in] _box bounding box to be drawn
      public: virtual void DrawBoundingBox(unsigned char *_data,
        const math::Color &_color, const BoundingBox &_box) const = 0;
    };
  }
  }
}
#endif
