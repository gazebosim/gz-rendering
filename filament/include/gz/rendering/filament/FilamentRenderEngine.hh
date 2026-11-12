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
      /// \brief Constructor
      public: FilamentRenderEnginePlugin() = default;

      /// \brief Destructor
      public: ~FilamentRenderEnginePlugin() = default;

      // Documentation inherited
      public: std::string Name() const override;

      // Documentation inherited
      public: RenderEngine *Engine() const override;
    };

    /// \brief Render engine backed by Google Filament. A singleton that
    /// owns the filament engine, renderer and the materials and textures
    /// shared by all scenes. Experimental: renders offscreen (OpenGL/EGL by
    /// default, Vulkan optional).
    class GZ_RENDERING_FILAMENT_VISIBLE FilamentRenderEngine :
      public virtual BaseRenderEngine,
      public common::SingletonT<FilamentRenderEngine>
    {
      /// \brief Constructor
      private: FilamentRenderEngine();

      /// \brief Destructor
      public: virtual ~FilamentRenderEngine();

      // Documentation inherited
      public: virtual void Destroy() override;

      // Documentation inherited
      public: virtual bool IsEnabled() const override;

      // Documentation inherited
      public: virtual std::string Name() const override;

      // Documentation inherited
      public: virtual rendering::GraphicsAPI GraphicsAPI() const override;

      /// \brief Get the native filament engine.
      /// \return The filament engine, null before Init and after Destroy.
      public: filament::Engine *FilamentEngine() const;

      /// \brief Get the filament renderer, shared by all cameras.
      /// \return The filament renderer, null before Init and after Destroy.
      public: filament::Renderer *Renderer() const;

      /// \brief Let the calling thread use filament's job system (render):
      /// the first thread that calls this keeps it. gz-gui creates the engine
      /// on one thread and renders from another.
      public: void AdoptThread();

      /// \brief True when filament's GL context shares objects with the
      /// caller's (load params "egl_display" and "egl_context"): camera
      /// colour targets are then GL textures the caller can sample.
      /// \return True if the GL context is shared with the caller's.
      public: bool SharesCallerGLContext() const;

      /// \brief True when the caller asked for the GL context current on
      /// its thread (load param "useCurrentGLContext", e.g. gz-gui): camera
      /// colour targets then come as a texture in that context
      /// (FilamentRenderTarget::SharedGLId), shared if it is an EGL one,
      /// else copied every frame.
      /// \return True if the caller's current GL context is requested.
      public: bool UsesCallerGLContext() const;

      /// \brief Open a frame on a headless swap chain: until EndFrame,
      /// FilamentRenderTarget::Render records into this one frame instead
      /// of submitting a standalone frame per call. Several passes then
      /// share one engine prepare, one command buffer and one submit.
      public: void BeginFrame();

      /// \brief Close (and submit) the frame opened by BeginFrame
      public: void EndFrame();

      /// \brief Whether a frame is open.
      /// \return True between BeginFrame and EndFrame.
      public: bool InFrame() const;

      /// \brief Get (and lazily load) a compiled material package by name,
      /// e.g. "gz_lit", looked up under the engine resource path. A path
      /// ending in ".mat" is a material source, compiled at runtime.
      /// \param[in] _name Name of the material package, or path of a .mat
      /// source.
      /// \return The material, owned by the engine. Null (and an error,
      /// logged once) if it can not be loaded.
      public: filament::Material *Material(const std::string &_name);

      /// \brief Compile a filament material source (.mat) with filamat.
      private: filament::Material *CompileMaterial(const std::string &_path);

      /// \brief Get a cached texture. It is loaded from _data when given,
      /// else from the file _path; an empty path gives a 1x1 white texture.
      /// \param[in] _path Image file to load, which also names the texture in
      /// the cache.
      /// \param[in] _data Image to use instead of loading _path.
      /// \param[in] _srgb True for color data (sRGB), false for linear data
      /// (normal, roughness and metalness maps).
      /// \return The texture, owned by the engine, or null if it can not be
      /// loaded.
      public: filament::Texture *Texture(const std::string &_path,
          std::shared_ptr<const common::Image> _data = nullptr,
          bool _srgb = true);

      /// \brief Whether a texture returned by Texture() has a texel with
      /// alpha below 1. Opaque textures (no alpha channel, like ogre2's
      /// check of the pixel format, or a fully opaque one) give false.
      /// \param[in] _texture A texture returned by Texture().
      /// \return True if the texture has a texel with alpha below 1.
      public: bool TextureHasAlpha(const filament::Texture *_texture) const;

      /// \brief Get a cached cube map from a DDS file (DXT1/DXT5, six faces,
      /// like ogre2's skybox.dds), looked up as given, then under the
      /// resource path's textures. Faces are converted from Ogre's
      /// left-handed cube map convention. An empty path gives a 1x1 black
      /// cube map.
      /// \param[in] _path DDS file to load, which also names the cube map in
      /// the cache.
      /// \param[in] _srgb True to decode the texels as sRGB (material
      /// environment maps, like ogre2), false to read them as linear values
      /// (ogre2's skybox samples without gamma).
      /// \return The cube map, owned by the engine, or nullptr if the file
      /// can not be loaded.
      public: filament::Texture *CubeTexture(const std::string &_path,
          bool _srgb = false);

      /// \brief Decode the RGBA8 faces of a cube map file, as uploaded by
      /// CubeTexture (filament face order and orientation).
      /// \param[in] _path DDS file to load, as in CubeTexture.
      /// \param[out] _size Width and height of each face in pixels.
      /// \param[out] _faces The six faces one after the other, each
      /// _size x _size RGBA8 pixels.
      /// \return False if the file can not be loaded.
      public: bool CubeFaces(const std::string &_path, unsigned int &_size,
          std::vector<uint8_t> &_faces);

      // Documentation inherited
      protected: virtual bool LoadImpl(
          const std::map<std::string, std::string> &_params) override;

      /// \brief Create the filament engine and renderer. OpenGL is used
      /// unless Vulkan was requested, and Vulkan falls back to OpenGL when
      /// it is unavailable.
      protected: virtual bool InitImpl() override;

      /// \brief Get the store of scenes managed by the render engine, null
      /// before Init and after Destroy.
      protected: virtual SceneStorePtr Scenes() const override;

      /// \brief Create a scene and add it to the engine's scene store.
      protected: virtual ScenePtr CreateSceneImpl(unsigned int _id,
                     const std::string &_name) override;

      /// \brief Singleton setup
      private: friend class common::SingletonT<FilamentRenderEngine>;

      /// \brief Private data pointer.
      private: std::unique_ptr<FilamentRenderEnginePrivate> dataPtr;
    };
    }
  }
}
#endif
