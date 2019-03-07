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

      public: virtual void ClearTexture() override = 0;

      public: virtual void ClearNormalMap() override = 0;

      public: virtual void SetShaderType(enum ShaderType _type) override = 0;

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

      public: virtual void SetCulling(CullMode _mode) override;

      public: virtual CullMode Culling() const override;

      protected: virtual void Reset();

      /// \brief Culling mode used by this material
      protected: CullMode culling = CM_BACK;
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
    std::string BaseMaterial<T>::VertexShader() const
    {
      return std::string();
    }

    //////////////////////////////////////////////////
    template <class T>
    ShaderParamsPtr BaseMaterial<T>::VertexShaderParams()
    {
      ignerr << "Vertex shader is not supported by the current render engine"
          << std::endl;
      return nullptr;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseMaterial<T>::SetVertexShader(const std::string &/*_path*/)
    {
      ignerr << "Vertex shader is not supported by the current render engine"
          << std::endl;
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
      ignerr << "Fragment shader is not supported by the current render engine"
          << std::endl;
      return nullptr;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseMaterial<T>::SetFragmentShader(const std::string &/*_path*/)
    {
      ignerr << "Fragment shader is not supported by the current render engine"
          << std::endl;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseMaterial<T>::SetCulling(CullMode _mode)
    {
      this->culling = _mode;
    }

    //////////////////////////////////////////////////
    template <class T>
    CullMode BaseMaterial<T>::Culling() const
    {
      return this->culling;
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
      this->SetShaderType(_material->ShaderType());
      this->SetVertexShader(_material->VertexShader());
      this->SetFragmentShader(_material->FragmentShader());
      this->SetCulling(_material->Culling());
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
      this->SetCulling(CM_BACK);
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseMaterial<T>::PreRender()
    {
      // do nothing
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseMaterial<T>::Reset()
    {
      this->SetLightingEnabled(true);
      this->SetAmbient(0.3, 0.3, 0.3);
      this->SetDiffuse(0.7, 0.7, 0.7);
      this->SetSpecular(0.01, 0.01, 0.01);
      this->SetEmissive(0, 0, 0);
      this->SetShininess(1.5);
      this->SetTransparency(0);
      this->SetReflectivity(0);
      this->SetCastShadows(true);
      this->SetReceiveShadows(true);
      this->SetReflectionEnabled(true);
      this->ClearTexture();
      this->ClearNormalMap();
      this->SetShaderType(ST_PIXEL);
      this->SetCulling(CM_BACK);
    }
    }
  }
}
#endif
