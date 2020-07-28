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
#include <Hlms/Pbs/OgreHlmsPbsDatablock.h>
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
  public: Ogre::ParticleSystem *ps = nullptr;

  /// \brief Ogre particle emitter.
  public: Ogre::ParticleEmitter *emitter = nullptr;

  // \brief Ogre colour image affector.
  public: Ogre::ParticleAffector *colourImageAffector = nullptr;

  // \brief Ogre colour interpolator affector.
  public: Ogre::ParticleAffector *colourInterpolatorAffector = nullptr;

  /// \brief Ogre scaler affector.
  public: Ogre::ParticleAffector *scalerAffector = nullptr;

  /// \brief Pointer to the material datablock.
  public: Ogre::HlmsUnlitDatablock *ogreDatablock = nullptr;
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
    const EmitterType _type)
{
  switch (_type)
  {
    case EmitterType::EM_POINT:
    {
      this->dataPtr->ps->removeAllEmitters();
      this->dataPtr->emitter = this->dataPtr->ps->addEmitter("Point");
      break;
    }
    case EmitterType::EM_BOX:
    {
      this->dataPtr->ps->removeAllEmitters();
      this->dataPtr->emitter = this->dataPtr->ps->addEmitter("Box");
      break;
    }
    case EmitterType::EM_CYLINDER:
    {
      this->dataPtr->ps->removeAllEmitters();
      this->dataPtr->emitter = this->dataPtr->ps->addEmitter("Cylinder");
      break;
    }
    case EmitterType::EM_ELLIPSOID:
    {
      this->dataPtr->ps->removeAllEmitters();
      this->dataPtr->emitter = this->dataPtr->ps->addEmitter("Ellipsoid");
      break;
    }
    default:
    {
      ignerr << "Unsupported particle emitter type [" << _type << "]"
             << std::endl;
      return;
    }
  }

  this->type = _type;
}

//////////////////////////////////////////////////
void Ogre2ParticleEmitter::SetEmitterSize(const ignition::math::Vector3d &_size)
{
  // Sanity check: Size should be non-negative.
  if (_size[0] < 0 || _size[1] < 0 || _size[2] < 0)
  {
    ignerr << "SetEmitterSize() error: Invalid size " << _size << ". "
           << "Emitter size values should be non-negative." << std::endl;
    return;
  }

  switch (this->type)
  {
    case EmitterType::EM_POINT:
    {
      // Nothing to do.
      break;
    }
    case EmitterType::EM_BOX:
    case EmitterType::EM_CYLINDER:
    case EmitterType::EM_ELLIPSOID:
    {
      std::string widthStr  = std::to_string(_size.X());
      std::string heightStr = std::to_string(_size.Y());
      std::string depthStr  = std::to_string(_size.Z());
      std::vector<std::pair<std::string, std::string>> allParamsToSet =
        {
          {"width",  widthStr},
          {"height", heightStr},
          {"depth",  depthStr},
        };

      // Set all parameters.
      for (auto[param, value] : allParamsToSet)
      {
        if (!this->dataPtr->emitter->setParameter(param,  value))
        {
          ignerr << "Ignoring SetEmitterSize() because SetParameter("
                 << param << " " << value << ") failed." << std::endl;
          return;
        }
      }
      break;
    }
    default:
    {
      ignerr << "Unsupported particle emitter type [" << this->type
             << "]" << std::endl;
      return;
    }
  }

  this->emitterSize = _size;
}

//////////////////////////////////////////////////
void Ogre2ParticleEmitter::SetRate(double _rate)
{
  // Sanity check: Rate should be non-negative.
  if (_rate < 0)
  {
    ignerr << "SetRate() error: Invalid rate [" << _rate << "]. "
           << "The rate should be non-negative." << std::endl;
    return;
  }

  this->dataPtr->emitter->setEmissionRate(_rate);
  this->rate = _rate;
}

//////////////////////////////////////////////////
void Ogre2ParticleEmitter::SetDuration(double _duration)
{
  this->dataPtr->emitter->setDuration(_duration);
  this->duration = _duration;
}

//////////////////////////////////////////////////
void Ogre2ParticleEmitter::SetEmitting(bool _enable)
{
  this->dataPtr->emitter->setEnabled(_enable);
  this->dataPtr->ps->setEmitting(_enable);
  this->emitting = _enable;
}

//////////////////////////////////////////////////
void Ogre2ParticleEmitter::SetParticleSize(
    const ignition::math::Vector3d &_size)
{
  // Sanity check: Size should be non-negative.
  if (_size[0] < 0 || _size[1] < 0 || _size[2] < 0)
  {
    ignerr << "SetParticleSize() error: Invalid size " << _size << ". "
           << "Particle size values should be non-negative." << std::endl;
    return;
  }
  this->dataPtr->ps->setDefaultDimensions(_size[0], _size[1]);
  this->particleSize = _size;
}

//////////////////////////////////////////////////
void Ogre2ParticleEmitter::SetLifetime(double _lifetime)
{
  // Sanity check: Lifetime should be non-negative.
  if (_lifetime < 0)
  {
    ignerr << "SetLifetime() error: Invalid lifetime [" << _lifetime << "]. "
           << "The lifetime should be non-negative." << std::endl;
    return;
  }

  this->dataPtr->emitter->setTimeToLive(_lifetime);
  this->lifetime = _lifetime;
}

