/*
 * Copyright (C) 2018 Open Source Robotics Foundation
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

#ifndef IGNITION_RENDERING_OGRE_OGREGPURAYS_HH_
#define IGNITION_RENDERING_OGRE_OGREGPURAYS_HH_

#include <string>
#include <vector>
#include <memory>
#include <sstream>

#include "ignition/rendering/RenderTypes.hh"
#include "ignition/rendering/base/BaseGpuRays.hh"
#include "ignition/rendering/ogre/OgreConversions.hh"
#include "ignition/rendering/ogre/OgreIncludes.hh"
#include "ignition/rendering/ogre/OgreRenderTarget.hh"
#include "ignition/rendering/ogre/OgreRenderTypes.hh"
#include "ignition/rendering/ogre/OgreMaterial.hh"
#include "ignition/rendering/ogre/OgreScene.hh"
#include "ignition/rendering/ogre/OgreSensor.hh"
#include "ignition/rendering/ogre/OgreSelectionBuffer.hh"

#ifndef _WIN32
  #include <dirent.h>
#else
  // Ensure that Winsock2.h is included before Windows.h, which can get
  // pulled in by anybody (e.g., Boost).
  #include <Winsock2.h>
  #include "ignition/common/win_dirent.h"
#endif

namespace Ogre
{
  class Material;
  class Renderable;
  class Pass;
  class AutoParamDataSource;
  class Matrix4;
  class MovableObject;
}

namespace ignition
{
  namespace rendering
  {
    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE
    {
    /// \internal
    /// \brief Private data for the OgreGpuRays class
    class OgreGpuRaysPrivate
    {
      public: const std::string laser_scan1st_vertex_shader_file =
        "laser_scan1st_vertex_shader.glsl";
      public: const std::string laser_scan1st_fragment_shader_file =
        "laser_scan1st_fragment_shader.glsl";

      public: const std::string laser_scan2nd_vertex_shader_file =
        "laser_scan2nd_vertex_shader.glsl";
      public: const std::string laser_scan2nd_fragment_shader_file =
        "laser_scan2nd_fragment_shader.glsl";

      /// \brief Event triggered when new laser range data are available.
      /// \param[in] _frame New frame containing raw laser data.
      /// \param[in] _width Width of frame.
      /// \param[in] _height Height of frame.
      /// \param[in] _depth Max depth of frame.
      /// \param[in] _format Format of frame.
      public: ignition::common::EventT<void(const float *,
                   unsigned int, unsigned int, unsigned int,
                   const std::string &)> newLaserFrame;

      /// \brief Raw buffer of laser data.
      public: float *laserBuffer = nullptr;

      /// \brief Outgoing laser data, used by newLaserFrame event.
      public: float *laserScan = nullptr;

      /// \brief Pointer to Ogre material for the first rendering pass.
      public: OgreMaterialPtr matFirstPass;

      /// \brief Pointer to Ogre material for the sencod rendering pass.
      public: OgreMaterialPtr matSecondPass;

      /// \brief Temporary pointer to the current material.
      public: OgreMaterialPtr currentMat;

      /// \brief An array of first pass textures.
      public: OgreRenderTexturePtr firstPassTextures[3];

      /// \brief Second pass texture.
      public: OgreRenderTexturePtr secondPassTexture;

      /// \brief Temporary pointer to the current render target.
      public: OgreRenderTexturePtr currentTexture;

      /// \brief Ogre orthorgraphic camera used in the second pass for
      /// undistortion.
      public: Ogre::Camera *orthoCam = nullptr;

      /// \brief Pointer to the ogre camera
      public: Ogre::Camera *ogreCamera = nullptr;

      /// \brief First pass viewports.
      public: Ogre::Viewport *firstPassViewports[3];

      /// \brief Second pass viewport
      public: Ogre::Viewport *secondPassViewport;

      /// \brief Ogre scenenode where the orthorgraphic camera is attached to.
      public: Ogre::SceneNode *pitchNodeOrtho;

      /// \brief Ogre mesh used to create a canvas for undistorting range values
      /// in the second rendering pass.
      public: common::Mesh *undistMesh;

      /// \brief Pointer to visual that holds the canvas.
      public: VisualPtr visual;

      /// \brief Number of first pass textures.
      public: unsigned int textureCount = 0;

      /// \brief A list of camera angles for first pass rendering.
      public: double cameraYaws[4];

      /// \brief Image width of second pass.
      public: unsigned int w2nd = 0;

      /// \brief Image height of second pass.
      public: unsigned int h2nd = 0;

      /// \brief Time taken to complete the two rendering passes.
      public: double lastRenderDuration = 0;

      /// \brief List of texture unit indices used during the second
      /// rendering pass.
      public: std::vector<int> texIdx;

      /// Number of second pass texture units created.
      public: int texCount;
    };

    /** \class OgreGpuRays OgreGpuRays.hh\
     * rendering/ogre/OgreGpuRays.hh
    **/
    /// \brief Gpu Rays used to render depth data into an image buffer
    class IGNITION_RENDERING_OGRE_VISIBLE OgreGpuRays :
      public BaseGpuRays<OgreSensor>, public Ogre::RenderObjectListener
    {
      /// \brief Constructor
      protected: OgreGpuRays();

      /// \brief Destructor
      public: virtual ~OgreGpuRays();

      // Documentation inherited
      public: virtual void Init() override;

      /// \brief Create the texture which is used to render laser data.
      public: virtual void CreateLaserTexture() override;

      /// \brief Set the number of samples in the width and height for the
      /// first pass texture.
      /// \param[in] _w Number of samples in the horizontal sweep
      /// \param[in] _h Number of samples in the vertical sweep
      public: virtual void SetSecondPassTextureSize(const unsigned int _w,
          const unsigned int _h = 1);

      // Documentation inherited
      public: virtual void PostRender() override;

      /// \brief All things needed to get back z buffer for laser data.
      /// \return Array of laser data.
      /// \deprecated use LaserDataBegin() and LaserDataEnd() instead
      public: virtual const float *LaserData() const override;

      /// \brief Connect to a laser frame signal
      /// \param[in] _subscriber Callback that is called when a new image is
      /// generated
      /// \return A pointer to the connection. This must be kept in scope.
      public: virtual common::ConnectionPtr ConnectNewLaserFrame(
                  std::function<void(const float *_frame, unsigned int _width,
                  unsigned int _height, unsigned int _depth,
                  const std::string &_format)> _subscriber) override;

      /// \return Pointer to the render target
      public: virtual RenderTargetPtr RenderTarget() const override;

      /// \internal
      /// \brief Implementation of Ogre::RenderObjectListener
      public: virtual void notifyRenderSingleObject(Ogre::Renderable *_rend,
              const Ogre::Pass *_p, const Ogre::AutoParamDataSource *_s,
              const Ogre::LightList *_ll, bool _supp);

      // Documentation inherited.
      private: virtual void Render();

      /// \brief Create a mesh.
      private: void CreateMesh();

      /// \brief Create first and second pass materials.
      private: void CreateMaterials();

      /// \brief Create a canvas.
      private: void CreateCanvas();

      /// \brief Create an ortho camera.
      private: void CreateOrthoCam();

      /// \brief Create an ortho camera.
      private: void CreateCamera();

      /// \brief Builds scaled Orthogonal Matrix from parameters.
      /// \param[in] _left Left clip.
      /// \param[in] _right Right clip.
      /// \param[in] _bottom Bottom clip.
      /// \param[in] _top Top clip.
      /// \param[in] _near Near clip.
      /// \param[in] _far Far clip.
      /// \return The Scaled orthogonal Ogre::Matrix4
      private: Ogre::Matrix4 BuildScaledOrthoMatrix(const float _left,
          const float _right, const float _bottom, const float _top,
          const float _near, const float _far);

      /// \brief Configure render target.
      /// \param[in] _target texture target.
      /// \param[in] _material material used for the texture.
      /// \param[in] _camera ogre camera.
      /// \param[in] _color background color.
      /// \param[in] _format pixel format.
      /// \param[in] _width width of the texture.
      /// \param[in] _height height of the texture.
      private: void SetRenderTexture(OgreRenderTexturePtr _target,
                                     OgreMaterialPtr _material,
                                     Ogre::Camera *_camera,
                                     Ogre::ColourValue _color,
                                     PixelFormat _format,
                                     const unsigned int _width,
                                     const unsigned int _height);

      private: void UpdateRenderTarget(Ogre::RenderTarget *_target,
                                        Ogre::MaterialPtr _material,
                                        Ogre::Camera *_cam,
                                        bool _updateTex);

      private: void SetFragmentShadersParams();

      /// \internal
      /// \brief Pointer to private data.
      private: std::unique_ptr<OgreGpuRaysPrivate> dataPtr = nullptr;

      private: friend class OgreScene;
      private: friend class OgreRayQuery;
    };
    }
  }
}
#endif
