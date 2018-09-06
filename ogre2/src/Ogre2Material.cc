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

// Note this include is placed in the src file because
// otherwise ogre produces compile errors
#include <Hlms/Pbs/OgreHlmsPbsDatablock.h>

#include <ignition/common/Console.hh>
#include <ignition/common/Filesystem.hh>

#include "ignition/rendering/ShaderParams.hh"
#include "ignition/rendering/ShaderType.hh"
#include "ignition/rendering/ogre2/Ogre2Material.hh"
#include "ignition/rendering/ogre2/Ogre2Conversions.hh"
#include "ignition/rendering/ogre2/Ogre2RenderEngine.hh"
#include "ignition/rendering/ogre2/Ogre2Scene.hh"

using namespace ignition;
using namespace rendering;

//////////////////////////////////////////////////
Ogre2Material::Ogre2Material()
{
}

//////////////////////////////////////////////////
Ogre2Material::~Ogre2Material()
{
}

//////////////////////////////////////////////////
bool Ogre2Material::LightingEnabled() const
{
  return this->lightingEnabled;
}

//////////////////////////////////////////////////
void Ogre2Material::SetLightingEnabled(bool  _enabled)
{
  this->lightingEnabled = _enabled;
  // TODO(anyone) figure out how to disable lighting in ogre2
}

//////////////////////////////////////////////////
math::Color Ogre2Material::Ambient() const
{
  // Not supported in Ogre2
  return math::Color::White;
}

//////////////////////////////////////////////////
void Ogre2Material::SetAmbient(const math::Color &/*_color*/)
{
  // Not supported in Ogre2
}

//////////////////////////////////////////////////
math::Color Ogre2Material::Diffuse() const
{
  Ogre::Vector3 diffuse =
      this->ogreDatablock->getDiffuse();
  return math::Color(diffuse.x, diffuse.y, diffuse.z, 1.0);
}

//////////////////////////////////////////////////
void Ogre2Material::SetDiffuse(const math::Color &_color)
{
  this->ogreDatablock->setDiffuse(
      Ogre::Vector3(_color.R(), _color.G(), _color.B()));
}

//////////////////////////////////////////////////
math::Color Ogre2Material::Specular() const
{
  Ogre::Vector3 specular =
      this->ogreDatablock->getSpecular();
  return math::Color(specular.x, specular.y, specular.z, 1.0);
}

//////////////////////////////////////////////////
void Ogre2Material::SetSpecular(const math::Color &_color)
{
  this->ogreDatablock->setSpecular(
      Ogre::Vector3(_color.R(), _color.G(), _color.B()));
}

//////////////////////////////////////////////////
math::Color Ogre2Material::Emissive() const
{
  Ogre::Vector3 emissive =
      this->ogreDatablock->getEmissive();
  return math::Color(emissive.x, emissive.y, emissive.z, 1.0);
}

//////////////////////////////////////////////////
void Ogre2Material::SetEmissive(const math::Color &_color)
{
  this->ogreDatablock->setEmissive(
      Ogre::Vector3(_color.R(), _color.G(), _color.B()));
}

//////////////////////////////////////////////////
double Ogre2Material::Shininess() const
{
  return 0.0;
}

//////////////////////////////////////////////////
void Ogre2Material::SetShininess(const double /*_shininess*/)
{
  // not supported
}

//////////////////////////////////////////////////
double Ogre2Material::Transparency() const
{
  return this->transparency;
}

//////////////////////////////////////////////////
void Ogre2Material::SetTransparency(const double _transparency)
{
  this->transparency = std::min(std::max(_transparency, 0.0), 1.0);
  // from ogre documentation: 0 = full transparency and 1 = fully opaque
  this->ogreDatablock->setTransparency(1-this->transparency);
}

//////////////////////////////////////////////////
double Ogre2Material::Reflectivity() const
{
  return 0.0;
}

//////////////////////////////////////////////////
void Ogre2Material::SetReflectivity(const double /*_reflectivity*/)
{
  // no supported
}

//////////////////////////////////////////////////
bool Ogre2Material::CastShadows() const
{
  return this->castShadows;
}

//////////////////////////////////////////////////
void Ogre2Material::SetCastShadows(const bool _castShadows)
{
  this->castShadows = _castShadows;
}

//////////////////////////////////////////////////
bool Ogre2Material::ReceiveShadows() const
{
  return this->ogreDatablock->getReceiveShadows();
}

//////////////////////////////////////////////////
void Ogre2Material::SetReceiveShadows(const bool _receiveShadows)
{
  this->ogreDatablock->setReceiveShadows(_receiveShadows);
}

//////////////////////////////////////////////////
bool Ogre2Material::ReflectionEnabled() const
{
  return false;
}

//////////////////////////////////////////////////
void Ogre2Material::SetReflectionEnabled(const bool /*_enabled*/)
{
  // not supported.
}

