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

#ifndef IGNITION_RENDERING_BASE_BASEPARTICLEEMITTER_HH_
#define IGNITION_RENDERING_BASE_BASEPARTICLEEMITTER_HH_

#include "ignition/rendering/base/BaseScene.hh"
#include "ignition/rendering/base/BaseNode.hh"
#include "ignition/rendering/ParticleEmitter.hh"

namespace ignition
{
  namespace rendering
  {
    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
    //
    /* \class BaseParticleEmitter BaseParticleEmitter.hh \
     * ignition/rendering/base/BaseParticleEmitter.hh
     */
    /// \brief A base implementation of the ParticleEmitter class
    template <class T>
    class BaseParticleEmitter :
      public virtual ParticleEmitter,
      public virtual T
    {
      /// \brief Constructor
      protected: BaseParticleEmitter();

      /// \brief Destructor
      public: virtual ~BaseParticleEmitter();

      // Documentation inherited
      public: virtual void Init() override;

      // Documentation inherited
      public: virtual void PreRender() override;

      /// \brief Reset the particle emitter visual state
      public: virtual void Reset();

      // Documentation inherited.
      public: virtual void SetType(const EmitterType _type) override;

      // Documentation inherited.
      public: virtual void SetEmitterSize(
                  const ignition::math::Vector3d &_size) override;

      // Documentation inherited.
      public: virtual void SetRate(const double _rate) override;

      // Documentation inherited.
      public: virtual void SetLocalPose(
                  const ignition::math::Pose3d &_pose) override;

      // Documentation inherited.
      public: virtual void SetDuration(const double &_duration) override;

      // Documentation inherited.
      public: virtual void SetEmitting(const bool _enable) override;

      // Documentation inherited.
      public: virtual void SetParticleSize(
                  const ignition::math::Vector3d &_size) override;

      // Documentation inherited.
      public: virtual void SetLifetime(const double &_lifetime) override;

      // Documentation inherited.
      public: virtual void SetMaterial(const MaterialPtr &_material) override;

      // Documentation inherited.
      public: virtual void SetVelocityRange(const double _minVel,
                                            const double _maxVel) override;

      // Documentation inherited.
      public: virtual void SetColorRange(
                  const ignition::math::Color &_colorStart,
                  const ignition::math::Color &_colorEnd) override;

      /// \brief Emitter type.
      protected: EmitterType type = EM_POINT;

      /// \brief Emitter size.
      protected: ignition::math::Vector3d emitterSize =
          ignition::math::Pose3d::One;

      /// \brief Rate of emission.
      protected: double rate = 10;

      /// \brief Emitter pose.
      protected: ignition::math::Pose3d pose = ignition::math::Pose3d::Zero;

      /// \brief Duration of the emitter.
      protected: double duration = 0;

      /// \brief Whether particle emitter is enabled or not.
      protected: bool emitting = false;

      /// \brief Particle size.
      protected: ignition::math::Vector3d size = {100, 100, 100};

      /// \brief Particle lifetime.
      protected: double lifetime = 5;

      /// \brief Particle material.
      protected: MaterialPtr material = nullptr;

      /// \brief The minimum velocity of each particle.
      protected: double minVel = 1;

      /// \brief The maximum velocity of each particle.
      protected: double maxVel = 1;

      /// \brief The start color of a particle to choose from.
      protected: ignition::math::Color colorStart =
          ignition::math::Color::Black;

      /// \brief The end color of a particle to choose from.
      protected: ignition::math::Color colorEnd =
          ignition::math::Color::White;

      /// \brief Only the scene can create a particle emitter
      private: friend class BaseScene;
    };

    //////////////////////////////////////////////////
    template <class T>
    BaseParticleEmitter<T>::BaseParticleEmitter()
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    BaseParticleEmitter<T>::~BaseParticleEmitter()
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseParticleEmitter<T>::Init()
    {
      T::Init();
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseParticleEmitter<T>::Reset()
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseParticleEmitter<T>::PreRender()
    {
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseParticleEmitter<T>::SetType(const EmitterType /*_type*/)
    {
      // no op
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseParticleEmitter<T>::SetEmitterSize(
                const ignition::math::Vector3d &/*_size*/)
    {
      // no op
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseParticleEmitter<T>::SetRate(const double /*_rate*/)
    {
      // no op
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseParticleEmitter<T>::SetLocalPose(
                const ignition::math::Pose3d &/*_pose*/)
    {
      // no op
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseParticleEmitter<T>::SetDuration(const double &/*_duration*/)
    {
      // no op
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseParticleEmitter<T>::SetEmitting(const bool /*_enable*/)
    {
      // no op
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseParticleEmitter<T>::SetParticleSize(
                const ignition::math::Vector3d &/*_size*/)
    {
      // no op
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseParticleEmitter<T>::SetLifetime(const double &/*_lifetime*/)
    {
      // no op
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseParticleEmitter<T>::SetMaterial(const MaterialPtr &/*_material*/)
    {
      // no op
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseParticleEmitter<T>::SetVelocityRange(const double /*_minVel*/,
                                                  const double /*_maxVel*/)
    {
      // no op
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseParticleEmitter<T>::SetColorRange(
                  const ignition::math::Color &/*_colorStart*/,
                  const ignition::math::Color &/*_colorEnd*/)
    {
      // no op
    }
    }
  }
}
#endif
