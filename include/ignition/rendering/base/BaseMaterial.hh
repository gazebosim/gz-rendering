/*
 * Copyright (C) 2015 Open Source Robotics Foundation
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
#ifndef IGNITION_RENDERING_BASE_BASEMATERIAL_HH_
#define IGNITION_RENDERING_BASE_BASEMATERIAL_HH_

#include <string>

#include "ignition/common/Console.hh"

#include "ignition/rendering/Material.hh"
#include "ignition/rendering/Scene.hh"
#include "ignition/rendering/ShaderType.hh"

namespace ignition
{
  namespace rendering
  {
    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
    //
    template <class T>
    class BaseMaterial :
      public virtual Material,
      public virtual T
    {
      protected: BaseMaterial();

      public: virtual ~BaseMaterial();

      public: virtual MaterialPtr Clone(const std::string &_name = "") const
                  override;

      public: virtual void SetAmbient(const double _r, const double _g,
                  const double _b, const double _a = 1.0) override;

      public: virtual void SetAmbient(const math::Color &_color) override = 0;

      public: virtual void SetDiffuse(const double _r, const double _g,
                  const double _b, const double _a = 1.0) override;

      public: virtual void SetDiffuse(const math::Color &_color) override = 0;

      public: virtual void SetSpecular(const double _r, const double _g,
                  const double _b, const double _a = 1.0) override;

      public: virtual void SetSpecular(const math::Color &_color) override = 0;

      public: virtual void SetEmissive(const double _r, const double _g,
                  const double _b, const double _a = 1.0) override;

      public: virtual void SetEmissive(const math::Color &_color) override = 0;

      public: virtual void SetReceiveShadows(const bool _receiveShadows)
                  override = 0;

      // Documentation inherited
      public: virtual bool HasTexture() const override;

      // Documentation inherited
      public: virtual std::string Texture() const override;

      // Documentation inherited
      public: virtual void SetTexture(const std::string &_texture) override;

      // Documentation inherited
      public: virtual void ClearTexture() override;

      // Documentation inherited
      public: virtual bool HasNormalMap() const override;

      // Documentation inherited
      public: virtual std::string NormalMap() const override;

      // Documentation inherited
      public: virtual void SetNormalMap(const std::string &_normalMap)
          override;

      // Documentation inherited
      public: virtual void ClearNormalMap() override;

      // Documentation inherited
      public: virtual bool HasRoughnessMap() const override;

      // Documentation inherited
      public: virtual std::string RoughnessMap() const override;

      // Documentation inherited
      public: virtual void SetRoughnessMap(const std::string &_roughnessMap)
          override;

      // Documentation inherited
      public: virtual void ClearRoughnessMap() override;

      // Documentation inherited
      public: virtual bool HasMetalnessMap() const override;

      // Documentation inherited
      public: virtual std::string MetalnessMap() const override;

      // Documentation inherited
      public: virtual void SetMetalnessMap(const std::string &_metalnessMap)
          override;

      // Documentation inherited
      public: virtual void ClearMetalnessMap() override;

      // Documentation inherited
      public: virtual bool HasEnvironmentMap() const override;

      // Documentation inherited
      public: virtual std::string EnvironmentMap() const override;

      // Documentation inherited
      public: virtual void SetEnvironmentMap(const std::string &_metalnessMap)
          override;

      // Documentation inherited
      public: virtual void ClearEnvironmentMap() override;

      // Documentation inherited
      public: virtual void SetRoughness(const float _roughness) override;

      // Documentation inherited
      public: virtual float Roughness() const override;

      // Documentation inherited
      public: virtual void SetMetalness(const float _metalness) override;

      // Documentation inherited
      public: virtual float Metalness() const override;

      // Documentation inherited
      public: virtual enum MaterialType Type() const override;

      public: virtual void SetShaderType(enum ShaderType /*_type*/) override
             {
               // no op
             }

      // Documentation inherited
      public: virtual enum ShaderType ShaderType() const override
             {
               return ST_PIXEL;
             }

      // Documentation inherited.
      // \sa Material::SetDepthMaterial()
      public: virtual void SetDepthMaterial(const double far,
                  const double near) override;

      // Documentation inherited.
      // \sa Material::VertexShader() const
      public: virtual std::string VertexShader() const override;

      // Documentation inherited.
      // \sa Material::VertexShaderParams()
      public: virtual ShaderParamsPtr VertexShaderParams() override;

      // Documentation inherited.
      // \sa Material::SetVertexShader(const std::string &)
      public: virtual void SetVertexShader(const std::string &_path) override;

      // Documentation inherited.
      // \sa Material::FragmentShader() const
      public: virtual std::string FragmentShader() const override;

      // Documentation inherited.
      // \sa Material::FragmentShaderParams()
      public: virtual ShaderParamsPtr FragmentShaderParams() override;

      // Documentation inherited.
      // \sa Material::SetFragmentShader(const std::string &)
      public: virtual void SetFragmentShader(const std::string &_path) override;

      public: virtual void CopyFrom(ConstMaterialPtr _material) override;

      public: virtual void CopyFrom(const common::Material &_material) override;

      public: virtual void PreRender() override;

      protected: virtual void Reset();
    };

    //////////////////////////////////////////////////
    template <class T>
    BaseMaterial<T>::BaseMaterial()
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    BaseMaterial<T>::~BaseMaterial()
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseMaterial<T>::SetAmbient(const double _r, const double _g,
        const double _b, const double _a)
    {
      this->SetAmbient(math::Color(_r, _g, _b, _a));
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseMaterial<T>::SetDiffuse(const double _r, const double _g,
        const double _b, const double _a)
    {
      this->SetDiffuse(math::Color(_r, _g, _b, _a));
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseMaterial<T>::SetSpecular(const double _r, const double _g,
        const double _b, const double _a)
    {
      this->SetSpecular(math::Color(_r, _g, _b, _a));
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseMaterial<T>::SetEmissive(const double _r, const double _g,
        const double _b, const double _a)
    {
      this->SetEmissive(math::Color(_r, _g, _b, _a));
    }

    //////////////////////////////////////////////////
    template <class T>
    enum MaterialType BaseMaterial<T>::Type() const
    {
      return MT_CLASSIC;
    }

    //////////////////////////////////////////////////
    template <class T>
    std::string BaseMaterial<T>::VertexShader() const
    {
      return std::string();
    }

    //////////////////////////////////////////////////
    template <class T>
    ShaderParamsPtr BaseMaterial<T>::VertexShaderParams()
    {
      return nullptr;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseMaterial<T>::SetVertexShader(const std::string &/*_path*/)
    {
      // no op
    }

    //////////////////////////////////////////////////
    template <class T>
    std::string BaseMaterial<T>::FragmentShader() const
    {
      return std::string();
    }

    //////////////////////////////////////////////////
    template <class T>
    ShaderParamsPtr BaseMaterial<T>::FragmentShaderParams()
    {
      return nullptr;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseMaterial<T>::SetFragmentShader(const std::string &/*_path*/)
    {
      // no op
    }

    //////////////////////////////////////////////////
    template <class T>
    bool BaseMaterial<T>::HasTexture() const
    {
      return false;
    }

    //////////////////////////////////////////////////
    template <class T>
    std::string BaseMaterial<T>::Texture() const
    {
      return std::string();
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseMaterial<T>::SetTexture(const std::string &)
    {
      // no op
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseMaterial<T>::ClearTexture()
    {
      // no op
    }

    //////////////////////////////////////////////////
    template <class T>
    bool BaseMaterial<T>::HasNormalMap() const
    {
      return false;
    }

    //////////////////////////////////////////////////
    template <class T>
    std::string BaseMaterial<T>::NormalMap() const
    {
      return std::string();
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseMaterial<T>::SetNormalMap(const std::string &)
    {
      // no op
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseMaterial<T>::ClearNormalMap()
    {
      // no op
    }

    //////////////////////////////////////////////////
    template <class T>
    bool BaseMaterial<T>::HasRoughnessMap() const
    {
      return false;
    }

    //////////////////////////////////////////////////
    template <class T>
    std::string BaseMaterial<T>::RoughnessMap() const
    {
      return std::string();
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseMaterial<T>::SetRoughnessMap(const std::string &)
    {
      // no op
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseMaterial<T>::ClearRoughnessMap()
    {
      // no op
    }

    //////////////////////////////////////////////////
    template <class T>
    bool BaseMaterial<T>::HasMetalnessMap() const
    {
      return false;
    }

    //////////////////////////////////////////////////
    template <class T>
    std::string BaseMaterial<T>::MetalnessMap() const
    {
      return std::string();
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseMaterial<T>::SetMetalnessMap(const std::string &)
    {
      // no op
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseMaterial<T>::ClearMetalnessMap()
    {
      // no op
    }

    //////////////////////////////////////////////////
    template <class T>
    bool BaseMaterial<T>::HasEnvironmentMap() const
    {
      return false;
    }

    //////////////////////////////////////////////////
    template <class T>
    std::string BaseMaterial<T>::EnvironmentMap() const
    {
      return std::string();
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseMaterial<T>::SetEnvironmentMap(const std::string &)
    {
      // no op
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseMaterial<T>::ClearEnvironmentMap()
    {
      // no op
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseMaterial<T>::SetRoughness(const float)
    {
      // no op
    }

    //////////////////////////////////////////////////
    template <class T>
    float BaseMaterial<T>::Roughness() const
    {
      return 0.0f;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseMaterial<T>::SetMetalness(const float)
    {
      // no op
    }

    //////////////////////////////////////////////////
    template <class T>
    float BaseMaterial<T>::Metalness() const
    {
      return 0.0f;
    }

    //////////////////////////////////////////////////
    template <class T>
    MaterialPtr BaseMaterial<T>::Clone(const std::string &_name) const
    {
      auto baseShared = this->shared_from_this();

      auto thisShared =
          std::dynamic_pointer_cast<const BaseMaterial<T>>(baseShared);

      MaterialPtr material = T::Scene()->CreateMaterial(_name);
      material->CopyFrom(thisShared);
      return material;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseMaterial<T>::CopyFrom(ConstMaterialPtr _material)
    {
      this->SetLightingEnabled(_material->LightingEnabled());
      this->SetAmbient(_material->Ambient());
      this->SetDiffuse(_material->Diffuse());
      this->SetSpecular(_material->Specular());
      this->SetEmissive(_material->Emissive());
      this->SetShininess(_material->Shininess());
      this->SetTransparency(_material->Transparency());
      this->SetReflectivity(_material->Reflectivity());
      this->SetCastShadows(_material->CastShadows());
      this->SetReceiveShadows(_material->ReceiveShadows());
      this->SetReflectionEnabled(_material->ReflectionEnabled());
      this->SetTexture(_material->Texture());
      this->SetNormalMap(_material->NormalMap());
      this->SetRoughnessMap(_material->RoughnessMap());
      this->SetMetalnessMap(_material->MetalnessMap());
      this->SetRoughness(_material->Roughness());
      this->SetMetalness(_material->Metalness());
      this->SetEnvironmentMap(_material->EnvironmentMap());
      this->SetShaderType(_material->ShaderType());
      this->SetVertexShader(_material->VertexShader());
      this->SetFragmentShader(_material->FragmentShader());
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseMaterial<T>::CopyFrom(const common::Material &_material)
    {
      this->SetLightingEnabled(_material.Lighting());
      this->SetAmbient(_material.Ambient());
      this->SetDiffuse(_material.Diffuse());
      this->SetSpecular(_material.Specular());
      this->SetEmissive(_material.Emissive());
      this->SetShininess(_material.Shininess());
      this->SetTransparency(_material.Transparency());
      // TODO(anyone): update common::Material
      this->SetReflectivity(0);
      this->SetTexture(_material.TextureImage());
      // TODO(anyone): update common::Material
      this->SetCastShadows(true);
      // TODO(anyone): update common::Material
      this->SetReceiveShadows(true);
      // TODO(anyone): update common::Material
      this->SetReflectionEnabled(true);
      // TODO(anyone): update common::Material
      this->ClearNormalMap();
      // TODO(anyone): update common::Material
      this->SetShaderType(ST_PIXEL);
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseMaterial<T>::PreRender()
    {
      // do nothing
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseMaterial<T>::SetDepthMaterial(const double /*far*/,
          const double /*near*/)
    {
      // do nothing
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseMaterial<T>::Reset()
    {
      this->SetLightingEnabled(true);
      this->SetAmbient(0.3, 0.3, 0.3);
      this->SetDiffuse(1.0, 1.0, 1.0);
      this->SetSpecular(0.2, 0.2, 0.2);
      this->SetEmissive(0, 0, 0);
      this->SetShininess(1.5);
      this->SetTransparency(0);
      this->SetReflectivity(0);
      this->SetCastShadows(true);
      this->SetReceiveShadows(true);
      this->SetReflectionEnabled(true);
      this->ClearTexture();
      this->ClearNormalMap();
      this->ClearRoughnessMap();
      this->ClearMetalnessMap();
      this->SetRoughness(1.0);
      this->SetMetalness(0.8);
      this->SetShaderType(ST_PIXEL);
    }
    }
  }
}
#endif
