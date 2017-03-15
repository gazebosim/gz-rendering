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
#ifndef IGNITION_RENDERING_OPTIXMATERIAL_HH_
#define IGNITION_RENDERING_OPTIXMATERIAL_HH_

#include "ignition/rendering/base/BaseMaterial.hh"
#include "ignition/rendering/optix/OptixObject.hh"
#include "ignition/rendering/optix/OptixIncludes.hh"

namespace ignition
{
  namespace rendering
  {
    class IGNITION_VISIBLE OptixMaterial :
      public BaseMaterial<OptixObject>
    {
      protected: OptixMaterial();

      public: virtual ~OptixMaterial();

      public: virtual bool LightingEnabled() const;

      public: virtual void SetLightingEnabled(bool _enabled);

      public: virtual math::Color Ambient() const;

      public: virtual void SetAmbient(const math::Color &_color);

      public: virtual math::Color Diffuse() const;

      public: virtual void SetDiffuse(const math::Color &_color);

      public: virtual math::Color Specular() const;

      public: virtual void SetSpecular(const math::Color &_color);

      public: virtual math::Color Emissive() const;

      public: virtual void SetEmissive(const math::Color &_color);

      public: virtual double Shininess() const;

      public: virtual void SetShininess(double _shininess);

      public: virtual double Transparency() const;

      public: virtual void SetTransparency(double _transparency);

      public: virtual double Reflectivity() const;

      public: virtual void SetReflectivity(double _reflectivity);

      public: virtual bool CastShadows() const;

      public: virtual void SetCastShadows(bool _castShadows);

      public: virtual bool ReceiveShadows() const;

      public: virtual void SetReceiveShadows(bool _receiveShadows);

      public: virtual bool ReflectionEnabled() const;

      public: virtual void SetReflectionEnabled(bool _enabled);

      public: virtual bool HasTexture() const;

      public: virtual std::string Texture() const;

      public: virtual void SetTexture(const std::string &_name);

      public: virtual void ClearTexture();

      public: virtual bool HasNormalMap() const;

      public: virtual std::string NormalMap() const;

      public: virtual void SetNormalMap(const std::string &_name);

      public: virtual void ClearNormalMap();

      public: virtual enum ShaderType ShaderType() const;

      public: virtual void SetShaderType(enum ShaderType _type);

      public: virtual void PreRender();

      public: virtual void Destroy();

      public: virtual optix::Material Material() const;

      protected: virtual void WriteColorToDevice();

      protected: virtual void WriteTextureToDevice();

      protected: virtual void WriteNormalMapToDevice();

      protected: virtual void WriteColorToDeviceImpl();

      protected: virtual void WriteTextureToDeviceImpl();

      protected: virtual void WriteNormalMapToDeviceImpl();

      protected: virtual void Init();

      protected: bool colorDirty;

      protected: bool textureDirty;

      protected: bool normalMapDirty;

      protected: bool lightingEnabled;

      protected: math::Color ambient;

      protected: math::Color diffuse;

      protected: math::Color specular;

      protected: math::Color emissive;

      protected: double shininess;

      protected: double transparency;

      protected: double reflectivity;

      protected: bool castShadows;

      protected: bool receiveShadows;

      protected: bool reflectionEnabled;

      protected: std::string textureName;

      protected: std::string normalMapName;

      protected: enum ShaderType shaderType;

      protected: optix::Material optixMaterial;

      protected: optix::TextureSampler optixTexture;

      protected: optix::TextureSampler optixNormalMap;

      protected: optix::TextureSampler optixEmptyTexture;

      private: static const std::string PTX_FILE_BASE;

      private: static const std::string PTX_CLOSEST_HIT_FUNC;

      private: static const std::string PTX_ANY_HIT_FUNC;

      private: friend class OptixScene;
    };
  }
}
#endif
