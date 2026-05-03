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

#include <gz/common/Console.hh>

#include "gz/rendering/ogre_next/OgreNextLight.hh"

#include "gz/rendering/ogre_next/OgreNextConversions.hh"
#include "gz/rendering/ogre_next/OgreNextScene.hh"

#ifdef _MSC_VER
  #pragma warning(push, 0)
#endif
#include <OgreSceneManager.h>
#ifdef _MSC_VER
  #pragma warning(pop)
#endif

/// \brief Private data for the OgreNextLight class
class gz::rendering::OgreNextLightPrivate
{
};

using namespace gz;
using namespace rendering;

//////////////////////////////////////////////////
// OgreNextLight
//////////////////////////////////////////////////
OgreNextLight::OgreNextLight() :
  ogreLightType(Ogre::Light::LT_POINT),
  dataPtr(std::make_unique<OgreNextLightPrivate>())
{
}

//////////////////////////////////////////////////
OgreNextLight::~OgreNextLight()
{
}

//////////////////////////////////////////////////
void OgreNextLight::SetLocalPosition(const math::Vector3d &_position)
{
  OgreNextNode::SetLocalPosition(_position);
  this->scene->SetLightsGiDirty();
}

//////////////////////////////////////////////////
math::Color OgreNextLight::DiffuseColor() const
{
  Ogre::ColourValue color = this->ogreLight->getDiffuseColour();
  return OgreNextConversions::Convert(color);
}

//////////////////////////////////////////////////
void OgreNextLight::SetDiffuseColor(const math::Color &_color)
{
  this->ogreLight->setDiffuseColour(_color.R(), _color.G(), _color.B());
  this->scene->SetLightsGiDirty();
}

//////////////////////////////////////////////////
math::Color OgreNextLight::SpecularColor() const
{
  Ogre::ColourValue color = this->ogreLight->getSpecularColour();
  return OgreNextConversions::Convert(color);
}

//////////////////////////////////////////////////
void OgreNextLight::SetSpecularColor(const math::Color &_color)
{
  this->ogreLight->setSpecularColour(_color.R(), _color.G(), _color.B());
}

//////////////////////////////////////////////////
double OgreNextLight::AttenuationConstant() const
{
  return this->ogreLight->getAttenuationConstant();
}

//////////////////////////////////////////////////
void OgreNextLight::SetAttenuationConstant(double _value)
{
  this->attenConstant = _value;
  this->UpdateAttenuation();
}

//////////////////////////////////////////////////
double OgreNextLight::AttenuationLinear() const
{
  return this->ogreLight->getAttenuationLinear();
}

//////////////////////////////////////////////////
void OgreNextLight::SetAttenuationLinear(double _value)
{
  this->attenLinear = _value;
  this->UpdateAttenuation();
}

//////////////////////////////////////////////////
double OgreNextLight::AttenuationQuadratic() const
{
  return this->ogreLight->getAttenuationQuadric();
}

//////////////////////////////////////////////////
void OgreNextLight::SetAttenuationQuadratic(double _value)
{
  this->attenQuadratic = _value;
  this->UpdateAttenuation();
}

//////////////////////////////////////////////////
double OgreNextLight::AttenuationRange() const
{
  return this->ogreLight->getAttenuationRange();
}

//////////////////////////////////////////////////
void OgreNextLight::SetAttenuationRange(double _range)
{
  this->attenRange = _range;
  this->UpdateAttenuation();
}

//////////////////////////////////////////////////
double OgreNextLight::Intensity() const
{
  return this->ogreLight->getPowerScale() / GZ_PI;
}

//////////////////////////////////////////////////
void OgreNextLight::SetIntensity(double _intensity)
{
  this->ogreLight->setPowerScale(_intensity * GZ_PI);
  this->scene->SetLightsGiDirty();
}

//////////////////////////////////////////////////
bool OgreNextLight::CastShadows() const
{
  return this->ogreLight->getCastShadows();
}

//////////////////////////////////////////////////
void OgreNextLight::SetCastShadows(bool _castShadows)
{
  this->ogreLight->setCastShadows(_castShadows);
  this->scene->SetShadowsDirty(true);
}

//////////////////////////////////////////////////
Ogre::Light *OgreNextLight::Light() const
{
  return this->ogreLight;
}

//////////////////////////////////////////////////
void OgreNextLight::Destroy()
{
  BaseLight::Destroy();
  Ogre::SceneManager *ogreSceneManager = this->scene->OgreSceneManager();
  ogreSceneManager->destroySceneNode(this->ogreLight->getParentSceneNode());
  ogreSceneManager->destroyLight(this->ogreLight);
  this->scene->SetLightsGiDirty();
}