//////////////////////////////////////////////////
void Ogre2ParticleEmitter::SetMaterial(const MaterialPtr &_material)
{
  // Sanity check: The material cannot be nullptr.
  if (!_material)
  {
    ignerr << "SetMaterial error: material is NULL" << std::endl;
    return;
  }

  auto ogreMaterial = std::dynamic_pointer_cast<Ogre2Material>(_material);
  ogreMaterial->FillUnlitDatablock(this->dataPtr->ogreDatablock);
  this->material = _material;
}

//////////////////////////////////////////////////
void Ogre2ParticleEmitter::SetVelocityRange(double _minVelocity,
    double _maxVelocity)
{
  this->dataPtr->emitter->setParticleVelocity(_minVelocity, _maxVelocity);
  this->minVelocity = _minVelocity;
  this->maxVelocity = _maxVelocity;
}

//////////////////////////////////////////////////
void Ogre2ParticleEmitter::SetColorRange(
    const ignition::math::Color &_colorStart,
    const ignition::math::Color &_colorEnd)
{
  // Colour interpolator affector.
  if (!this->dataPtr->colourInterpolatorAffector)
  {
    this->dataPtr->colourInterpolatorAffector =
      this->dataPtr->ps->addAffector("ColourInterpolator");
  }

  std::string color0 = std::to_string(_colorStart.R()) + " " +
                       std::to_string(_colorStart.G()) + " " +
                       std::to_string(_colorStart.B()) + " " +
                       std::to_string(_colorStart.A());

  std::string color1 = std::to_string(_colorEnd.R()) + " " +
                       std::to_string(_colorEnd.G()) + " " +
                       std::to_string(_colorEnd.B()) + " " +
                       std::to_string(_colorEnd.A());

  std::vector<std::pair<std::string, std::string>> allParamsToSet =
    {
      {"time0",  "0"},
      {"colour0", color0},
      {"time1",  "1"},
      {"colour1", color1},
    };

  // Set all parameters.
  for (auto[param, value] : allParamsToSet)
  {
    if (!this->dataPtr->colourInterpolatorAffector->setParameter(param,  value))
    {
      ignerr << "Ignoring SetColorRange() because SetParameter("
             << param << " " << value << ") failed." << std::endl;
      return;
    }
  }

  this->colorStart = _colorStart;
  this->colorEnd = _colorEnd;
}

//////////////////////////////////////////////////
void Ogre2ParticleEmitter::SetScaleRate(double _scaleRate)
{
  // Sanity check: The rate should be non-negative.
  if (_scaleRate < 0)
  {
    ignerr << "SetScaleRate() error: Invalid rate [" << _scaleRate << "]. "
           << "The rate should be non-negative." << std::endl;
    return;
  }

  // Scaler affector
  if (!this->dataPtr->scalerAffector)
  {
    this->dataPtr->scalerAffector = this->dataPtr->ps->addAffector("Scaler");
  }

  std::vector<std::pair<std::string, std::string>> allParamsToSet =
    {
      {"rate",  std::to_string(_scaleRate)},
    };

  // Set all parameters.
  for (auto[param, value] : allParamsToSet)
  {
    if (!this->dataPtr->scalerAffector->setParameter(param,  value))
    {
      ignerr << "Ignoring SetScaleRate() because SetParameter("
             << param << " " << value << ") failed." << std::endl;
      return;
    }
  }

  this->scaleRate = _scaleRate;
}

//////////////////////////////////////////////////
void Ogre2ParticleEmitter::SetColorImage(const std::string &_image)
{
  // Sanity check: Make sure that the texture can be found.
  if (!common::exists(_image) || !common::isFile(_image))
  {
    ignerr << "SetColorImage() error: Texture [" << _image << "] not found"
           << std::endl;
    return;
  }

  // Colour image affector.
  if (!this->dataPtr->colourImageAffector)
  {
    this->dataPtr->colourImageAffector =
      this->dataPtr->ps->addAffector("ColourImage");
  }

  std::vector<std::pair<std::string, std::string>> allParamsToSet =
    {
      {"image",  _image},
    };

  // Set all parameters.
  for (auto[param, value] : allParamsToSet)
  {
    if (!this->dataPtr->colourImageAffector->setParameter(param,  value))
    {
      ignerr << "Ignoring SetColorImage() because SetParameter("
             << param << " " << value << ") failed." << std::endl;
      return;
    }
  }

  this->colorImage = _image;
}

//////////////////////////////////////////////////
void Ogre2ParticleEmitter::Init()
{
  Ogre2Visual::Init();

  // Instantiate the particle system and default parameters.
  this->dataPtr->ps = this->scene->OgreSceneManager()->createParticleSystem();
  this->dataPtr->ps->setCullIndividually(true);
  this->dataPtr->ps->setParticleQuota(500);
  this->dataPtr->ps->setSortingEnabled(true);

  // Instantiate the particle emitter and default parameters.
  this->dataPtr->emitter = this->dataPtr->ps->addEmitter("Point");
  this->dataPtr->emitter->setDirection(Ogre::Vector3::UNIT_X);

  // Instantiate the default material.
  auto mat = this->scene->CreateMaterial();
  auto ogreMat = std::dynamic_pointer_cast<Ogre2Material>(mat);
  this->dataPtr->ogreDatablock = ogreMat->UnlitDatablock();
  this->dataPtr->ps->setMaterialName(
      *(this->dataPtr->ogreDatablock->getNameStr()));

  // Default emitter parameters.
  this->SetParticleSize({1, 1, 1});

  this->ogreNode->attachObject(this->dataPtr->ps);
  igndbg << "Particle emitter initialized" << std::endl;
}
