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
#ifndef GZ_RENDERING_BASE_BASESHADERPASS_HH_
#define GZ_RENDERING_BASE_BASESHADERPASS_HH_

#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "gz/rendering/PixelFormat.hh"
#include "gz/rendering/ShaderParams.hh"
#include "gz/rendering/ShaderPass.hh"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    //
    /* \class BaseShaderPass BaseShaderPass.hh \
     * gz/rendering/base/BaseShaderPass.hh
     */
    /// \brief Base shader render pass: what the user sets, kept for the
    /// engine's pass to use.
    template <class T>
    class BaseShaderPass :
      public virtual ShaderPass,
      public virtual T
    {
      /// \brief Constructor
      protected: BaseShaderPass();

      /// \brief Destructor
      public: ~BaseShaderPass() override;

      // Documentation inherited.
      public: void SetShaders(const std::string &_vertexPath,
                              const std::string &_fragmentPath) override;

      // Documentation inherited.
      public: std::string VertexShader() const override;

      // Documentation inherited.
      public: std::string FragmentShader() const override;

      // Documentation inherited.
      public: ShaderParamsPtr FragmentShaderParams() override;

      // Documentation inherited.
      public: bool SetTexture(const void *_data, unsigned int _width,
          unsigned int _height, PixelFormat _format) override;

      // Documentation inherited.
      public: void SetPreRenderCallback(PreRenderCallback _callback)
          override;

      /// \brief Path of the vertex shader, empty for the full screen one.
      protected: std::string vertexPath;

      /// \brief Path of the fragment shader.
      protected: std::string fragmentPath;

      /// \brief The fragment shader's uniforms.
      protected: ShaderParamsPtr fragmentParams;

      /// \brief The data texture's texels, as SetTexture got them.
      protected: std::vector<std::uint8_t> textureData;

      /// \brief Texels across the data texture.
      protected: unsigned int textureWidth = 0u;

      /// \brief Texels down the data texture.
      protected: unsigned int textureHeight = 0u;

      /// \brief The data texture's format.
      protected: PixelFormat textureFormat = PF_UNKNOWN;

      /// \brief Whether the data texture changed since it was uploaded.
      protected: bool textureDirty = false;

      /// \brief Called right before each frame.
      protected: PreRenderCallback preRenderCallback;
    };

    //////////////////////////////////////////////////
    // BaseShaderPass
    //////////////////////////////////////////////////
    template <class T>
    BaseShaderPass<T>::BaseShaderPass()
      : fragmentParams(std::make_shared<ShaderParams>())
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    BaseShaderPass<T>::~BaseShaderPass()
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseShaderPass<T>::SetShaders(const std::string &_vertexPath,
        const std::string &_fragmentPath)
    {
      this->vertexPath = _vertexPath;
      this->fragmentPath = _fragmentPath;
    }

    //////////////////////////////////////////////////
    template <class T>
    std::string BaseShaderPass<T>::VertexShader() const
    {
      return this->vertexPath;
    }

    //////////////////////////////////////////////////
    template <class T>
    std::string BaseShaderPass<T>::FragmentShader() const
    {
      return this->fragmentPath;
    }

    //////////////////////////////////////////////////
    template <class T>
    ShaderParamsPtr BaseShaderPass<T>::FragmentShaderParams()
    {
      return this->fragmentParams;
    }

    //////////////////////////////////////////////////
    template <class T>
    bool BaseShaderPass<T>::SetTexture(const void *_data,
        unsigned int _width, unsigned int _height, PixelFormat _format)
    {
      if (!_data || _width == 0u || _height == 0u)
        return false;
      if (_format != PF_L8 && _format != PF_R8G8B8A8 &&
          _format != PF_FLOAT32_R && _format != PF_FLOAT32_RGBA)
      {
        return false;
      }
      const std::size_t size = static_cast<std::size_t>(_width) * _height *
          PixelUtil::BytesPerPixel(_format);
      this->textureData.resize(size);
      std::memcpy(this->textureData.data(), _data, size);
      this->textureWidth = _width;
      this->textureHeight = _height;
      this->textureFormat = _format;
      this->textureDirty = true;
      return true;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseShaderPass<T>::SetPreRenderCallback(PreRenderCallback _callback)
    {
      this->preRenderCallback = std::move(_callback);
    }
    }
  }
}
#endif
