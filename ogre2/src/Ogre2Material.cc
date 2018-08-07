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
  return true;
}

//////////////////////////////////////////////////
void Ogre2Material::SetLightingEnabled(bool /*_enabled*/)
{
  // Not supported in Ogre2
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
  return this->shininess;
}

//////////////////////////////////////////////////
void Ogre2Material::SetShininess(const double _shininess)
{
  this->shininess = _shininess;
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
  return this->reflectivity;
}

//////////////////////////////////////////////////
void Ogre2Material::SetReflectivity(const double _reflectivity)
{
  this->reflectivity = std::min(std::max(_reflectivity, 0.0), 1.0);
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
//  return this->reflectionEnabled;
  return false;
}

//////////////////////////////////////////////////
void Ogre2Material::SetReflectionEnabled(const bool _enabled)
{
  this->reflectionEnabled = _enabled;
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
  Ogre::HlmsTextureManager *hlmsTextureManager =
      this->ogreHlmsPbs->getHlmsManager()->getTextureManager();
  this->ogreDatablock->setTexture(Ogre::PBSM_DIFFUSE, 0,
      hlmsTextureManager->getBlankTexture().texture);
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

  Ogre::HlmsTextureManager *hlmsTextureManager =
      this->ogreHlmsPbs->getHlmsManager()->getTextureManager();
  this->ogreDatablock->setTexture(Ogre::PBSM_NORMAL, 0,
      hlmsTextureManager->getBlankTexture().texture);
}

//////////////////////////////////////////////////
void Ogre2Material::PreRender()
{
  // this->UpdateShaderParams();
}
/*
//////////////////////////////////////////////////
enum ShaderType Ogre2Material::ShaderType() const
{
  return this->shaderType;
}

//////////////////////////////////////////////////
void Ogre2Material::SetShaderType(enum ShaderType _type)
{
  this->shaderType = (ShaderUtil::IsValid(_type)) ? _type : ST_PIXEL;
}
//////////////////////////////////////////////////
void Ogre2Material::UpdateShaderParams()
{
  if (this->vertexShaderParams && this->vertexShaderParams->IsDirty())
  {
    Ogre::GpuProgramParametersSharedPtr ogreParams;
    ogreParams = this->ogrePass->getVertexProgramParameters();
    this->UpdateShaderParams(this->vertexShaderParams, ogreParams);
    this->vertexShaderParams->ClearDirty();
  }
  if (this->fragmentShaderParams && this->fragmentShaderParams->IsDirty())
  {
    Ogre::GpuProgramParametersSharedPtr ogreParams;
    ogreParams = this->ogrePass->getFragmentProgramParameters();
    this->UpdateShaderParams(this->fragmentShaderParams, ogreParams);
    this->fragmentShaderParams->ClearDirty();
  }
}

//////////////////////////////////////////////////
void Ogre2Material::UpdateShaderParams(ConstShaderParamsPtr _params,
    Ogre::GpuProgramParametersSharedPtr _ogreParams)
{
  for (const auto name_param : *_params)
  {
    if (ShaderParam::PARAM_FLOAT == name_param.second.Type())
    {
      float value;
      name_param.second.Value(&value);
      _ogreParams->setNamedConstant(name_param.first, value);
    }
    else if (ShaderParam::PARAM_INT == name_param.second.Type())
    {
      int value;
      name_param.second.Value(&value);
      _ogreParams->setNamedConstant(name_param.first, value);
    }
  }
}

//////////////////////////////////////////////////
void Ogre2Material::SetVertexShader(const std::string &_path)
{
  if (_path.empty())
    return;

  if (!common::exists(_path))
  {
    ignerr << "Vertex shader path does not exist: " << _path << std::endl;
    return;
  }

  Ogre::ResourceGroupManager::getSingleton().addResourceLocation(_path,
  "FileSystem", "General", false);

  Ogre::HighLevelGpuProgramPtr vertexShader =
    Ogre::HighLevelGpuProgramManager::getSingletonPtr()->createProgram(
        "__ignition_rendering_vertex__" + _path,
        this->ogreGroup,
        "glsl", Ogre::GpuProgramType::GPT_VERTEX_PROGRAM);

  vertexShader->setSourceFile(_path);
  vertexShader->load();

  assert(vertexShader->isLoaded());
  assert(!(vertexShader->hasCompileError()));
  assert(vertexShader->isSupported());

  this->ogrePass->setVertexProgram(vertexShader->getName());

  this->ogreMaterial->compile();
  this->ogreMaterial->load();

  this->vertexShaderPath = _path;
  this->vertexShaderParams.reset(new ShaderParams);
}

//////////////////////////////////////////////////
std::string Ogre2Material::VertexShader() const
{
  return this->vertexShaderPath;
}

//////////////////////////////////////////////////
ShaderParamsPtr Ogre2Material::VertexShaderParams()
{
//  return this->vertexShaderParams;
  return ShaderParamsPtr();
}

//////////////////////////////////////////////////
void Ogre2Material::SetFragmentShader(const std::string &_path)
{
  if (_path.empty())
    return;

  if (!common::exists(_path))
  {
    ignerr << "Fragment shader path does not exist: " << _path << std::endl;
    return;
  }

  Ogre::ResourceGroupManager::getSingleton().addResourceLocation(_path,
  "FileSystem", "General", false);

  Ogre::HighLevelGpuProgramPtr fragmentShader =
    Ogre::HighLevelGpuProgramManager::getSingleton().createProgram(
        "__ignition_rendering_fragment__" + _path,
        this->ogreGroup,
        "glsl", Ogre::GpuProgramType::GPT_FRAGMENT_PROGRAM);

  fragmentShader->setSourceFile(_path);
  fragmentShader->load();

  assert(fragmentShader->isLoaded());
  assert(!(fragmentShader->hasCompileError()));
  assert(fragmentShader->isSupported());

  this->ogrePass->setFragmentProgram(fragmentShader->getName());

  this->ogreMaterial->setLightingEnabled(false);

  this->ogreMaterial->compile();
  this->ogreMaterial->load();

  this->fragmentShaderPath = _path;
  this->fragmentShaderParams.reset(new ShaderParams);
}

//////////////////////////////////////////////////
std::string Ogre2Material::FragmentShader() const
{
  return this->fragmentShaderPath;
}

//////////////////////////////////////////////////
ShaderParamsPtr Ogre2Material::FragmentShaderParams()
{
//  return this->fragmentShaderParams;
  return ShaderParamsPtr();
}
*/
//////////////////////////////////////////////////
Ogre::MaterialPtr Ogre2Material::Material() const
{
  return this->ogreMaterial;
}

