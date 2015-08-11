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
#ifndef _IGNITION_RENDERING_OGREMATERIAL_HH_
#define _IGNITION_RENDERING_OGREMATERIAL_HH_

#include "ignition/rendering/base/BaseMaterial.hh"
#include "ignition/rendering/ogre/OgreObject.hh"
#include "ignition/rendering/ogre/OgreIncludes.hh"

namespace ignition
{
  namespace rendering
  {
    class IGNITION_VISIBLE OgreMaterial :
      public BaseMaterial<OgreObject>
    {
      protected: OgreMaterial();

      public: virtual ~OgreMaterial();

      public: virtual bool GetLightingEnabled() const;

      public: virtual void SetLightingEnabled(bool _enabled);

      public: virtual gazebo::common::Color GetAmbient() const;

      public: virtual void SetAmbient(const gazebo::common::Color &_color);

      public: virtual gazebo::common::Color GetDiffuse() const;

      public: virtual void SetDiffuse(const gazebo::common::Color &_color);

      public: virtual gazebo::common::Color GetSpecular() const;

      public: virtual void SetSpecular(const gazebo::common::Color &_color);

      public: virtual gazebo::common::Color GetEmissive() const;

      public: virtual void SetEmissive(const gazebo::common::Color &_color);

      public: virtual double GetShininess() const;

      public: virtual void SetShininess(double _shininess);

      public: virtual double GetTransparency() const;

      public: virtual void SetTransparency(double _transparency);

      public: virtual double GetReflectivity() const;

      public: virtual void SetReflectivity(double _reflectivity);

      public: virtual bool GetReceiveShadows() const;

      public: virtual void SetReceiveShadows(bool _receiveShadows);

      public: virtual bool HasTexture() const;

      public: virtual std::string GetTexture() const;

      public: virtual void SetTexture(const std::string &_name);

      public: virtual void ClearTexture();

      public: virtual bool HasNormalMap() const;

      public: virtual std::string GetNormalMap() const;

      public: virtual void SetNormalMap(const std::string &_name);

      public: virtual void ClearNormalMap();

      public: virtual ShaderType GetShaderType() const;

      public: virtual void SetShaderType(ShaderType _type);

      public: virtual Ogre::MaterialPtr GetOgreMaterial() const;

      protected: virtual void LoadImage(const std::string &_name,
                     Ogre::Image &_image);

      protected: virtual void SetTextureImpl(Ogre::TexturePtr _texture);

      protected: virtual Ogre::TexturePtr GetTexture(const std::string &_name);

      protected: virtual Ogre::TexturePtr CreateTexture(
                     const std::string &_name);

      protected: virtual void UpdateTransparency();

      protected: virtual void UpdateColorOperation();

      protected: virtual void Init();

      protected: Ogre::MaterialPtr ogreMaterial;

      protected: Ogre::Technique *ogreTechnique;

      protected: Ogre::Pass *ogrePass;

      protected: Ogre::TextureUnitState *ogreTexState;

      protected: Ogre::String ogreGroup;

      protected: double transparency;

      protected: double reflectivity;

      protected: std::string textureName;

      protected: std::string normalMapName;

      protected: ShaderType shaderType;

      private: friend class OgreScene;
    };
  }
}
#endif
