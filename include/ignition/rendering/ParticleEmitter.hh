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
#ifndef IGNITION_RENDERING_PARTICLEEMITTER_HH_
#define IGNITION_RENDERING_PARTICLEEMITTER_HH_

#include "ignition/math/Pose3.hh"
#include "ignition/math/Vector3.hh"
#include "ignition/rendering/config.hh"
#include "ignition/rendering/Export.hh"
#include "ignition/rendering/RenderTypes.hh"
#include "ignition/rendering/Visual.hh"

namespace ignition
{
  namespace rendering
  {
    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
    //
    /// \brief Enum for emitter types.
    enum IGNITION_RENDERING_VISIBLE EmitterType
    {
      /// \brief Point emitter.
      EM_POINT            = 0,

      /// \brief Box emitter.
      EM_BOX              = 1,

      /// \brief Cylinder emitter.
      EM_CYLINDER         = 2,

      /// \brief Ellipsoid emitter.
      EM_ELLISOID         = 3,
    };

    /// \class ParticleEmitter ParticleEmitter.hh
    /// ignition/rendering/ParticleEmitter.hh
    //
    /// \brief Class to manage a particle emitter.
    class IGNITION_RENDERING_VISIBLE ParticleEmitter :
      public virtual Visual
    {
      /// \brief Destructor
      public: virtual ~ParticleEmitter();

      /// \brief \brief Set the emitter type.
      /// Default value is EM_POINT.
      /// \param[in] _type Emitter type.
      /// \sa EmitterType.
      public: virtual void SetType(const EmitterType _type) = 0;

      /// \brief Sets the size of the emitter where the particles are sampled.
      /// Default value is (1, 1, 1).
      /// Note that the interpretation of the emitter area varies depending on
      /// the emmiter type:
      ///   - EM_POINT: The area is ignored.
      ///   - EM_BOX: The area is interpreted as width X height X depth.
      ///   - EM_CYLINDER: The area is interpreted as the bounding box of the
      ///                  cilinder. The cylinder is oriented along the Z-axis.
      ///   - EM_ELLIPSOID: The area is interpreted as the bounding box of an
      ///                   ellipsoid shaped area, i.e. a sphere or
      ///                   squashed-sphere area. The parameters are again
      ///                   identical to EM_BOX, except that the dimensions
      ///                   describe the widest points along each of the axes.
      /// \param[in] _size Size of the emitter (width, height, depth).
      public: virtual void SetEmitterSize(
                  const ignition::math::Vector3d &_size) = 0;

      /// \brief Sets how many particles per second should be emitted.
      /// Default value is 10.
      /// \param[in] _rate Particles per second.
      public: virtual void SetRate(const double _rate) = 0;

      /// \brief Sets the position of the emitter relative to the SceneNode the
      /// particle system is attached to.
      /// Default value is (0, 0, 0, 0, 0, 0).
      /// \param[in] _pose Position of the emitter.
      public: virtual void SetLocalPose(
                  const ignition::math::Pose3d &_pose) = 0;

      /// \brief Sets the number of seconds the emitter is active.
      /// A value of 0 means infinite duration.
      /// Default value is 0.
      /// \param[in] _duration Total duration of the emitter (seconds).
      public: virtual void SetDuration(const double &_duration) = 0;

      /// \brief This is used to turn on or off particle emission.
      /// \param[in] _enable True for enabling the emission or false otherwise.
      public: virtual void SetEmitting(const bool _enable) = 0;

      /// \brief Sets the particle dimensions (width, height, depth).
      /// \param[in] _size Particle dimensions.
      public: virtual void SetParticleSize(
                  const ignition::math::Vector3d &_size) = 0;

      /// \brief Sets the number of seconds each particle will ’live’ for before
      /// being destroyed.
      /// Default value is 5.
      /// \param[in] _timeToLive Lifetime of each particle (seconds).
      public: virtual void SetLifetime(const double &_lifetime) = 0;

      /// \brief Sets the material which all particles in the emitter will use.
      /// \param[in] _material The material pointer.
      public: virtual void SetMaterial(const MaterialPtr &_material) = 0;

      /// \brief Set a velocity range and each particle is emitted with a
      /// random velocity within this range.
      /// Default value is 1 for both velocities.
      /// \param[in] _minVel Minimum velocity (m/s).
      /// \param[in] _maxVel Maximum velocity (m/s).
      public: virtual void SetVelocityRange(const double _minVel,
                                            const double _maxVel) = 0;

      /// \brief Sets a colour for all particle emitted.
      /// The actual colour will be randomly chosen between these two values.
      /// Default values are Color::Black and Color::White respectively.
      /// \param[in] _colorStart Start color.
      /// \param[in] _colorEnd End color.
      public: virtual void SetColorRange(
                  const ignition::math::Color &_colorStart,
                  const ignition::math::Color &_colorEnd) = 0;
    };
    }
  }
}
#endif
