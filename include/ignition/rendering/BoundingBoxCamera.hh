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
#ifndef IGNITION_RENDERING_BoundingBoxCAMERA_HH_
#define IGNITION_RENDERING_BoundingBoxCAMERA_HH_

#include <string>
#include <vector>
#include <cstdint>

#include <ignition/common/Event.hh>
#include "ignition/rendering/Camera.hh"


namespace ignition
{
  namespace rendering
  {
    /// \brief BoundingBox types for Visible / Full BoundingBox
    /// FullBox: Shows the full box of occluded objects
    /// VisibleBox: Shows the visible part of the occluded object
    enum BoundingBoxType {
      FullBox,
      VisibleBox
    };

    /// \brief Bounding box structure to store its boundaries
    /// min & max of x & y, and the label
    struct BoundingBox
    {
      uint32_t minX;
      uint32_t minY;
      uint32_t maxX;
      uint32_t maxY;
      uint32_t label;
    };

    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
    /// \class Camera Camera.hh ignition/rendering/Camera.hh
    /// \brief Poseable BoundingBox camera used for rendering the scene graph.
    /// This camera is designed to produced BoundingBox data, instead of a 2D
    /// image.
    class IGNITION_RENDERING_VISIBLE BoundingBoxCamera :
      public virtual Camera,
      public virtual Sensor
    {
      /// \brief Destructor
      public: virtual ~BoundingBoxCamera() { }

      /// \brief Get the BoundingBox data
      /// \return Buffer of bounding boxes info (label, minX, minY, maxX, maxY)
      public: virtual std::vector<BoundingBox> BoundingBoxData() const = 0;

      /// \brief Connect to the new BoundingBox info
      /// \param[in] _subscriber Subscriber callback function
      /// \return Pointer to the new Connection. This must be kept in scope
      public: virtual ignition::common::ConnectionPtr ConnectNewBoundingBoxes(
        std::function<void(const std::vector<BoundingBox> &)>  _subscriber) = 0;

      /// \brief Set BoundingBox Type (Visible / Full)
      /// \param[in] _type BoundingBox Type (Visible / Full)
      public: virtual void SetBoundingBoxType(BoundingBoxType _type) = 0;

      /// \brief Draw a bounding box on the givin image in green color
      /// \param[in] data buffer contains the image data
      /// \param[in] box bounding box to draw
      public: virtual void DrawBoundingBox(unsigned char *_data,
        BoundingBox &_box) = 0;
    };
  }
  }
}
#endif
