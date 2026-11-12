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
#ifndef GZ_RENDERING_FILAMENT_FILAMENTMATERIAL_HH_
#define GZ_RENDERING_FILAMENT_FILAMENTMATERIAL_HH_

#include <array>
#include <memory>
#include <string>

#include "gz/rendering/config.hh"
#include "gz/rendering/base/BaseMaterial.hh"
#include "gz/rendering/filament/FilamentObject.hh"
#include "gz/rendering/filament/FilamentRenderTypes.hh"
#include "gz/rendering/filament/Export.hh"

namespace filament
{
  class MaterialInstance;
  class Texture;
}

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    //
    /// \brief Filament implementation of the material class. A gz material
    /// holds one instance each of the gz_lit, gz_unlit and
    /// gz_unlit_transparent filament materials, plus gz_lit_transparent and
    /// gz_lit_masked instances created once needed, and an instance compiled
    /// from a .mat shader source if one is set. Parameters are pushed to them
    /// in PreRender.
    class GZ_RENDERING_FILAMENT_VISIBLE FilamentMaterial :
      public BaseMaterial<FilamentObject>
    {
      /// \brief Constructor
      protected: FilamentMaterial();

      /// \brief Destructor
      public: virtual ~FilamentMaterial() = default;

      /// \brief Destroy the filament material instances. Meshes still
      /// drawing with them are first rebound to the default instance of
      /// gz_unlit.
      public: virtual void Destroy() override;

      /// \brief Load the textures, create the blended and alpha tested
      /// instances once they are needed and, if anything changed, push the
      /// parameters to the instances.
      public: virtual void PreRender() override;

      // Documentation inherited
      public: virtual bool HasTexture() const override;

      // Documentation inherited
      public: virtual std::string Texture() const override;

      // Documentation inherited
      public: virtual void SetTexture(const std::string &_texture) override;

      // Documentation inherited
      public: virtual void SetTexture(const std::string &_texture,
          const std::shared_ptr<const common::Image> &_img) override;

      // Documentation inherited
      public: virtual std::shared_ptr<const common::Image> TextureData()
          const override;

      // Documentation inherited
      public: virtual void ClearTexture() override;

      // Documentation inherited
      public: virtual void SetRoughness(const float _roughness) override;

      // Documentation inherited
      public: virtual float Roughness() const override;

      // Documentation inherited
      public: virtual void SetMetalness(const float _metalness) override;

      // Documentation inherited
      public: virtual float Metalness() const override;

      // Documentation inherited
      public: virtual bool HasNormalMap() const override;

      // Documentation inherited
      public: virtual std::string NormalMap() const override;

      // Documentation inherited
      public: virtual std::shared_ptr<const common::Image> NormalMapData()
          const override;

      // Documentation inherited
      public: virtual void SetNormalMap(const std::string &_name,
          const std::shared_ptr<const common::Image> &_img) override;

      // Documentation inherited
      public: virtual void ClearNormalMap() override;

      // Documentation inherited
      public: virtual bool HasRoughnessMap() const override;

      // Documentation inherited
      public: virtual std::string RoughnessMap() const override;

      // Documentation inherited
      public: virtual std::shared_ptr<const common::Image> RoughnessMapData()
          const override;

      // Documentation inherited
      public: virtual void SetRoughnessMap(const std::string &_name,
          const std::shared_ptr<const common::Image> &_img) override;

      // Documentation inherited
      public: virtual void ClearRoughnessMap() override;

      // Documentation inherited
      public: virtual bool HasMetalnessMap() const override;

      // Documentation inherited
      public: virtual std::string MetalnessMap() const override;

      // Documentation inherited
      public: virtual std::shared_ptr<const common::Image> MetalnessMapData()
          const override;

      // Documentation inherited
      public: virtual void SetMetalnessMap(const std::string &_name,
          const std::shared_ptr<const common::Image> &_img) override;

      // Documentation inherited
      public: virtual void ClearMetalnessMap() override;

      // Documentation inherited
      public: virtual bool HasEnvironmentMap() const override;

      // Documentation inherited
      public: virtual std::string EnvironmentMap() const override;

      // Documentation inherited
      public: virtual std::shared_ptr<const common::Image> EnvironmentMapData()
          const override;

      /// \brief Set the environment map. Unlike the other maps, the image
      /// is not used to build the texture, only stored for
      /// EnvironmentMapData(): the cube map is always loaded from _name.
      /// \param[in] _name Path of a DXT1 or DXT5 DDS cube map file
      /// \param[in] _img Stored, not rendered
      public: virtual void SetEnvironmentMap(const std::string &_name,
          const std::shared_ptr<const common::Image> &_img) override;

      // Documentation inherited
      public: virtual void ClearEnvironmentMap() override;

      // Documentation inherited
      public: virtual bool HasEmissiveMap() const override;

      // Documentation inherited
      public: virtual std::string EmissiveMap() const override;

      // Documentation inherited
      public: virtual std::shared_ptr<const common::Image> EmissiveMapData()
          const override;

      // Documentation inherited
      public: virtual void SetEmissiveMap(const std::string &_name,
          const std::shared_ptr<const common::Image> &_img) override;

      // Documentation inherited
      public: virtual void ClearEmissiveMap() override;

      // Documentation inherited
      public: virtual bool HasLightMap() const override;

      // Documentation inherited
      public: virtual std::string LightMap() const override;

      // Documentation inherited
      public: virtual unsigned int LightMapTexCoordSet() const override;

      // Documentation inherited
      public: virtual std::shared_ptr<const common::Image> LightMapData()
          const override;

      // Documentation inherited
      public: virtual void SetLightMap(const std::string &_name,
          const std::shared_ptr<const common::Image> &_img,
          unsigned int _uvSet = 0u) override;

      // Documentation inherited
      public: virtual void ClearLightMap() override;

      // Documentation inherited
      public: virtual enum MaterialType Type() const override;

      // Documentation inherited
      public: virtual std::string VertexShader() const override;

      // Documentation inherited
      public: virtual ShaderParamsPtr VertexShaderParams() override;

      /// \brief Set the vertex shader path. Handled like the fragment shader,
      /// see SetFragmentShader: it is only used when the fragment shader path
      /// is not a .mat source.
      /// \param[in] _path Path of the shader.
      public: virtual void SetVertexShader(const std::string &_path) override;

      // Documentation inherited
      public: virtual std::string FragmentShader() const override;

      // Documentation inherited
      public: virtual ShaderParamsPtr FragmentShaderParams() override;

      /// \brief Set the fragment shader path. Filament does not take GLSL: a
      /// path ending in .mat names a filament material source, which is
      /// compiled and drawn instead of the regular instances. Any other path
      /// is ignored with a warning and the regular material is drawn.
      /// \param[in] _path Path of the shader.
      public: virtual void SetFragmentShader(const std::string &_path)
          override;

      /// \brief Alpha of the drawn color: (1 - transparency) * diffuse alpha.
      /// \return Opacity, 1 for a fully opaque material.
      public: double Opacity() const;

      /// \brief Active material instance: the custom shader instance if there
      /// is one, else the lit or unlit instance, in its blended or alpha
      /// tested variant, that fits the current settings.
      /// \return The instance to draw with, nullptr if the base instances
      /// could not be created.
      public: filament::MaterialInstance *Instance() const;

      /// \brief Whether the alpha tested (alpha from texture) instance is in
      /// use.
      /// \return True if Instance() is the alpha tested one.
      public: bool AlphaTested() const;

      /// \brief Diffuse texture on the GPU, nullptr if none.
      /// \return Texture owned by the render engine.
      public: filament::Texture *FilamentTexture() const;

      /// \brief Create the lit, unlit and unlit transparent instances and
      /// push the initial parameters to them.
      protected: virtual void Init() override;

      /// \brief Push the parameters of the material to an instance: color,
      /// texture and depth flags, and for the lit instances also roughness,
      /// metalness, specular, emissive and the PBR maps.
      /// \param[in] _instance Instance to update.
      protected: void Update(filament::MaterialInstance *_instance);

      /// \brief Rebind meshes using _instance to the default instance of
      /// gz_unlit, then destroy it.
      /// \param[in] _instance Instance to destroy, may be nullptr.
      protected: void DestroyInstance(filament::MaterialInstance *_instance);

      /// \brief Create, switch or drop the custom shader instance, push the
      /// shader params to it.
      protected: void UpdateCustomShader();

      /// \brief Instance of the filament material compiled from the .mat
      /// shader source, nullptr without one. Takes over from the others.
      protected: filament::MaterialInstance *custom{nullptr};

      /// \brief .mat source of the custom instance
      protected: std::string customSource;

      /// \brief Vertex shader path set by SetVertexShader
      protected: std::string vertexShaderPath;

      /// \brief Fragment shader path set by SetFragmentShader
      protected: std::string fragmentShaderPath;

      /// \brief Parameters of the vertex shader, pushed to the custom
      /// instance when they change
      protected: ShaderParamsPtr vertexShaderParams;

      /// \brief Parameters of the fragment shader, pushed to the custom
      /// instance when they change
      protected: ShaderParamsPtr fragmentShaderParams;

      /// \brief Instance of gz_lit, used when lighting is enabled and no
      /// other lit instance applies
      protected: filament::MaterialInstance *lit{nullptr};

      /// \brief Instance of gz_unlit, used when lighting is disabled and the
      /// material is opaque
      protected: filament::MaterialInstance *unlit{nullptr};

      /// \brief gz_unlit with alpha blending, used instead of unlit while
      /// Opacity() < 1.
      protected: filament::MaterialInstance *unlitTransparent{nullptr};

      /// \brief Instance of gz_lit_transparent, used when lighting is
      /// enabled and the material is not opaque. Created on first use.
      protected: filament::MaterialInstance *litTransparent{nullptr};

      /// \brief Instance of gz_lit_masked, used instead of lit when alpha
      /// from texture is enabled and the texture has alpha. Created on
      /// first use.
      protected: filament::MaterialInstance *litMasked{nullptr};

      /// \brief Diffuse texture loaded for textureName, nullptr if none. It
      /// is owned by the render engine.
      protected: filament::Texture *texture{nullptr};

      /// \brief Name of the texture last loaded into texture, empty to load
      /// it again
      protected: std::string loadedTexture;

      /// \brief Name of the diffuse texture, as set by SetTexture
      protected: std::string textureName;

      /// \brief True once the parameters were pushed to the instances. With
      /// pushedKey, pushedTexture and pushedMaps it skips redundant uniform
      /// uploads.
      protected: bool pushed{false};

      /// \brief Values of the material parameters last pushed to the
      /// instances
      protected: std::array<float, 18> pushedKey{};

      /// \brief Diffuse texture last pushed to the instances
      protected: filament::Texture *pushedTexture{nullptr};

      /// \brief Slot of a PBR texture map: its index in maps and pushedMaps,
      /// and its bit in the mapFlags parameter of the lit instances.
      protected: enum MapSlot { NORMAL, ROUGHNESS, METALNESS, EMISSIVE, LIGHT,
          ENVIRONMENT, MAP_COUNT };

      /// \brief A texture map: name and optional in-memory image as set, and
      /// the filament texture loaded from them in PreRender
      protected: struct Map
      {
        /// \brief Name of the map, empty if the slot is not set
        std::string name;
        /// \brief In-memory image of the map, nullptr to load it by name
        std::shared_ptr<const common::Image> data;
        /// \brief Filament texture loaded in PreRender, owned by the render
        /// engine. nullptr until then.
        filament::Texture *texture{nullptr};
        /// \brief True once texture was loaded for the current name
        bool loaded{false};
      };

      /// \brief Set a map slot (empty name clears it)
      /// \param[in] _slot Slot to set.
      /// \param[in] _name Name of the map, empty to clear the slot.
      /// \param[in] _img Image of the map, nullptr to load it by name. The
      /// environment map is always loaded by name.
      protected: void SetMap(MapSlot _slot, const std::string &_name,
          const std::shared_ptr<const common::Image> &_img);

      /// \brief PBR texture maps, indexed by MapSlot
      protected: std::array<Map, MAP_COUNT> maps;

      /// \brief Light map texture coordinate set
      protected: unsigned int lightMapUvSet{0u};

      /// \brief Map textures last pushed to the instances
      protected: std::array<filament::Texture *, MAP_COUNT> pushedMaps{};

      /// \brief Image of the diffuse texture as set by SetTexture, nullptr to
      /// load it by textureName
      protected: std::shared_ptr<const common::Image> textureData;

      /// \brief Roughness, the gz::common::Pbr default until set, like ogre2
      /// after Reset()
      protected: float roughness{0.5f};

      /// \brief Metalness, the gz::common::Pbr default until set, like ogre2
      /// after Reset()
      protected: float metalness{0.0f};

      private: friend class FilamentScene;

      /// \brief Sets its gz_terrain instance as the custom instance
      private: friend class FilamentHeightmap;
    };
    }
  }
}
#endif
