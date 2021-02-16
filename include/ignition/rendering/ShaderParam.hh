/*
 * Copyright (C) 2017 Open Source Robotics Foundation
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

#ifndef IGNITION_RENDERING_SHADERPARAM_HH_
#define IGNITION_RENDERING_SHADERPARAM_HH_

#include <cstdint>
#include <memory>

#include <ignition/common/SuppressWarning.hh>

#include "ignition/rendering/config.hh"
#include "ignition/rendering/Export.hh"

namespace ignition
{
  namespace rendering
  {
    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
    //
    /// \brief forward declaration
    class ShaderParamPrivate;

    /// \brief a variant type that holds params that can be passed to a shader
    class IGNITION_RENDERING_VISIBLE ShaderParam
    {
      public: enum ParamType : uint16_t
      {
        /// \brief Type none
        PARAM_NONE = 0,

        /// \brief  Float type param
        PARAM_FLOAT = 1,

        /// \brief Integer type param
        PARAM_INT = 2,
      };

      /// \brief constructor
      public: ShaderParam();

      /// \brief copy constructor
      /// \param[in] _other Another ShaderParam
      public: ShaderParam(const ShaderParam &_other);

      /// \brief destructor
      public: ~ShaderParam();

      /// \brief Get the type of this parameter
      /// \return Type of this parameter
      public: ParamType Type() const;

      /// \brief Set from another ShaderParam.
      /// \param[in] _other Another ShaderParam.
      /// \return Reference to this ShaderParam.
      public: ShaderParam &operator=(const ShaderParam &_other);

      /// \brief Set this to be a float param
      /// \param[in] _value Value to set this param to.
      public: void operator=(const float _value);

      /// \brief Set this to be an integer param;
      /// \param[in] _value Value to set this param to.
      public: void operator=(const int _value);

      /// \brief Get the value of this parameter if it is a float
      /// \param[out] _value variable the value will be copied to
      /// \return true if the param is the expected type
      public: bool Value(float *_value) const;

      /// \brief Get the value of this parameter if it is an int
      /// \param[out] _value variable the value will be copied to
      /// \return true if the param is the expected type
      public: bool Value(int *_value) const;

      /// \brief private implementation
      IGN_COMMON_WARN_IGNORE__DLL_INTERFACE_MISSING
      private: std::unique_ptr<ShaderParamPrivate> dataPtr;
      IGN_COMMON_WARN_RESUME__DLL_INTERFACE_MISSING
    };
    }
  }
}

#endif
