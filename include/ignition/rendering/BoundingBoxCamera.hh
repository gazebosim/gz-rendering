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

#include <cstdint>
#include <vector>

#include <ignition/common/Event.hh>
#include <ignition/math/Color.hh>
#include <ignition/math/Vector3.hh>

#include "ignition/rendering/Camera.hh"

namespace ignition
{
  namespace rendering
  {
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

    /// \brief 2D or 3D Bounding box. It stores the
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
      /// \param[in] _type Bounding Box Type
      /// \param[in] _label Label for the object in the bounding box
      BoundingBox(ignition::rendering::BoundingBoxType _type, uint32_t _label)
      {
        this->type = _type;
        this->label = _label;
      }

      /// \brief Get the vertices of the 3D bounding box. If the bounding box
      /// type isn't 3D, an empty vector is returned
      /// \return The vertices of the 3D bounding box
      std::vector<math::Vector3d> Vertices() const
      {
        if (this->type != BoundingBoxType::BBT_BOX3D)
          return {};

        /*
          The numbers listed here are the corresponding indices in the vector
          that is returned by this method

            1 -------- 0
           /|         /|
          2 -------- 3 .
          | |        | |
          . 5 -------- 4
          |/         |/
          6 -------- 7
        */

        std::vector<math::Vector3d> vertices;
        auto w = this->size.X();
        auto h = this->size.Y();
        auto l = this->size.Z();

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
    /// \brief Poseable BoundingBox camera used for rendering bounding boxes of
    /// objects in the scene.
    class IGNITION_RENDERING_VISIBLE BoundingBoxCamera :
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
      public: virtual ignition::common::ConnectionPtr ConnectNewBoundingBoxes(
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
