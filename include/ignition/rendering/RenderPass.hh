/*
 * Copyright (C) 2019 Open Source Robotics Foundation
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
#ifndef IGNITION_RENDERING_RENDERPASS_HH_
#define IGNITION_RENDERING_RENDERPASS_HH_

#include "ignition/rendering/config.hh"
#include "ignition/rendering/Object.hh"
#include "ignition/rendering/Export.hh"

namespace ignition
{
  namespace rendering
  {
    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
    //
    /// \class RenderPass RenderPass.hh ignition/rendering/RenderPass.hh
    /// \brief A render pass can be added to a camera to affect how the scene
    /// is rendered. It can be used to add post-processing effects. Multiple
    /// passes can be chained together.
    class IGNITION_RENDERING_VISIBLE RenderPass
      : public virtual Object
    {
      /// \brief Destructor
      public: virtual ~RenderPass() { }

    };
    }
  }
}
#endif