//////////////////////////////////////////////////
bool Ogre2Material::HasTexture() const
{
  return !this->textureName.empty();
}

//////////////////////////////////////////////////
std::string Ogre2Material::Texture() const
{
  return this->textureName;
}

//////////////////////////////////////////////////
void Ogre2Material::SetTexture(const std::string &_name)
{
  if (_name.empty())
  {
    this->ClearTexture();
    return;
  }

  this->textureName = _name;
  this->SetTextureImpl(this->textureName);
}

//////////////////////////////////////////////////
void Ogre2Material::ClearTexture()
{
  this->textureName = "";
  this->ogreDatablock->setTexture(Ogre::PBSM_DIFFUSE, 0, Ogre::TexturePtr());
}

//////////////////////////////////////////////////
bool Ogre2Material::HasNormalMap() const
{
  return !this->normalMapName.empty();
}

//////////////////////////////////////////////////
std::string Ogre2Material::NormalMap() const
{
  return this->normalMapName;
}

//////////////////////////////////////////////////
void Ogre2Material::SetNormalMap(const std::string &_name)
{
  if (_name.empty())
  {
    this->ClearNormalMap();
    return;
  }

  this->normalMapName = _name;
  this->SetNormalMapImpl(this->normalMapName);
}

//////////////////////////////////////////////////
void Ogre2Material::ClearNormalMap()
{
  this->normalMapName = "";
  this->ogreDatablock->setTexture(Ogre::PBSM_NORMAL, 0, Ogre::TexturePtr());
}

//////////////////////////////////////////////////
void Ogre2Material::PreRender()
{
}

//////////////////////////////////////////////////
Ogre::MaterialPtr Ogre2Material::Material()
{
  if (!this->ogreMaterial)
  {
    // low level ogre material used by render targets
    Ogre::MaterialManager &matManager = Ogre::MaterialManager::getSingleton();
    this->ogreMaterial = matManager.create(this->name,
        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
  }

  return this->ogreMaterial;
}

//////////////////////////////////////////////////
Ogre::HlmsPbsDatablock *Ogre2Material::Datablock() const
{
  return this->ogreDatablock;
}

//////////////////////////////////////////////////
void Ogre2Material::SetTextureImpl(const std::string &_texture)
{
  Ogre::HlmsTextureManager *hlmsTextureManager =
      this->ogreHlmsPbs->getHlmsManager()->getTextureManager();
  Ogre::HlmsTextureManager::TextureLocation texLocation =
      hlmsTextureManager->createOrRetrieveTexture(_texture,
      Ogre::HlmsTextureManager::TEXTURE_TYPE_DIFFUSE);

  this->ogreDatablock->setTexture(Ogre::PBSM_DIFFUSE, texLocation.xIdx,
      texLocation.texture);
}

//////////////////////////////////////////////////
void Ogre2Material::SetNormalMapImpl(const std::string &_normalMap)
{
  Ogre::HlmsTextureManager *hlmsTextureManager =
      this->ogreHlmsPbs->getHlmsManager()->getTextureManager();
  Ogre::HlmsTextureManager::TextureLocation texLocation =
      hlmsTextureManager->createOrRetrieveTexture(_normalMap,
      Ogre::HlmsTextureManager::TEXTURE_TYPE_NORMALS);

  this->ogreDatablock->setTexture(Ogre::PBSM_NORMAL, texLocation.xIdx,
      texLocation.texture);
}


//////////////////////////////////////////////////
Ogre::TexturePtr Ogre2Material::Texture(const std::string &_name)
{
  Ogre::HlmsTextureManager *hlmsTextureManager =
      this->ogreHlmsPbs->getHlmsManager()->getTextureManager();
  Ogre::HlmsTextureManager::TextureLocation texLocation =
      hlmsTextureManager->createOrRetrieveTexture(_name,
      Ogre::HlmsTextureManager::TEXTURE_TYPE_DIFFUSE);

  return texLocation.texture;
}

//////////////////////////////////////////////////
void Ogre2Material::Init()
{
  BaseMaterial::Init();

  Ogre::Root *root = Ogre2RenderEngine::Instance()->OgreRoot();
  Ogre::HlmsManager *hlmsManager = root->getHlmsManager();
  this->ogreHlmsPbs = static_cast<Ogre::HlmsPbs*>(
      hlmsManager->getHlms(Ogre::HLMS_PBS));

  if (!this->ogreHlmsPbs)
  {
    ignerr << "Ogre HLMS PBS not ready. Is Ogre2 Render Engine initiallized?"
           << std::endl;
    return;
  }
  this->ogreDatablock = static_cast<Ogre::HlmsPbsDatablock *>(
      this->ogreHlmsPbs->createDatablock(this->name, this->name,
      Ogre::HlmsMacroblock(), Ogre::HlmsBlendblock(), Ogre::HlmsParamVec()));

  this->Reset();
}
