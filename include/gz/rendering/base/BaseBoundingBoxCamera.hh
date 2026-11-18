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
#ifndef GZ_RENDERING_BASE_BASEBOUNDINGBOXCAMERA_HH_
#define GZ_RENDERING_BASE_BASEBOUNDINGBOXCAMERA_HH_

#include <algorithm>
#include <cstdlib>
#include <limits>
#include <map>
#include <string>
#include <vector>

#include <gz/common/Event.hh>
#include <gz/math/Color.hh>
#include <gz/math/Vector2.hh>
#include <gz/math/Vector4.hh>

#include "gz/rendering/base/BaseCamera.hh"
#include "gz/rendering/BoundingBoxCamera.hh"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {

    /// \brief Base implementation of the bounding box camera class: the
    /// bounding box type and the helpers engines share to find the visible
    /// items in an id map, to compute, merge and draw their boxes
    template <class T>
    class BaseBoundingBoxCamera:
      public virtual BoundingBoxCamera,
      public virtual BaseCamera<T>,
      public virtual T
    {
      /// \brief Constructor
      protected: BaseBoundingBoxCamera();

      /// \brief Destructor
      public: ~BaseBoundingBoxCamera() override;

      // Documentation inherited
      public: const std::vector<BoundingBox> &BoundingBoxData() const override;

      // Documentation inherited
      public: gz::common::ConnectionPtr ConnectNewBoundingBoxes(
        std::function<void(const std::vector<BoundingBox> &)> _subscriber)
        override = 0;

      // Documentation inherited
      public: void SetBoundingBoxType(BoundingBoxType _type) override;

      // Documentation inherited
      public: BoundingBoxType Type() const override;

      // Documentation inherited
      public: void DrawBoundingBox(unsigned char *_data,
        const math::Color &_color, const BoundingBox &_box) const override = 0;

      // The helpers below work on an id map: an RGB8 image of the camera's
      // size where every item is drawn with R = low byte and G = high byte of
      // its 16 bit id and B = its label. Background pixels have the
      // background label.

      /// \brief Find the items that are visible in an id map
      /// \param[in] _idMap The id map
      /// \param[in] _backgroundLabel Label of the background pixels
      /// \return Label of every visible item. Key: item id
      protected: std::map<uint32_t, uint32_t> VisibleBoxesLabel(
          const uint8_t *_idMap, uint32_t _backgroundLabel) const;

      /// \brief Compute the 2D box around the visible pixels of every item
      /// in an id map
      /// \param[in] _idMap The id map
      /// \param[in] _backgroundLabel Label of the background pixels
      /// \return Labelled box of every visible item. Key: item id
      protected: std::map<uint32_t, BoundingBox> VisibleBoxes2D(
          const uint8_t *_idMap, uint32_t _backgroundLabel) const;

      /// \brief Merge the 2D boxes of the items of the same model (multi
      /// links models) and append the result to the output boxes
      /// \param[in] _boxes 2D box of every item. Key: item id
      /// \param[in] _parentName Function returning the name of the model
      /// of an item id
      /// \param[in, out] _output The output boxes, reversed in the end
      protected: template <class ParentNameFn> void MergeMultiLinksModels2D(
          const std::map<uint32_t, BoundingBox> &_boxes,
          ParentNameFn _parentName, std::vector<BoundingBox> &_output) const;

      /// \brief Merge a vector of 2D boxes. Used in multi-links model.
      /// \param[in] _boxes Vector of 2D boxes
      /// \return Merged bounding box
      protected: BoundingBox MergeBoxes2D(
          const std::vector<BoundingBox> &_boxes) const;

      /// \brief Draw a 2D bounding box on the given image
      /// \param[in] _data buffer contains the image data (RGB8)
      /// \param[in] _color Color of the box
      /// \param[in] _box bounding box to draw
      protected: void DrawBoundingBox2D(unsigned char *_data,
          const math::Color &_color, const BoundingBox &_box) const;

      /// \brief Draw the edges of a 3D bounding box on the given image
      /// \param[in] _data buffer contains the image data (RGB8)
      /// \param[in] _color Color of the box
      /// \param[in] _vertices2d The 8 vertices of the box (in the order of
      /// BoundingBox::Vertices3D) projected to normalized device coordinates
      protected: void DrawBoundingBox3D(unsigned char *_data,
          const math::Color &_color,
          const std::vector<math::Vector2d> &_vertices2d) const;

      /// \brief Draw a line on the given image, using Bresenham's algorithm
      /// \param[in] _data buffer contains the image data (RGB8)
      /// \param[in] _point1 The 1st point of the line, in pixels
      /// \param[in] _point2 The 2nd point of the line, in pixels
      /// \param[in] _color Color of the line
      protected: void DrawLine(unsigned char *_data,
          const math::Vector2i &_point1, const math::Vector2i &_point2,
          const math::Color &_color) const;

      /// \brief Add a line to the viewport. If the line's endpoints are not
      /// inside the viewport, the added line will be a clipped line that fits
      /// in the viewport (Cohen-Sutherland algorithm). If the line doesn't
      /// intersect the viewport at all, it won't be added to _lines
      /// \param[in] _bounds The bounds of the viewport: xmin, ymin, xmax, ymax
      /// \param[in] _p0 The line's first endpoint
      /// \param[in] _p1 The line's second endpoint
      /// \param[out] _lines The list of lines in the viewport
      private: void AddToViewportLines(const math::Vector4d &_bounds,
          const math::Vector2d &_p0, const math::Vector2d &_p1,
          std::vector<math::Vector2d> &_lines) const;

      /// \brief The bounding box type
      protected: BoundingBoxType type = BoundingBoxType::BBT_FULLBOX2D;

      /// \brief The bounding box data
      protected: std::vector<BoundingBox> boundingBoxes;
    };

    //////////////////////////////////////////////////
    template <class T>
    BaseBoundingBoxCamera<T>::BaseBoundingBoxCamera()
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    BaseBoundingBoxCamera<T>::~BaseBoundingBoxCamera()
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    const std::vector<BoundingBox> &
    BaseBoundingBoxCamera<T>::BoundingBoxData() const
    {
      return this->boundingBoxes;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseBoundingBoxCamera<T>::SetBoundingBoxType(BoundingBoxType _type)
    {
      this->type = _type;
    }

    //////////////////////////////////////////////////
    template <class T>
    BoundingBoxType BaseBoundingBoxCamera<T>::Type() const
    {
      return this->type;
    }

    //////////////////////////////////////////////////
    template <class T>
    std::map<uint32_t, uint32_t> BaseBoundingBoxCamera<T>::VisibleBoxesLabel(
        const uint8_t *_idMap, uint32_t _backgroundLabel) const
    {
      std::map<uint32_t, uint32_t> visibleBoxesLabel;
      const size_t size =
          static_cast<size_t>(this->ImageWidth()) * this->ImageHeight() * 3u;
      // items cover runs of pixels: only look up the map when the id changes
      uint32_t prevId = std::numeric_limits<uint32_t>::max();
      for (size_t index = 0; index < size; index += 3)
      {
        const uint32_t label = _idMap[index + 2];
        if (label == _backgroundLabel)
          continue;

        // the item id is encoded in 16 bits
        const uint32_t id = _idMap[index + 1] * 256u + _idMap[index];
        if (id == prevId)
          continue;
        prevId = id;
        visibleBoxesLabel.emplace(id, label);
      }
      return visibleBoxesLabel;
    }

    //////////////////////////////////////////////////
    template <class T>
    std::map<uint32_t, BoundingBox> BaseBoundingBoxCamera<T>::VisibleBoxes2D(
        const uint8_t *_idMap, uint32_t _backgroundLabel) const
    {
      const uint32_t width = this->ImageWidth();
      const uint32_t height = this->ImageHeight();

      struct BoxBoundary
      {
        uint32_t minX;
        uint32_t minY;
        uint32_t maxX;
        uint32_t maxY;
        uint32_t label;
      };
      std::map<uint32_t, BoxBoundary> boxesBoundary;

      // find the items' boundaries
      // items cover runs of pixels: only look up the map when the id changes
      uint32_t prevId = std::numeric_limits<uint32_t>::max();
      BoxBoundary *boundary = nullptr;
      for (uint32_t y = 0; y < height; ++y)
      {
        for (uint32_t x = 0; x < width; ++x)
        {
          const size_t index = (static_cast<size_t>(y) * width + x) * 3u;
          const uint32_t label = _idMap[index + 2];
          if (label == _backgroundLabel)
            continue;

          // the item id is encoded in 16 bits
          const uint32_t id = _idMap[index + 1] * 256u + _idMap[index];
          if (id != prevId)
          {
            // a new box is labelled by its first pixel
            boundary = &boxesBoundary.try_emplace(id,
                BoxBoundary{width, height, 0, 0, label}).first->second;
            prevId = id;
          }
          boundary->minX = std::min(boundary->minX, x);
          boundary->minY = std::min(boundary->minY, y);
          boundary->maxX = std::max(boundary->maxX, x);
          boundary->maxY = std::max(boundary->maxY, y);
        }
      }

      std::map<uint32_t, BoundingBox> boxes;
      for (const auto &[id, b] : boxesBoundary)
      {
        const uint32_t boxWidth = b.maxX - b.minX;
        const uint32_t boxHeight = b.maxY - b.minY;
        BoundingBox &box = boxes[id];
        box.SetLabel(b.label);
        box.SetCenter({b.minX + boxWidth * 0.5, b.minY + boxHeight * 0.5, 0});
        box.SetSize({static_cast<double>(boxWidth),
            static_cast<double>(boxHeight), 0.0});
      }
      return boxes;
    }

    //////////////////////////////////////////////////
    template <class T>
    template <class ParentNameFn>
    void BaseBoundingBoxCamera<T>::MergeMultiLinksModels2D(
        const std::map<uint32_t, BoundingBox> &_boxes,
        ParentNameFn _parentName, std::vector<BoundingBox> &_output) const
    {
      // Combine the boxes with the same parent name together to merge them
      std::map<std::string, std::vector<BoundingBox>> parentNameToBoxes;
      for (const auto &[id, box] : _boxes)
        parentNameToBoxes[_parentName(id)].push_back(box);

      for (const auto &nameToBoxes : parentNameToBoxes)
        _output.push_back(this->MergeBoxes2D(nameToBoxes.second));

      // reverse the order of the boxes (useful in testing)
      std::reverse(_output.begin(), _output.end());
    }

    //////////////////////////////////////////////////
    template <class T>
    BoundingBox BaseBoundingBoxCamera<T>::MergeBoxes2D(
        const std::vector<BoundingBox> &_boxes) const
    {
      if (_boxes.size() == 1)
        return _boxes[0];

      BoundingBox mergedBox;
      double minX = std::numeric_limits<double>::max();
      double maxX = 0.0;
      double minY = std::numeric_limits<double>::max();
      double maxY = 0.0;

      for (const auto &box : _boxes)
      {
        double boxMinX = box.Center().X() - box.Size().X() * 0.5;
        double boxMaxX = box.Center().X() + box.Size().X() * 0.5;
        double boxMinY = box.Center().Y() - box.Size().Y() * 0.5;
        double boxMaxY = box.Center().Y() + box.Size().Y() * 0.5;

        minX = std::min(minX, boxMinX);
        maxX = std::max(maxX, boxMaxX);
        minY = std::min(minY, boxMinY);
        maxY = std::max(maxY, boxMaxY);
      }

      auto width = maxX - minX;
      auto height = maxY - minY;
      mergedBox.SetSize({width, height, 0});
      mergedBox.SetCenter({minX + width * 0.5, minY + height * 0.5, 0});
      mergedBox.SetLabel(_boxes[0].Label());

      return mergedBox;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseBoundingBoxCamera<T>::DrawBoundingBox2D(unsigned char *_data,
        const math::Color &_color, const BoundingBox &_box) const
    {
      math::Vector2i minVertex(
        static_cast<int>(_box.Center().X() - _box.Size().X() / 2),
        static_cast<int>(_box.Center().Y() - _box.Size().Y() / 2));

      math::Vector2i maxVertex(
        static_cast<int>(_box.Center().X() + _box.Size().X() / 2),
        static_cast<int>(_box.Center().Y() + _box.Size().Y() / 2));

      uint32_t width = this->ImageWidth();

      std::vector<uint32_t> xValues =
        {uint32_t(minVertex.X()),
            std::min(this->ImageWidth() - 1, uint32_t(maxVertex.X()))};
      std::vector<uint32_t> yValues =
        {uint32_t(minVertex.Y()),
            std::min(this->ImageHeight() - 1, uint32_t(maxVertex.Y()))};

      for (auto i = minVertex.Y(); i < maxVertex.Y(); i++)
      {
        for (auto j : xValues)
        {
          auto index = (i * width + j) * 3;
          _data[index] = static_cast<unsigned char>(255 * _color.R());
          _data[index + 1] = static_cast<unsigned char>(255 * _color.G());
          _data[index + 2] = static_cast<unsigned char>(255 * _color.B());
        }
      }
      for (auto i : yValues)
      {
        for (auto j = minVertex.X(); j < maxVertex.X(); j++)
        {
          auto index = (i * width + j) * 3;
          _data[index] = static_cast<unsigned char>(255 * _color.R());
          _data[index + 1] = static_cast<unsigned char>(255 * _color.G());
          _data[index + 2] = static_cast<unsigned char>(255 * _color.B());
        }
      }
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseBoundingBoxCamera<T>::DrawBoundingBox3D(unsigned char *_data,
        const math::Color &_color,
        const std::vector<math::Vector2d> &_vertices2d) const
    {
      // clip the values outside the frustum range [-1, 1]
      /*

          1 -------- 0
         /|         /|
        2 -------- 3 .
        | |        | |
        . 5 -------- 4
        |/         |/
        6 -------- 7

      */
      // edges: top, bottom and pillars
      static const int kEdges[12][2] = {{0, 1}, {1, 2}, {2, 3}, {3, 0},
          {4, 5}, {5, 6}, {6, 7}, {7, 4}, {0, 4}, {1, 5}, {2, 6}, {3, 7}};
      std::vector<math::Vector2d> clippedVertices;
      const math::Vector4d bounds(-1.0, -1.0, 1.0, 1.0);
      for (const auto &edge : kEdges)
      {
        this->AddToViewportLines(bounds, _vertices2d[edge[0]],
            _vertices2d[edge[1]], clippedVertices);
      }

      // Convert To screen coord.
      uint32_t width = this->ImageWidth();
      uint32_t height = this->ImageHeight();

      std::vector<math::Vector2i> projVertices;
      projVertices.reserve(clippedVertices.size());
      for (auto &vertex : clippedVertices)
      {
        // convert from [-1, 1] range to [0, 1] range & to the screen range
        vertex.X() = uint32_t((vertex.X() + 1.0) / 2 * width );
        vertex.Y() = uint32_t((1.0 - vertex.Y()) / 2 * height);

        vertex.X() = std::max<double>(0.0, vertex.X());
        vertex.Y() = std::max<double>(0.0, vertex.Y());
        vertex.X() = std::min<double>(vertex.X(), width - 1.0);
        vertex.Y() = std::min<double>(vertex.Y(), height - 1.0);

        projVertices.push_back(math::Vector2i(static_cast<int>(vertex.X()),
            static_cast<int>(vertex.Y())));
      }

      for (size_t endPt = 0; endPt < projVertices.size(); endPt += 2)
      {
        this->DrawLine(_data, projVertices[endPt], projVertices[endPt + 1],
            _color);
      }
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseBoundingBoxCamera<T>::DrawLine(unsigned char *_data,
        const math::Vector2i &_point1, const math::Vector2i &_point2,
        const math::Color &_color) const
    {
      // walk along the major axis, from low to high
      const bool shallow = std::abs(_point2.Y() - _point1.Y()) <
          std::abs(_point2.X() - _point1.X());
      const bool forward = shallow ? _point1.X() < _point2.X() :
          _point1.Y() < _point2.Y();
      const math::Vector2i &start = forward ? _point1 : _point2;
      const math::Vector2i &end = forward ? _point2 : _point1;

      int major = shallow ? start.X() : start.Y();
      int minor = shallow ? start.Y() : start.X();
      const int majorEnd = shallow ? end.X() : end.Y();
      const int dMajor = majorEnd - major;
      int dMinor = shallow ? end.Y() - start.Y() : end.X() - start.X();
      int minorStep = 1;
      if (dMinor < 0)
      {
        minorStep = -1;
        dMinor = -dMinor;
      }
      auto D = 2 * dMinor - dMajor;

      for (; major < majorEnd; major++)
      {
        // Plot the point
        const int x = shallow ? major : minor;
        const int y = shallow ? minor : major;
        auto index = (y * this->ImageWidth() + x) * 3;
        _data[index] = static_cast<unsigned char>(255 * _color.R());
        _data[index + 1] = static_cast<unsigned char>(255 * _color.G());
        _data[index + 2] = static_cast<unsigned char>(255 * _color.B());

        if (D > 0)
        {
          minor += minorStep;
          D += (2 * (dMinor - dMajor));
        }
        else
        {
          D += (2 * dMinor);
        }
      }
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseBoundingBoxCamera<T>::AddToViewportLines(
        const math::Vector4d &_bounds, const math::Vector2d &_p0,
        const math::Vector2d &_p1, std::vector<math::Vector2d> &_lines) const
    {
      const auto xmin = _bounds[0];
      const auto ymin = _bounds[1];
      const auto xmax = _bounds[2];
      const auto ymax = _bounds[3];

      // The location of a point relative to the viewport is a bitwise
      // combination of:
      const int kLeft = 1;
      const int kRight = 2;
      const int kBottom = 4;
      const int kTop = 8;
      auto location = [&](double _x, double _y)
      {
        int relativeLocation = 0;
        if (_x < xmin)
          relativeLocation |= kLeft;
        else if (_x > xmax)
          relativeLocation |= kRight;
        if (_y < ymin)
          relativeLocation |= kBottom;
        else if (_y > ymax)
          relativeLocation |= kTop;
        return relativeLocation;
      };

      auto x0 = _p0.X();
      auto y0 = _p0.Y();
      auto x1 = _p1.X();
      auto y1 = _p1.Y();
      auto location0 = location(x0, y0);
      auto location1 = location(x1, y1);

      // bitwise OR is 0 when both endpoints are in the bounds
      while (location0 | location1)
      {
        // bitwise AND is not 0 when both points share a zone outside (left,
        // right, top, bottom) of the viewport: the line does not appear in
        // the viewport. Reject this line
        if (location0 & location1)
          return;

        // calculate the line segment to clip from an outside point to an
        // intersection with clip edge
        double x = 0.0;
        double y = 0.0;

        // at least one endpoint is outside the clip rectangle; pick it
        int outerLocation = location1 > location0 ? location1 : location0;

        // Find the intersection point. Use formulas:
        //    slope = (y1 - y0) / (x1 - x0)
        //    x = x0 + (1 / slope) * (ym - y0), where ym is ymin or ymax
        //    y = y0 + slope * (xm - x0), where xm is xmin or xmax
        // Divide by zero wont happen because in each case, outerLocation bit
        // being tested guarantees the denominator is non-zero
        if (outerLocation & kTop)
        {
          x = x0 + (x1 - x0) * (ymax - y0) / (y1 - y0);
          y = ymax;
        }
        else if (outerLocation & kBottom)
        {
          x = x0 + (x1 - x0) * (ymin - y0) / (y1 - y0);
          y = ymin;
        }
        else if (outerLocation & kRight)
        {
          y = y0 + (y1 - y0) * (xmax - x0) / (x1 - x0);
          x = xmax;
        }
        else
        {
          y = y0 + (y1 - y0) * (xmin - x0) / (x1 - x0);
          x = xmin;
        }

        // update the outside point to the intersection point
        if (outerLocation == location0)
        {
          x0 = x;
          y0 = y;
          location0 = location(x0, y0);
        }
        else
        {
          x1 = x;
          y1 = y;
          location1 = location(x1, y1);
        }
      }

      const math::Vector2d p0(x0, y0);
      const math::Vector2d p1(x1, y1);
      if (p0.IsFinite() && p1.IsFinite())
      {
        _lines.push_back(p0);
        _lines.push_back(p1);
      }
    }
    }
  }
}
#endif
