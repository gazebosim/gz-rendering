/*
 * Copyright (C) 2026 Open Source Robotics Foundation
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
#ifndef GZ_RENDERING_OGRE2_OGRE2SHADERPASS_HH_
#define GZ_RENDERING_OGRE2_OGRE2SHADERPASS_HH_

#include <memory>

#include "gz/rendering/base/BaseShaderPass.hh"
#include "gz/rendering/ogre2/Ogre2RenderPass.hh"
#include "gz/rendering/ogre2/Export.hh"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    //
    // forward declaration
    class Ogre2ShaderPassPrivate;

    /* \class Ogre2ShaderPass Ogre2ShaderPass.hh \
     * gz/rendering/ogre2/Ogre2ShaderPass.hh
     */
    /// \brief Ogre2 implementation of a render pass with the user's shaders.
    ///
    /// The pass is a compositor node with one quad pass, as the Gaussian
    /// noise pass. If a shader file cannot be read or loaded, the pass
    /// logs the error once and disables itself, and the camera renders
    /// without it. Ogre compiles GLSL when it is first drawn: a shader that
    /// does not compile throws from the first frame's render, as a
    /// material's shader does, and the pass is disabled from the next.
    class GZ_RENDERING_OGRE2_VISIBLE Ogre2ShaderPass :
      public BaseShaderPass<Ogre2RenderPass>
    {
      /// \brief Constructor
      public: Ogre2ShaderPass();

      /// \brief Destructor
      public: ~Ogre2ShaderPass() override;

      // Documentation inherited
      public: bool IsEnabled() const override;

      // Documentation inherited
      public: void PreRender(const CameraPtr &_camera) override;

      // Documentation inherited
      public: void CreateRenderPass() override;

      // Documentation inherited
      public: void Destroy() override;

      /// \brief Pointer to private data class
      private: std::unique_ptr<Ogre2ShaderPassPrivate> dataPtr;
    };
    }
  }
}
#endif
