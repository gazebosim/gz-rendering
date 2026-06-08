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
#ifndef GZ_RENDERING_OGRE_NEXT_OGRE_NEXTGAUSSIANNOISEPASS_HH_
#define GZ_RENDERING_OGRE_NEXT_OGRE_NEXTGAUSSIANNOISEPASS_HH_

#include <memory>

#include "gz/rendering/base/BaseGaussianNoisePass.hh"
#include "gz/rendering/ogre_next/OgreNextRenderPass.hh"
#include "gz/rendering/ogre_next/Export.hh"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    //
    // forward declaration
    class OgreNextGaussianNoisePassPrivate;

    /* \class OgreNextGaussianNoisePass OgreNextGaussianNoisePass.hh \
     * gz/rendering/ogre_next/OgreNextGaussianNoisePass.hh
     */
    /// \brief OgreNext Implementation of a Gaussian noise render pass.
    class GZ_RENDERING_OGRE_NEXT_VISIBLE OgreNextGaussianNoisePass :
      public BaseGaussianNoisePass<OgreNextRenderPass>
    {
      /// \brief Constructor
      public: OgreNextGaussianNoisePass();

      /// \brief Destructor
      public: virtual ~OgreNextGaussianNoisePass();

      // Documentation inherited
      public: virtual void PreRender(const CameraPtr &_camera) override;

      // Documentation inherited
      public: void CreateRenderPass() override;

      /// \brief Pointer to private data class
      private: std::unique_ptr<OgreNextGaussianNoisePassPrivate> dataPtr;
    };
    }
  }
}
#endif
