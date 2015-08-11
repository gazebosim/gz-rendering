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
#ifndef _IGNITION_RENDERING_BASEMATERIAL_HH_
#define _IGNITION_RENDERING_BASEMATERIAL_HH_

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

      public: virtual void SetAmbient(const gazebo::common::Color &_color) = 0;

      public: virtual void SetDiffuse(double _r, double _g, double _b,
                  double _a = 1.0);

      public: virtual void SetDiffuse(const gazebo::common::Color &_color) = 0;

      public: virtual void SetSpecular(double _r, double _g, double _b,
                  double _a = 1.0);

      public: virtual void SetSpecular(const gazebo::common::Color &_color) = 0;

      public: virtual void SetEmissive(double _r, double _g, double _b,
                  double _a = 1.0);

      public: virtual void SetEmissive(const gazebo::common::Color &_color) = 0;

      public: virtual void SetReceiveShadows(bool _receiveShadows) = 0;

      public: virtual void ClearTexture() = 0;

      public: virtual void ClearNormalMap() = 0;

      public: virtual void SetShaderType(ShaderType _type) = 0;

      public: virtual void CopyFrom(ConstMaterialPtr _material);

      public: virtual void CopyFrom(const gazebo::common::Material &_material);

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
      this->SetAmbient(gazebo::common::Color(_r, _g, _b, _a));
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseMaterial<T>::SetDiffuse(double _r, double _g, double _b, double _a)
    {
      this->SetDiffuse(gazebo::common::Color(_r, _g, _b, _a));
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseMaterial<T>::SetSpecular(double _r, double _g, double _b,
        double _a)
    {
      this->SetSpecular(gazebo::common::Color(_r, _g, _b, _a));
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseMaterial<T>::SetEmissive(double _r, double _g, double _b,
        double _a)
    {
      this->SetEmissive(gazebo::common::Color(_r, _g, _b, _a));
    }

    //////////////////////////////////////////////////
    template <class T>
    MaterialPtr BaseMaterial<T>::Clone() const
    {
      auto baseShared = this->shared_from_this();

      auto thisShared =
          boost::dynamic_pointer_cast<const BaseMaterial<T>>(baseShared);

      MaterialPtr material = T::GetScene()->CreateMaterial();
      material->CopyFrom(thisShared);
      return material;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseMaterial<T>::CopyFrom(ConstMaterialPtr _material)
    {
      this->SetLightingEnabled(_material->GetLightingEnabled());
      this->SetAmbient(_material->GetAmbient());
      this->SetDiffuse(_material->GetDiffuse());
      this->SetSpecular(_material->GetSpecular());
      this->SetEmissive(_material->GetEmissive());
      this->SetShininess(_material->GetShininess());
      this->SetTransparency(_material->GetTransparency());
      this->SetReflectivity(_material->GetReflectivity());
      this->SetReceiveShadows(_material->GetReceiveShadows());
      this->SetTexture(_material->GetTexture());
      this->SetNormalMap(_material->GetNormalMap());
      this->SetShaderType(_material->GetShaderType());
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseMaterial<T>::CopyFrom(const gazebo::common::Material &_material)
    {
      this->SetLightingEnabled(_material.GetLighting());
      this->SetAmbient(_material.GetAmbient());
      this->SetDiffuse(_material.GetDiffuse());
      this->SetSpecular(_material.GetSpecular());
      this->SetEmissive(_material.GetEmissive());
      this->SetShininess(_material.GetShininess());
      this->SetTransparency(_material.GetTransparency());
      this->SetReflectivity(0); // TODO: update gazebo::common::Material
      this->SetTexture(_material.GetTextureImage());
      this->SetReceiveShadows(true); // TODO: update gazebo::common::Material
      this->ClearNormalMap(); // TODO: update gazebo::common::Material
      this->SetShaderType(ST_PIXEL); // TODO: update gazebo::common::Material
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
      this->SetAmbient(gazebo::common::Color::White);
      this->SetDiffuse(gazebo::common::Color::White);
      this->SetSpecular(gazebo::common::Color::Black);
      this->SetEmissive(gazebo::common::Color::Black);
      this->SetShininess(0);
      this->SetTransparency(0);
      this->SetReflectivity(0);
      this->SetReceiveShadows(true);
      this->ClearTexture();
      this->ClearNormalMap();
      this->SetShaderType(ST_PIXEL);
    }
  }
}
#endif
