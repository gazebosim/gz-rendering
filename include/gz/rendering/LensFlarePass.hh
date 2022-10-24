/*
 * Copyright (C) 2022 Open Source Robotics Foundation
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
#ifndef GZ_RENDERING_LENSFLAREPASS_HH_
#define GZ_RENDERING_LENSFLAREPASS_HH_

#include <string>
#include "gz/rendering/config.hh"
#include "gz/rendering/Export.hh"
#include "gz/rendering/RenderPass.hh"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {

    /// \brief A render pass that applies Lens Flare to the render target
    class GZ_RENDERING_VISIBLE LensFlarePass
      : public virtual RenderPass
    {
      /// \brief Initializes the Lens Flare Pass with given scene
      /// \param[in] _scene Pointer to scene
      public: virtual void Init(ScenePtr _scene) = 0;

      /// \brief Set the light that generates lens flare
      /// \param[in] _light Pointer to light
      public: virtual void SetLight(LightPtr _light) = 0;
    };
    }
  }
}
#endif
