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

#ifndef IGNITION_RENDERING_OGRE2_OGRE2BOUNDINGBOXCAMERA_HH_
#define IGNITION_RENDERING_OGRE2_OGRE2BOUNDINGBOXCAMERA_HH_

#ifdef _WIN32
  // Ensure that Winsock2.h is included before Windows.h, which can get
  // pulled in by anybody (e.g., Boost).
  #include <Winsock2.h>
#endif

#include <memory>
#include <string>
#include <vector>

#include "ignition/rendering/base/BaseBoundingBoxCamera.hh"
#include "ignition/rendering/ogre2/Ogre2Includes.hh"
#include "ignition/rendering/ogre2/Ogre2Sensor.hh"

#include "ignition/common/Event.hh"
#include "ignition/common/Console.hh"

namespace Ogre
{
  class Material;
  class RenderTarget;
  class Texture;
  class Viewport;
}

namespace ignition
{
  namespace rendering
  {
    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
    //
    // Forward declaration
    class Ogre2BoundingBoxCameraPrivate;

    /// \brief BoundingBox camera used to detect 2d / 3d bounding boxes
    /// of labeled objects in the scene
    class IGNITION_RENDERING_OGRE2_VISIBLE Ogre2BoundingBoxCamera :
      public BaseBoundingBoxCamera<Ogre2Sensor>
    {
      /// \brief Constructor
      protected: Ogre2BoundingBoxCamera();

      /// \brief Destructor
      public: virtual ~Ogre2BoundingBoxCamera();

      /// \brief Initialize the camera
      public: virtual void Init() override;

      /// \brief Destroy the camera
      public: virtual void Destroy() override;

      /// \brief Create the camera.
      protected: void CreateCamera();

      // Documentation inherited
      public: virtual void PreRender() override;

      // Documentation inherited
      public: virtual void Render() override;

      // Documentation inherited
      public: virtual void PostRender() override;

      // Documentation inherited
      public: virtual std::vector<BoundingBox> BoundingBoxData() const override;

      // Documentation inherited
      public: virtual ignition::common::ConnectionPtr
        ConnectNewBoundingBoxes(
          std::function<void(const std::vector<BoundingBox> &)>) override;

      // Documentation inherited
      public: virtual void SetBoundingBoxType(BoundingBoxType _type) override;

      // Documentation inherited
      public: virtual BoundingBoxType Type() const override;

      /// \brief Create texture to hold ogre Ids to calculate the boundaries
      /// of each ogre id mask
      public: virtual void CreateBoundingBoxTexture();

      /// \brief Create dummy render texture. Needed to satisfy inheritance
      /// and to set image's dims
      public: virtual void CreateRenderTexture();

      /// \brief Get a pointer to the render target.
      /// \return Pointer to the render target
      protected: virtual RenderTargetPtr RenderTarget() const override;

      /// \brief Get the full bounding boxes by projecting all mesh vertices
      /// of each object then get the min & max of x & y to get the full bbox
      /// check the visibility by looping over pixels of the ogre Ids map
      public: virtual void FullBoundingBoxes();

      /// \brief Get the visible bounding boxes by looping over pixels of
      /// the ogre Ids map, to get the boundaries of each unique ogre Id mask
      public: virtual void VisibleBoundingBoxes();

      /// \brief Get the 3D bounding boxes after checking their visiblity
      public: virtual void BoundingBoxes3D();

      /// \brief Get minimal bounding box of the mesh by projecting the 3d
      /// vertices of the vertex buffer to 2d then get the min & max of x & y
      /// \param[in] mesh Mesh of the item to get its minimal bbox
      /// \param[in] viewMatrix Camera view matrix
      /// \param[in] projMatrix Camera projection matrix
      /// \param[out] minVertex Minimum of projected x & y & z of the vertices
      /// \param[out] maxVertex Maximum of projected x & y & z of the vertices
      /// \param[in] position position of the item in the world space
      /// \param[in] oreintation rotation of the item
      /// \param[in] scale scale of the item
      public: virtual void MeshMinimalBox(
          const Ogre::MeshPtr mesh,
          const Ogre::Matrix4 &viewMatrix,
          const Ogre::Matrix4 &projMatrix,
          Ogre::Vector3 &minVertex,
          Ogre::Vector3 &maxVertex,
          const Ogre::Vector3 &position,
          const Ogre::Quaternion &oreintation,
          const Ogre::Vector3 &scale
          );

      /// \brief Draw bounding box on the given image
      /// \param[in] data buffer contains the image data
      /// \param[in] box bounding box to draw
      public: virtual void DrawBoundingBox(unsigned char *_data,
        const BoundingBox &_box) override;

      /// \brief Draw line between any 2 points in the image data buffer
      /// Algorithm: https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
      /// \param[in] _data buffer contains the image data
      /// \param[in] _point1 The 1st 2D point in screen coordinates
      /// \param[in] _point1 The 2nd 2D point in screen coordinates
      public: virtual void DrawLine(unsigned char *_data,
        const math::Vector2i _point1, const math::Vector2i _point2);

      /// \brief Convert from clip coord (after projection) to screen coord.
      /// \param[inout] minVertex min vertex in clip coord to screen coord
      /// \param[inout] maxVertex max vertex in clip coord to screen coord
      public: virtual void ConvertToScreenCoord(Ogre::Vector3 &minVertex,
          Ogre::Vector3 &maxVertex);

      /// \brief Mark the visible boxes by checking the ogre ids map and mark
      /// the ogre id which appears in the map.
      public: virtual void MarkVisibleBoxes();

      /// \brief Merge a vector of 2D boxes, used in multi-links model.
      /// \param[in] _boxes Vector of 2D boxes
      /// \return Merged bounding box
      public: virtual BoundingBox MergeBoxes2D(
        const std::vector<BoundingBox *> &_boxes);

      /// \brief Get the 3d vertices(in world coord.) of the item's that belongs
      /// to the same parent (only used in multi-links models)
      /// \param[in] _ogreIds vector of ogre ids that belongs to the same model
      /// \param[out] _vertices vector of 3d vertices of the item
      public: virtual void MeshVertices(
        std::vector<uint32_t> ogreIds, std::vector<math::Vector3d> &_vertices);

      /// \brief Merge a links's 2d boxes of multi links models
      private: virtual void MergeMultiLinksModels2D();

      /// \brief Merge a links's 3d boxes of multi links models
      private: virtual void MergeMultiLinksModels3D();

      /// \brief Pointer to the ogre camera
      protected: Ogre::Camera *ogreCamera{nullptr};

      /// \internal
      /// \brief Pointer to private data.
      private: std::unique_ptr<Ogre2BoundingBoxCameraPrivate> dataPtr;

      private: friend class Ogre2Scene;
    };
    }
  }
}
#endif
