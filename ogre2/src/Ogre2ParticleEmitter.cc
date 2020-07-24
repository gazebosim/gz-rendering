/*
 * Copyright (C) 2020 Open Source Robotics Foundation
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
#include <Hlms/Unlit/OgreHlmsUnlitDatablock.h>

#include "ignition/rendering/ogre2/Ogre2Conversions.hh"
#include "ignition/rendering/ogre2/Ogre2Includes.hh"
#include "ignition/rendering/ogre2/Ogre2Material.hh"
#include "ignition/rendering/ogre2/Ogre2ParticleEmitter.hh"
#include "ignition/rendering/ogre2/Ogre2RenderEngine.hh"
#include "ignition/rendering/ogre2/Ogre2Scene.hh"

using namespace ignition;
using namespace rendering;

class ignition::rendering::Ogre2ParticleEmitterPrivate
{
  /// \brief Internal material name.
  public: const std::string kMaterialName = "__particle_emitter_material__";

  /// \brief Ogre particle system.
  public: Ogre::ParticleSystem *ps;

  /// \brief Ogre particle emitter.
  public: Ogre::ParticleEmitter *emitter;

  /// \brief Ogre unlit material.
  public: Ogre::HlmsUnlit *ogreHlmsUnlit;

  /// \brief The datablock Id.
  public: std::string ogreDatablockId;

  /// \brief Pointer to the material datablock.
  public: Ogre::HlmsUnlitDatablock *ogreDatablock;
};

//////////////////////////////////////////////////
Ogre2ParticleEmitter::Ogre2ParticleEmitter()
    : dataPtr(new Ogre2ParticleEmitterPrivate)
{
}

//////////////////////////////////////////////////
Ogre2ParticleEmitter::~Ogre2ParticleEmitter()
{
}

//////////////////////////////////////////////////
void Ogre2ParticleEmitter::Ogre2ParticleEmitter::SetType(
    const EmitterType /*_type*/)
{
}

//////////////////////////////////////////////////
void Ogre2ParticleEmitter::SetEmitterSize(
    const ignition::math::Vector3d &/*_size*/)
{
}

//////////////////////////////////////////////////
void Ogre2ParticleEmitter::SetRate(double _rate)
{
  this->dataPtr->emitter->setEmissionRate(_rate);
}

//////////////////////////////////////////////////
void Ogre2ParticleEmitter::SetDuration(double _duration)
{
  this->dataPtr->emitter->setDuration(_duration);
}

//////////////////////////////////////////////////
void Ogre2ParticleEmitter::SetEmitting(bool _enable)
{
  this->dataPtr->emitter->setEnabled(_enable);
  this->dataPtr->ps->setEmitting(_enable);
}

//////////////////////////////////////////////////
void Ogre2ParticleEmitter::SetParticleSize(
    const ignition::math::Vector3d &_size)
{
  this->dataPtr->ps->setDefaultDimensions(_size[0], _size[1]);
}

//////////////////////////////////////////////////
void Ogre2ParticleEmitter::SetLifetime(double _lifetime)
{
  this->dataPtr->emitter->setTimeToLive(_lifetime);
}

//////////////////////////////////////////////////
void Ogre2ParticleEmitter::SetMaterial(const MaterialPtr &_material)
{
  // Color.
  math::Color color = _material->Diffuse();
  this->dataPtr->ogreDatablock->setColour(Ogre2Conversions::Convert(color));

  // TODO: Other atributes (texture?)
}

//////////////////////////////////////////////////
void Ogre2ParticleEmitter::SetVelocityRange(double _minVelocity,
    double _maxVelocity)
{
  this->dataPtr->emitter->setParticleVelocity(_minVelocity, _maxVelocity);
}

//////////////////////////////////////////////////
void Ogre2ParticleEmitter::SetColorRange(
    const ignition::math::Color &_colorStart,
    const ignition::math::Color &_colorEnd)
{
  this->dataPtr->emitter->setColour(Ogre2Conversions::Convert(_colorStart),
    Ogre2Conversions::Convert(_colorEnd));
}

//////////////////////////////////////////////////
void Ogre2ParticleEmitter::Init()
{
  Ogre2Visual::Init();

  Ogre::SceneManager *ogreSceneManager = this->scene->OgreSceneManager();

  // Instantiate the particle system and default parameters.
  this->dataPtr->ps = ogreSceneManager->createParticleSystem();
  this->dataPtr->ps->setCullIndividually(true);
  this->dataPtr->ps->setParticleQuota(500);
  this->dataPtr->ps->setSortingEnabled(true);

  // Instantiate the particle emitter and default parameters.
  this->dataPtr->emitter = this->dataPtr->ps->addEmitter("Point");
  this->dataPtr->emitter->setDirection(Ogre::Vector3::UNIT_Z);

  // Create the internal UNLIT material.
  Ogre::Root *root = Ogre2RenderEngine::Instance()->OgreRoot();
  Ogre::HlmsManager *hlmsManager = root->getHlmsManager();
  this->dataPtr->ogreHlmsUnlit = static_cast<Ogre::HlmsUnlit*>(
      hlmsManager->getHlms(Ogre::HLMS_UNLIT));

  if (!this->dataPtr->ogreHlmsUnlit)
  {
    ignerr << "Ogre HLMS UNLIT not ready. Is Ogre2 Render Engine initiallized?"
           << std::endl;
    return;
  }

  this->dataPtr->ogreDatablockId = this->dataPtr->kMaterialName;
  this->dataPtr->ogreDatablock = static_cast<Ogre::HlmsUnlitDatablock *>(
    this->dataPtr->ogreHlmsUnlit->createDatablock(
      this->dataPtr->ogreDatablockId, this->dataPtr->kMaterialName,
      Ogre::HlmsMacroblock(), Ogre::HlmsBlendblock(), Ogre::HlmsParamVec()));

  // This is the default material.
  this->dataPtr->ps->setMaterialName(this->dataPtr->kMaterialName);

  this->ogreNode->attachObject(this->dataPtr->ps);
  igndbg << "Particle emitter initialized" << std::endl;
}
