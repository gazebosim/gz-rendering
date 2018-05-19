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

#include "ignition/rendering/ShaderParam.hh"

using namespace ignition::rendering;


class ignition::rendering::ShaderParamPrivate
{
  /// \brief Type of parameter held
  public: ShaderParam::ParamType type = ShaderParam::PARAM_NONE;

  /// \brief Union for the type held in this param
  public: union
    {
      float vFloat;
      int vInt;
    } paramValue;
};


//////////////////////////////////////////////////
ShaderParam::ShaderParam() :
  dataPtr(new ShaderParamPrivate)
{
}

//////////////////////////////////////////////////
ShaderParam::ShaderParam(const ShaderParam &_other)
  : dataPtr(new ShaderParamPrivate)
{
  // Avoid incorrect cppcheck error about dataPtr being assigned in constructor
  ShaderParamPrivate &dp = *(this->dataPtr);
  dp = *(_other.dataPtr);
}

//////////////////////////////////////////////////
ShaderParam::~ShaderParam()
{
}

//////////////////////////////////////////////////
ShaderParam::ParamType ShaderParam::Type() const
{
  return this->dataPtr->type;
}

//////////////////////////////////////////////////
void ShaderParam::operator=(const float _value)
{
  this->dataPtr->type = PARAM_FLOAT;
  this->dataPtr->paramValue.vFloat = _value;
}

//////////////////////////////////////////////////
void ShaderParam::operator=(const int _value)
{
  this->dataPtr->type = PARAM_INT;
  this->dataPtr->paramValue.vInt = _value;
}

//////////////////////////////////////////////////
bool ShaderParam::Value(float *_value) const
{
  if (PARAM_FLOAT == this->dataPtr->type)
  {
    *_value = this->dataPtr->paramValue.vFloat;
    return true;
  }
  return false;
}

//////////////////////////////////////////////////
bool ShaderParam::Value(int *_value) const
{
  if (PARAM_INT == this->dataPtr->type)
  {
    *_value = this->dataPtr->paramValue.vInt;
    return true;
  }
  return false;
}
