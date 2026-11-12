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
    /// \brief A gz material is one instance each of the gz_lit, gz_unlit and
    /// gz_unlit_transparent filament materials; parameters are pushed in
    /// PreRender.
    class GZ_RENDERING_FILAMENT_VISIBLE FilamentMaterial :
      public BaseMaterial<FilamentObject>
    {
      protected: FilamentMaterial();

      public: virtual ~FilamentMaterial() = default;

      public: virtual void Destroy() override;

      public: virtual void PreRender() override;

      public: virtual bool HasTexture() const override;

      public: virtual std::string Texture() const override;

      public: virtual void SetTexture(const std::string &_texture) override;

      public: virtual void SetTexture(const std::string &_texture,
          const std::shared_ptr<const common::Image> &_img) override;

      public: virtual std::shared_ptr<const common::Image> TextureData()
          const override;

      public: virtual void ClearTexture() override;

      public: virtual void SetRoughness(const float _roughness) override;

      public: virtual float Roughness() const override;

      public: virtual void SetMetalness(const float _metalness) override;

      public: virtual float Metalness() const override;

      // documentation inherited
      public: virtual bool HasNormalMap() const override;

      // documentation inherited
      public: virtual std::string NormalMap() const override;

      // documentation inherited
      public: virtual std::shared_ptr<const common::Image> NormalMapData()
          const override;

      // documentation inherited
      public: virtual void SetNormalMap(const std::string &_name,
          const std::shared_ptr<const common::Image> &_img) override;

      // documentation inherited
      public: virtual void ClearNormalMap() override;

      // documentation inherited
      public: virtual bool HasRoughnessMap() const override;

      // documentation inherited
      public: virtual std::string RoughnessMap() const override;

      // documentation inherited
      public: virtual std::shared_ptr<const common::Image> RoughnessMapData()
          const override;

      // documentation inherited
      public: virtual void SetRoughnessMap(const std::string &_name,
          const std::shared_ptr<const common::Image> &_img) override;

      // documentation inherited
      public: virtual void ClearRoughnessMap() override;

      // documentation inherited
      public: virtual bool HasMetalnessMap() const override;

      // documentation inherited
      public: virtual std::string MetalnessMap() const override;

      // documentation inherited
      public: virtual std::shared_ptr<const common::Image> MetalnessMapData()
          const override;

      // documentation inherited
      public: virtual void SetMetalnessMap(const std::string &_name,
          const std::shared_ptr<const common::Image> &_img) override;

      // documentation inherited
      public: virtual void ClearMetalnessMap() override;

      // documentation inherited
      public: virtual bool HasEnvironmentMap() const override;

      // documentation inherited
      public: virtual std::string EnvironmentMap() const override;

      // documentation inherited
      public: virtual std::shared_ptr<const common::Image> EnvironmentMapData()
          const override;

      // documentation inherited
      public: virtual void SetEnvironmentMap(const std::string &_name,
          const std::shared_ptr<const common::Image> &_img) override;

      // documentation inherited
      public: virtual void ClearEnvironmentMap() override;

      // documentation inherited
      public: virtual bool HasEmissiveMap() const override;

      // documentation inherited
      public: virtual std::string EmissiveMap() const override;

      // documentation inherited
      public: virtual std::shared_ptr<const common::Image> EmissiveMapData()
          const override;

      // documentation inherited
      public: virtual void SetEmissiveMap(const std::string &_name,
          const std::shared_ptr<const common::Image> &_img) override;

      // documentation inherited
      public: virtual void ClearEmissiveMap() override;

      // documentation inherited
      public: virtual bool HasLightMap() const override;

      // documentation inherited
      public: virtual std::string LightMap() const override;

      // documentation inherited
      public: virtual unsigned int LightMapTexCoordSet() const override;

      // documentation inherited
      public: virtual std::shared_ptr<const common::Image> LightMapData()
          const override;

      // documentation inherited
      public: virtual void SetLightMap(const std::string &_name,
          const std::shared_ptr<const common::Image> &_img,
          unsigned int _uvSet = 0u) override;

      // documentation inherited
      public: virtual void ClearLightMap() override;

      // documentation inherited
      public: virtual enum MaterialType Type() const override;

      // documentation inherited
      public: virtual std::string VertexShader() const override;

      // documentation inherited
      public: virtual ShaderParamsPtr VertexShaderParams() override;

      // documentation inherited
      public: virtual void SetVertexShader(const std::string &_path) override;

      // documentation inherited
      public: virtual std::string FragmentShader() const override;

      // documentation inherited
      public: virtual ShaderParamsPtr FragmentShaderParams() override;

      // documentation inherited
      public: virtual void SetFragmentShader(const std::string &_path)
          override;

      /// \brief Alpha of the drawn color: (1 - transparency) * diffuse alpha
      public: double Opacity() const;

      /// \brief Active material instance (lit, unlit, their transparent
      /// variants, or alpha tested lit)
      public: filament::MaterialInstance *Instance() const;

      /// \brief Whether the alpha tested (alpha from texture) instance is in
      /// use
      public: bool AlphaTested() const;

      /// \brief Diffuse texture on the GPU, nullptr if none
      public: filament::Texture *FilamentTexture() const;

      protected: virtual void Init() override;

      protected: void Update(filament::MaterialInstance *_instance);

      /// \brief Rebind meshes using _instance to a default one, destroy it
      protected: void DestroyInstance(filament::MaterialInstance *_instance);

      /// \brief Create, switch or drop the custom shader instance, push the
      /// shader params to it
      protected: void UpdateCustomShader();

      /// \brief Instance of the filament material compiled from the .mat
      /// shader source, nullptr without one. Takes over from the others.
      protected: filament::MaterialInstance *custom{nullptr};

      /// \brief .mat source of the custom instance
      protected: std::string customSource;

      protected: std::string vertexShaderPath;

      protected: std::string fragmentShaderPath;

      protected: ShaderParamsPtr vertexShaderParams;

      protected: ShaderParamsPtr fragmentShaderParams;

      protected: filament::MaterialInstance *lit{nullptr};

      protected: filament::MaterialInstance *unlit{nullptr};

      /// \brief gz_unlit with alpha blending, used instead of unlit while
      /// Transparency() > 0
      protected: filament::MaterialInstance *unlitTransparent{nullptr};

      /// \brief Instance of gz_lit_transparent, used when lighting is
      /// enabled and the material is not opaque. Created on first use.
      protected: filament::MaterialInstance *litTransparent{nullptr};

      /// \brief Instance of gz_lit_masked, used instead of lit when alpha
      /// from texture is enabled and the texture has alpha. Created on
      /// first use.
      protected: filament::MaterialInstance *litMasked{nullptr};

      protected: filament::Texture *texture{nullptr};

      protected: std::string loadedTexture;

      protected: std::string textureName;

      /// \brief Last values pushed to the instances, to skip redundant
      /// uniform uploads
      protected: bool pushed{false};

      protected: std::array<float, 18> pushedKey{};

      protected: filament::Texture *pushedTexture{nullptr};

      /// \brief PBR texture maps, in MapSlot order
      protected: enum MapSlot { NORMAL, ROUGHNESS, METALNESS, EMISSIVE, LIGHT,
          ENVIRONMENT, MAP_COUNT };

      /// \brief A texture map: name and optional in-memory image as set, and
      /// the filament texture loaded from them in PreRender
      protected: struct Map
      {
        std::string name;
        std::shared_ptr<const common::Image> data;
        filament::Texture *texture{nullptr};
        bool loaded{false};
      };

      /// \brief Set a map slot (empty name clears it)
      protected: void SetMap(MapSlot _slot, const std::string &_name,
          const std::shared_ptr<const common::Image> &_img);

      /// \brief Texture maps
      protected: std::array<Map, MAP_COUNT> maps;

      /// \brief Light map texture coordinate set
      protected: unsigned int lightMapUvSet{0u};

      /// \brief Map textures last pushed to the instances
      protected: std::array<filament::Texture *, MAP_COUNT> pushedMaps{};

      protected: std::shared_ptr<const common::Image> textureData;

      /// \brief gz::common::Pbr defaults, like ogre2 after Reset()
      protected: float roughness{0.5f};

      protected: float metalness{0.0f};

      private: friend class FilamentScene;

      /// \brief Sets its gz_terrain instance as the custom instance
      private: friend class FilamentHeightmap;
    };
    }
  }
}
#endif
