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
#ifndef GZ_RENDERING_SHADERPASS_HH_
#define GZ_RENDERING_SHADERPASS_HH_

#include <functional>
#include <string>

#include "gz/rendering/config.hh"
#include "gz/rendering/Export.hh"
#include "gz/rendering/PixelFormat.hh"
#include "gz/rendering/RenderPass.hh"
#include "gz/rendering/RenderTypes.hh"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    //
    /* \class ShaderPass ShaderPass.hh gz/rendering/ShaderPass.hh
     */
    /// \brief A render pass that runs shaders of the user's on the image
    /// the passes before it left, as a full screen quad.
    ///
    /// The fragment shader reads two textures:
    ///  * `inputTexture`: the image so far;
    ///  * `dataTexture`: a texture the user sets from memory with
    ///    SetTexture, such as a map of what to draw over the image. A 1x1
    ///    texture of zeros until it is set.
    /// The vertex shader's interpolated texture coordinates are `uv0`, and
    /// it gets the `worldViewProj` matrix. Without a vertex shader of the
    /// user's, gz-rendering's own full screen one is used, which passes
    /// `uv0` to the fragment shader in a block named `inPs`.
    ///
    /// The shaders are GLSL for OpenGL. Uniforms of the fragment shader are
    /// set with FragmentShaderParams, as a material's; a callback set with
    /// SetPreRenderCallback is called right before each frame of a camera
    /// the pass is on, on the render thread, to update them, or the
    /// texture, for that frame.
    ///
    /// A shader file that cannot be read disables the pass, with an error.
    /// With ogre2, a shader that does not compile throws from the first
    /// frame's render, as a material's shader does, and the pass is
    /// disabled from the next frame.
    ///
    /// Supported by the ogre2 render engine.
    class GZ_RENDERING_VISIBLE ShaderPass
      : public virtual RenderPass
    {
      /// \brief A function called right before a camera renders a frame
      /// with this pass.
      public: using PreRenderCallback =
          std::function<void(const CameraPtr &_camera)>;

      /// \brief Constructor
      public: ShaderPass();

      /// \brief Destructor
      public: virtual ~ShaderPass();

      /// \brief Set the shaders. Call before the pass is added to a camera.
      /// \param[in] _vertexPath Path of a GLSL vertex shader, or empty for
      /// gz-rendering's full screen one.
      /// \param[in] _fragmentPath Path of a GLSL fragment shader.
      public: virtual void SetShaders(const std::string &_vertexPath,
                                      const std::string &_fragmentPath) = 0;

      /// \brief The vertex shader's path.
      /// \return The path, empty for gz-rendering's full screen one.
      public: virtual std::string VertexShader() const = 0;

      /// \brief The fragment shader's path.
      /// \return The path.
      public: virtual std::string FragmentShader() const = 0;

      /// \brief The uniforms of the fragment shader, set before each
      /// frame. Float, int and buffer parameters are supported; a name the
      /// shader does not have is warned about once.
      /// \return The parameters.
      public: virtual ShaderParamsPtr FragmentShaderParams() = 0;

      /// \brief Set the data texture from memory. The data is copied, and
      /// the texture is uploaded before the next frame.
      /// \param[in] _data Rows of texels, top first, tightly packed.
      /// \param[in] _width Texels across.
      /// \param[in] _height Texels down.
      /// \param[in] _format One of PF_L8, PF_R8G8B8A8, PF_FLOAT32_R and
      /// PF_FLOAT32_RGBA.
      /// \return False, and nothing changed, for a null pointer, a zero
      /// size or another format.
      public: virtual bool SetTexture(const void *_data, unsigned int _width,
          unsigned int _height, PixelFormat _format) = 0;

      /// \brief Set a function called right before each frame a camera
      /// renders with this pass, on the render thread, whether the pass is
      /// enabled or not: what it changes (the parameters, the texture) is
      /// used for that frame. Enabling or disabling the pass from it takes
      /// effect from the next frame.
      /// \param[in] _callback The function, or an empty one for none.
      public: virtual void SetPreRenderCallback(
          PreRenderCallback _callback) = 0;
    };
    }
  }
}
#endif