//////////////////////////////////////////////////
void OgreNextLight::Init()
{
  OgreNextNode::Init();
  this->CreateLight();
  this->Reset();
}

//////////////////////////////////////////////////
void OgreNextLight::CreateLight()
{
  Ogre::SceneManager *sceneManager;
  sceneManager = this->scene->OgreSceneManager();
  this->ogreLight = sceneManager->createLight();
  this->ogreLight->setType(this->ogreLightType);
  // create an intermediate scene node to hold light object otherwise
  // functions that update the light pose will affect the light direction
  this->ogreNode->createChildSceneNode()->attachObject(this->ogreLight);
  this->ogreLight->setCastShadows(true);
  this->ogreLight->setPowerScale(Ogre::Math::PI);
  this->UpdateAttenuation();
}

//////////////////////////////////////////////////
void OgreNextLight::UpdateAttenuation()
{
  this->ogreLight->setAttenuation(this->attenRange, this->attenConstant,
      this->attenLinear, this->attenQuadratic);
  this->scene->SetLightsGiDirty();
}

//////////////////////////////////////////////////
// OgreNextDirectionalLight
//////////////////////////////////////////////////
OgreNextDirectionalLight::OgreNextDirectionalLight()
{
  this->ogreLightType = Ogre::Light::LT_DIRECTIONAL;
}

//////////////////////////////////////////////////
OgreNextDirectionalLight::~OgreNextDirectionalLight()
{
}

//////////////////////////////////////////////////
math::Vector3d OgreNextDirectionalLight::Direction() const
{
  return OgreNextConversions::Convert(this->ogreLight->getDirection());
}

//////////////////////////////////////////////////
void OgreNextDirectionalLight::SetDirection(const math::Vector3d &_dir)
{
  this->ogreLight->setDirection(OgreNextConversions::Convert(_dir));
  this->scene->SetLightsGiDirty();
}

//////////////////////////////////////////////////
// OgrePointLight
//////////////////////////////////////////////////
OgreNextPointLight::OgreNextPointLight()
{
  this->ogreLightType = Ogre::Light::LT_POINT;
}

//////////////////////////////////////////////////
OgreNextPointLight::~OgreNextPointLight()
{
}

//////////////////////////////////////////////////
// OgreSpotLight
//////////////////////////////////////////////////
OgreNextSpotLight::OgreNextSpotLight()
{
  this->ogreLightType = Ogre::Light::LT_SPOTLIGHT;
}

//////////////////////////////////////////////////
OgreNextSpotLight::~OgreNextSpotLight()
{
}

//////////////////////////////////////////////////
math::Vector3d OgreNextSpotLight::Direction() const
{
  return OgreNextConversions::Convert(this->ogreLight->getDirection());
}

//////////////////////////////////////////////////
void OgreNextSpotLight::SetDirection(const math::Vector3d &_dir)
{
  this->ogreLight->setDirection(OgreNextConversions::Convert(_dir));
  this->scene->SetLightsGiDirty();
}

//////////////////////////////////////////////////
math::Angle OgreNextSpotLight::InnerAngle() const
{
  return OgreNextConversions::Convert(
      this->ogreLight->getSpotlightInnerAngle());
}

//////////////////////////////////////////////////
void OgreNextSpotLight::SetInnerAngle(const math::Angle &_angle)
{
  this->ogreLight->setSpotlightInnerAngle(OgreNextConversions::Convert(_angle));
  this->scene->SetLightsGiDirty();
}

//////////////////////////////////////////////////
math::Angle OgreNextSpotLight::OuterAngle() const
{
  return OgreNextConversions::Convert(
      this->ogreLight->getSpotlightOuterAngle());
}

//////////////////////////////////////////////////
void OgreNextSpotLight::SetOuterAngle(const math::Angle &_angle)
{
  this->ogreLight->setSpotlightOuterAngle(OgreNextConversions::Convert(_angle));
  this->scene->SetLightsGiDirty();
}

//////////////////////////////////////////////////
double OgreNextSpotLight::Falloff() const
{
  return this->ogreLight->getSpotlightFalloff();
}

//////////////////////////////////////////////////
void OgreNextSpotLight::SetFalloff(double _falloff)
{
  this->ogreLight->setSpotlightFalloff(_falloff);
  this->scene->SetLightsGiDirty();
}

//////////////////////////////////////////////////
