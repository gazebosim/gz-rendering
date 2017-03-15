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
#ifndef IGNITION_RENDERING_BASEMATERIAL_HH_
#define IGNITION_RENDERING_BASEMATERIAL_HH_

#include "ignition/rendering/Material.hh"
#include "ignition/rendering/Scene.hh"

namespace ignition
{
  namespace rendering
  {
    template <class T>
    class IGNITION_VISIBLE BaseMaterial :
      public virtual Material,
      public virtual T
    {
      protected: BaseMaterial();

      public: virtual ~BaseMaterial();

      public: virtual MaterialPtr Clone() const;

      public: virtual void SetAmbient(double _r, double _g, double _b,
                  double _a = 1.0);

      public: virtual void SetAmbient(const math::Color &_color) = 0;
      public: virtual void SetAmbient(const common::Color &_color)
      {
        this->SetAmbient(_color.R(), _color.G(), _color.B(), _color.A());
      };

      public: virtual void SetDiffuse(double _r, double _g, double _b,
                  double _a = 1.0);

      public: virtual void SetDiffuse(const math::Color &_color) = 0;
      public: virtual void SetDiffuse(const common::Color &_color)
      {
        this->SetDiffuse(_color.R(), _color.G(), _color.B(), _color.A());
      };

      public: virtual void SetSpecular(double _r, double _g, double _b,
                  double _a = 1.0);

      public: virtual void SetSpecular(const math::Color &_color) = 0;
      public: virtual void SetSpecular(const common::Color &_color)
      {
        this->SetSpecular(_color.R(), _color.G(), _color.B(), _color.A());
      };

      public: virtual void SetEmissive(double _r, double _g, double _b,
                  double _a = 1.0);

      public: virtual void SetEmissive(const math::Color &_color) = 0;
      public: virtual void SetEmissive(const common::Color &_color)
      {
        this->SetEmissive(_color.R(), _color.G(), _color.B(), _color.A());
      };

      public: virtual void SetReceiveShadows(bool _receiveShadows) = 0;

      public: virtual void ClearTexture() = 0;

      public: virtual void ClearNormalMap() = 0;

      public: virtual void SetShaderType(enum ShaderType _type) = 0;

      public: virtual void CopyFrom(ConstMaterialPtr _material);

      public: virtual void CopyFrom(const common::Material &_material);

      public: virtual void PreRender();

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
    void BaseMaterial<T>::SetAmbient(double _r, double _g, double _b, double _a)
    {
      this->SetAmbient(math::Color(_r, _g, _b, _a));
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseMaterial<T>::SetDiffuse(double _r, double _g, double _b, double _a)
    {
      this->SetDiffuse(math::Color(_r, _g, _b, _a));
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseMaterial<T>::SetSpecular(double _r, double _g, double _b,
        double _a)
    {
      this->SetSpecular(math::Color(_r, _g, _b, _a));
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseMaterial<T>::SetEmissive(double _r, double _g, double _b,
        double _a)
    {
      this->SetEmissive(math::Color(_r, _g, _b, _a));
    }

    //////////////////////////////////////////////////
    template <class T>
    MaterialPtr BaseMaterial<T>::Clone() const
    {
      auto baseShared = this->shared_from_this();

      auto thisShared =
          std::dynamic_pointer_cast<const BaseMaterial<T>>(baseShared);

      MaterialPtr material = T::Scene()->CreateMaterial();
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
      this->SetReflectivity(0); // TODO: update common::Material
      this->SetTexture(_material.TextureImage());
      this->SetCastShadows(true); // TODO: update common::Material
      this->SetReceiveShadows(true); // TODO: update common::Material
      this->SetReflectionEnabled(true); // TODO: update common::Material
      this->ClearNormalMap(); // TODO: update common::Material
      this->SetShaderType(ST_PIXEL); // TODO: update common::Material
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
    }
  }
}
#endif