//////////////////////////////////////////////////
Ogre::HlmsPbsDatablock *Ogre2Material::Datablock() const
{
  return this->ogreDatablock;
}
/*
//////////////////////////////////////////////////
void Ogre2Material::LoadImage(const std::string &_name, Ogre::Image &_image)
{
  try
  {
    if (Ogre::ResourceGroupManager::getSingleton().resourceExists(
        this->ogreGroup, _name))
    {
      _image.load(_name, this->ogreGroup);
    }
    else
    {
      std::string path = common::findFile(_name);
      if (!path.empty())
      {
        Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
            path, "FileSystem", this->ogreGroup);
        _image.load(path, this->ogreGroup);
      }
      else
        ignerr << "Unable to find texture image: " << _name << std::endl;
    }
  }
  catch (const Ogre::Exception &ex)
  {
    ignerr << "Unable to load texture image: " << ex.what() << std::endl;
  }
}

  */
//////////////////////////////////////////////////
void Ogre2Material::SetTextureImpl(const std::string &_texture)
{
/*  if (!Ogre::ResourceGroupManager::getSingleton().resourceExists(
      this->ogreGroup, _texture))
  {
    Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
        _texture, "FileSystem", this->ogreGroup);
  }

  this->ogreTexState->setTextureName(_texture);
  this->UpdateColorOperation();
  */
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

/*
//////////////////////////////////////////////////
Ogre::TexturePtr Ogre2Material::Texture(const std::string &_name)
{
  Ogre::TextureManager &texManager = Ogre::TextureManager::getSingleton();

  if (texManager.resourceExists(_name))
  {
    return texManager.getByName(_name);
  }

  return this->CreateTexture(_name);
  // return Ogre::TexturePtr();
}
  */
/*
//////////////////////////////////////////////////
Ogre::TexturePtr Ogre2Material::CreateTexture(const std::string &_name)
{
  Ogre::Image image;
  Ogre::TexturePtr texture;

  this->LoadImage(_name, image);

  if (image.getWidth() == 0)
  {
    texture.setNull();
    return texture;
  }

  texture = Ogre::TextureManager::getSingleton().createManual(_name,
      this->ogreGroup, Ogre::TEX_TYPE_2D, image.getWidth(),
      image.getHeight(), 0, Ogre::PF_X8R8G8B8);

  texture->loadImage(image);
  return texture;
  // return Ogre::TexturePtr();
}

  */
/*
//////////////////////////////////////////////////
void Ogre2Material::UpdateColorOperation()
{
  Ogre::LayerBlendOperationEx operation;
  Ogre::LayerBlendSource source1;
  Ogre::LayerBlendSource source2;
  Ogre::ColourValue color;

  bool texOff = !this->HasTexture();
  bool lightOff = !this->LightingEnabled();

  operation = (texOff) ? Ogre::LBX_SOURCE1 : Ogre::LBX_MODULATE;
  source1 = (texOff && lightOff) ? Ogre::LBS_MANUAL : Ogre::LBS_CURRENT;
  source2 = Ogre::LBS_TEXTURE;
  color = this->ogrePass->getAmbient();

  this->ogreTexState->setColourOperationEx(operation, source1, source2, color);
}

  */

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
  this->ogreDatablock =  static_cast<Ogre::HlmsPbsDatablock*>(
      this->ogreHlmsPbs->createDatablock(this->name,
                                     this->name,
                                     Ogre::HlmsMacroblock(),
                                     Ogre::HlmsBlendblock(),
                                     Ogre::HlmsParamVec()));

  this->Reset();

/*  this->ogreGroup = Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME;
  Ogre::MaterialManager &matManager = Ogre::MaterialManager::getSingleton();
  this->ogreMaterial = matManager.create(this->name, this->ogreGroup);
  this->ogreTechnique = this->ogreMaterial->getTechnique(0);
  this->ogrePass = this->ogreTechnique->getPass(0);
  this->ogreTexState = this->ogrePass->createTextureUnitState();
  this->ogreTexState->setBlank();
  this->Reset();

  // TODO: provide function interface
  this->ogreMaterial->setTextureAnisotropy(8);
  */
}

//////////////////////////////////////////////////
