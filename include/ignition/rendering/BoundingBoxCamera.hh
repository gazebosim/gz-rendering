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
#ifndef IGNITION_RENDERING_BOUNDINGBOXCAMERA_HH_
#define IGNITION_RENDERING_BOUNDINGBOXCAMERA_HH_

#include <string>
#include <vector>
#include <cstdint>

#include <ignition/common/Event.hh>
#include <ignition/math/Vector3.hh>
#include "ignition/rendering/Camera.hh"

namespace ignition
{
  namespace rendering
  {
    /// \brief BoundingBox types for Visible / Full 2D Boxes / 3D Boxes
    enum class BoundingBoxType {

      /// 2D box that shows the full box of occluded objects
      BBT_FULLBOX2D = 0,

      /// 2D box that shows the visible part of the
      /// occluded object
      BBT_VISIBLEBOX2D = 1,

      /// 3D oriented box
      BBT_BOX3D = 2
    };

    /// \brief Bounding box for both 2d & 3d boxes, it stores the
    /// position / orientation / size info of the box and its label
    struct BoundingBox
    {
      /// \brief Box type
      ignition::rendering::BoundingBoxType type;

      /// \brief Center of the box in pixel coord in 2D, and camera coord in 3D
      ignition::math::Vector3d center;

      /// \brief Size of the box (width, height, depth), depth = 0 in 2D boxes
      ignition::math::Vector3d size;

      /// \brief Orientation of the 3D box in camera coord.
      /// The 2D boxes are axis aligned (orientation = 0)
      ignition::math::Quaterniond orientation;

      /// \brief Label of the annotated object inside the box
      uint32_t label;

      /// \brief Constructor
      BoundingBox()
      {
        this->label = 0u;
      };

      /// \brief Constructor
      /// \param[in] _type Bounding Box Type
      explicit BoundingBox(ignition::rendering::BoundingBoxType _type)
      {
        this->label = 0u;
        this->type = _type;
      }

      std::vector<math::Vector3d> Vertices() const
      {
        /*
            1 -------- 0
           /|         /|
          2 -------- 3 .
          | |        | |
          . 5 -------- 4
          |/         |/
          6 -------- 7
        */

        std::vector<math::Vector3d> vertices;
        auto w = size.X();
        auto h = size.Y();
        auto l = size.Z();

        // 8 vertices | box corners
        vertices.push_back(math::Vector3d(w/2, h/2, l/2));
        vertices.push_back(math::Vector3d(-w/2, h/2, l/2));
        vertices.push_back(math::Vector3d(-w/2, h/2, -l/2));
        vertices.push_back(math::Vector3d(w/2, h/2, -l/2));
        vertices.push_back(math::Vector3d(w/2, -h/2, l/2));
        vertices.push_back(math::Vector3d(-w/2, -h/2, l/2));
        vertices.push_back(math::Vector3d(-w/2, -h/2, -l/2));
        vertices.push_back(math::Vector3d(w/2, -h/2, -l/2));

        // Transform
        for (auto &vertex : vertices)
        {
          vertex = this->orientation * vertex + this->center;
        }

        return vertices;
      }
    };

    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
    /// \class BoundingBoxCamera BoundingBoxCamera.hh
    /// ignition/rendering/BoundingBoxCamera.hh
    /// \brief Poseable BoundingBox camera used for rendering the scene graph.
    /// This camera is designed to produced BoundingBox data, instead of a 2D
    /// image.
    class IGNITION_RENDERING_VISIBLE BoundingBoxCamera :
      public virtual Camera,
      public virtual Sensor
    {
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

      /// \brief Get the BoundingBox Type (Visible / Full)
      /// \return BoundingBox Type (Visible / Full)
      public: virtual BoundingBoxType Type() const = 0;

      /// \brief Draw a bounding box on the given image in green color
      /// \param[in] data buffer contains the image data
      /// \param[in] box bounding box to be drawn
      public: virtual void DrawBoundingBox(unsigned char *_data,
        const BoundingBox &_box) = 0;
    };
  }
  }
}
#endif
