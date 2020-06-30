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
    /// \class ParticleEmitter ParticleEmitter.hh
    /// ignition/rendering/ParticleEmitter.hh
    //
    /// \brief Class to manage a particle emitter.
    class IGNITION_RENDERING_VISIBLE ParticleEmitter :
      public virtual Visual
    {
      /// \brief Destructor
      public: virtual ~ParticleEmitter() { }
    };
    }
  }
}
#endif
