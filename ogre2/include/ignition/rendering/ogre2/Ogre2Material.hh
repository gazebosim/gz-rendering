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
#ifndef IGNITION_RENDERING_OGRE2_OGRE2MATERIAL_HH_
#define IGNITION_RENDERING_OGRE2_OGRE2MATERIAL_HH_

#include <string>

#include "ignition/rendering/base/BaseMaterial.hh"
#include "ignition/rendering/ogre2/Ogre2Object.hh"
#include "ignition/rendering/ogre2/Ogre2Includes.hh"

namespace ignition
{
  namespace rendering
  {
    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
    //
    class IGNITION_RENDERING_OGRE2_VISIBLE Ogre2Material :
      public BaseMaterial<Ogre2Object>
    {
      protected: Ogre2Material();

      public: virtual ~Ogre2Material();

      public: virtual bool LightingEnabled() const override;

      public: virtual void SetLightingEnabled(const bool _enabled) override;

      public: virtual math::Color Ambient() const override;

      public: virtual void SetAmbient(const math::Color &_color) override;

      public: virtual math::Color Diffuse() const override;

      public: virtual void SetDiffuse(const math::Color &_color) override;

      public: virtual math::Color Specular() const override;

      public: virtual void SetSpecular(const math::Color &_color) override;

      public: virtual math::Color Emissive() const override;

      public: virtual void SetEmissive(const math::Color &_color) override;

      public: virtual double Shininess() const override;

      public: virtual void SetShininess(const double _shininess) override;

      public: virtual double Transparency() const override;

      public: virtual void SetTransparency(const double _transparency) override;

      public: virtual double Reflectivity() const override;

      public: virtual void SetReflectivity(const double _reflectivity) override;

      public: virtual bool CastShadows() const override;

      public: virtual void SetCastShadows(const bool _castShadows) override;

      public: virtual bool ReceiveShadows() const override;

      public: virtual void SetReceiveShadows(const bool _receiveShadows)
          override;

      public: virtual bool ReflectionEnabled() const override;

      public: virtual void SetReflectionEnabled(const bool _enabled) override;

      public: virtual bool HasTexture() const override;

      public: virtual std::string Texture() const override;

      public: virtual void SetTexture(const std::string &_name) override;

      public: virtual void ClearTexture() override;

      public: virtual bool HasNormalMap() const override;

      public: virtual std::string NormalMap() const override;

      public: virtual void SetNormalMap(const std::string &_name) override;

      public: virtual void ClearNormalMap() override;

//      public: virtual enum ShaderType ShaderType() const override;

//      public: virtual void SetShaderType(enum ShaderType _type) override;

      /// \brief Return ogre low level material
      /// \return Ogre material pointer
      public: virtual Ogre::MaterialPtr Material() const;

      /// \brief Return ogre Hlms material datablock
      /// \return Ogre Hlms datablock
      public: virtual Ogre::HlmsPbsDatablock *Datablock() const;

      // Documentation inherited.
      // \sa Material::SetVertexShader(const std::string &)
//      public: virtual void SetVertexShader(const std::string &_path) override;

      // Documentation inherited.
      // \sa Material::VertexShader() const
//      public: virtual std::string VertexShader() const override;

      // Documentation inherited.
      // \sa Material::VertexShaderParams()
//      public: virtual ShaderParamsPtr VertexShaderParams() override;

      // Documentation inherited.
      // \sa Material::SetFragmentShader(const std::string &)
//      public: virtual void SetFragmentShader(const std::string &_path) override;

      // Documentation inherited.
      // \sa Material::FragmentShader() const
//      public: virtual std::string FragmentShader() const override;

      // Documentation inherited.
      // \sa Material::FragmentShaderParams()
//      public: virtual ShaderParamsPtr FragmentShaderParams() override;

      // Documentation inherited.
      // \sa BaseMaterial::PreRender()
      public: virtual void PreRender() override;

//      protected: virtual void LoadImage(const std::string &_name,
//                     Ogre::Image &_image);

      /// \brief Set the texture for this material
      /// \param[in] _texture Name of the texture.
      protected: virtual void SetTextureImpl(const std::string &_texture);

      protected: virtual void SetNormalMapImpl(const std::string &_normalMap);

//      protected: virtual Ogre::TexturePtr Texture(const std::string &_name);

//      protected: virtual Ogre::TexturePtr CreateTexture(
//                     const std::string &_name);

//      protected: virtual void UpdateTransparency();

//      protected: virtual void UpdateColorOperation();

      /// \brief bind shader parameters that have changed
//      protected: void UpdateShaderParams();

      /// \brief Transfer params from ign-rendering type to ogre type
      /// \param[in] _params ignition rendering params
      /// \param[out] _ogreParams ogre type for holding params
//      protected: void UpdateShaderParams(ConstShaderParamsPtr _params,
//        Ogre::GpuProgramParametersSharedPtr _ogreParams);

      protected: virtual void Init() override;

      protected: Ogre::MaterialPtr ogreMaterial;

//      protected: Ogre::Technique *ogreTechnique = nullptr;

//      protected: Ogre::Pass *ogrePass = nullptr;

//      protected: Ogre::TextureUnitState *ogreTexState = nullptr;

//      protected: Ogre::String ogreGroup;

      protected: Ogre::HlmsPbsDatablock *ogreDatablock = nullptr;

      protected: Ogre::HlmsPbs *ogreHlmsPbs= nullptr;

      protected: double shininess = 0.0;

      protected: double transparency = 0.0;

      protected: double reflectivity = 0.0;

      protected: bool castShadows = true;

      protected: bool reflectionEnabled = true;

      protected: std::string textureName;

      protected: std::string normalMapName;

//      protected: enum ShaderType shaderType = ST_PIXEL;

      /// \brief Path to vertex shader program.
//      protected: std::string vertexShaderPath;

      /// \brief Path to fragment shader program.
//      protected: std::string fragmentShaderPath;

      /// \brief Parameters to be bound to the vertex shader
//      protected: ShaderParamsPtr vertexShaderParams;

      /// \brief Parameters to be bound to the fragment shader
//      protected: ShaderParamsPtr fragmentShaderParams;

      private: friend class Ogre2Scene;
    };
    }
  }
}
#endif
