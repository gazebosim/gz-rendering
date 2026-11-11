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
#ifndef GZ_RENDERING_FILAMENT_FILAMENTRENDERENGINE_HH_
#define GZ_RENDERING_FILAMENT_FILAMENTRENDERENGINE_HH_

#include <map>
#include <memory>

#include <gz/common/Image.hh>
#include <string>
#include <vector>

#include <gz/common/SingletonT.hh>

#include "gz/rendering/RenderEnginePlugin.hh"
#include "gz/rendering/base/BaseRenderEngine.hh"
#include "gz/rendering/filament/FilamentRenderTypes.hh"
#include "gz/rendering/filament/Export.hh"

namespace filament
{
  class Engine;
  class Material;
  class Renderer;
  class Texture;
}

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    //
    class FilamentRenderEnginePrivate;

    /// \brief Plugin for loading the filament render engine
    class GZ_RENDERING_FILAMENT_VISIBLE FilamentRenderEnginePlugin :
      public RenderEnginePlugin
    {
      public: FilamentRenderEnginePlugin();

      public: ~FilamentRenderEnginePlugin() = default;

      public: std::string Name() const override;

      public: RenderEngine *Engine() const override;
    };

    /// \brief Render engine backed by Google Filament.
    /// Experimental: supports visuals, meshes, lights, cameras and depth
    /// cameras rendered offscreen (OpenGL/EGL by default, Vulkan optional).
    class GZ_RENDERING_FILAMENT_VISIBLE FilamentRenderEngine :
      public virtual BaseRenderEngine,
      public common::SingletonT<FilamentRenderEngine>
    {
      private: FilamentRenderEngine();

      public: virtual ~FilamentRenderEngine();

      public: virtual void Destroy() override;

      public: virtual bool IsEnabled() const override;

      public: virtual std::string Name() const override;

      public: virtual rendering::GraphicsAPI GraphicsAPI() const override;

      /// \brief Whether float sensor targets (depth, lidar ranges) hold
      /// their float bits as RGBA8 instead of R32F: on Vulkan, whose
      /// readPixels converts R32F texel by texel but copies RGBA8 rows as
      /// they are. OpenGL reads both directly and keeps R32F.
      public: bool PackedFloatTargets() const;

      /// \brief Native filament engine
      public: filament::Engine *FilamentEngine() const;

      /// \brief Shared filament renderer
      public: filament::Renderer *Renderer() const;

      /// \brief Let the calling thread use filament's job system (render):
      /// the first thread that calls this keeps it. gz-gui creates the engine
      /// on one thread and renders from another.
      public: void AdoptThread();

      /// \brief True when filament's GL context shares objects with the
      /// caller's (load params "egl_display" and "egl_context"): camera
      /// colour targets are then GL textures the caller can sample.
      public: bool SharesCallerGLContext() const;

      /// \brief Open a frame on a headless swap chain: until EndFrame,
      /// FilamentRenderTarget::Render records into this one frame instead
      /// of submitting a standalone frame per call. Several passes then
      /// share one engine prepare, one command buffer and one submit.
      public: void BeginFrame();

      /// \brief Close (and submit) the frame opened by BeginFrame
      public: void EndFrame();

      /// \brief True between BeginFrame and EndFrame
      public: bool InFrame() const;

      /// \brief Get (and lazily load) a compiled material package by name,
      /// e.g. "gz_lit", looked up under the engine resource path. A path
      /// ending in ".mat" is a material source, compiled at runtime.
      public: filament::Material *Material(const std::string &_name);

      /// \brief Compile a filament material source (.mat) with filamat
      private: filament::Material *CompileMaterial(const std::string &_path);

      /// \brief Cached texture by name. Loaded from _data when given, else
      /// from the file _path; an empty name gives a 1x1 white texture.
      /// _srgb: color data (sRGB), else linear data (normal, roughness and
      /// metalness maps).
      public: filament::Texture *Texture(const std::string &_path,
          std::shared_ptr<const common::Image> _data = nullptr,
          bool _srgb = true);

      /// \brief Whether a texture returned by Texture() has a texel with
      /// alpha below 1. Opaque textures (no alpha channel, like ogre2's
      /// check of the pixel format, or a fully opaque one) give false.
      public: bool TextureHasAlpha(const filament::Texture *_texture) const;

      /// \brief Cached cube map from a DDS file (DXT1/DXT5, six faces, like
      /// ogre2's skybox.dds), looked up as given, then under the resource
      /// path's textures. Faces are converted from Ogre's left-handed cube
      /// map convention. An empty name gives a 1x1 black cube map; nullptr
      /// if the file can not be loaded. _srgb: decode the texels as sRGB
      /// (material environment maps, like ogre2), else read them as linear
      /// values (ogre2's skybox samples without gamma).
      public: filament::Texture *CubeTexture(const std::string &_path,
          bool _srgb = false);

      /// \brief Decoded RGBA8 faces of a cube map file, as uploaded by
      /// CubeTexture (filament face order and orientation).
      /// \return false if the file can not be loaded
      public: bool CubeFaces(const std::string &_path, unsigned int &_size,
          std::vector<uint8_t> &_faces);

      protected: virtual bool LoadImpl(
          const std::map<std::string, std::string> &_params) override;

      protected: virtual bool InitImpl() override;

      protected: virtual SceneStorePtr Scenes() const override;

      protected: virtual ScenePtr CreateSceneImpl(unsigned int _id,
                     const std::string &_name) override;

      /// \brief Singleton setup
      private: friend class common::SingletonT<FilamentRenderEngine>;

      private: std::unique_ptr<FilamentRenderEnginePrivate> dataPtr;
    };
    }
  }
}
#endif
