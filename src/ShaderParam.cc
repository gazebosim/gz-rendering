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

  public: std::shared_ptr<float> wordBuffer;
  public: uint32_t count;
};


//////////////////////////////////////////////////
ShaderParam::ShaderParam() :
  dataPtr(new ShaderParamPrivate)
{
  this->dataPtr->count = 0;
  this->dataPtr->wordBuffer.reset();
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
  this->dataPtr->wordBuffer.reset();
}

//////////////////////////////////////////////////
ShaderParam::ParamType ShaderParam::Type() const
{
  return this->dataPtr->type;
}

//////////////////////////////////////////////////
uint32_t ShaderParam::Count() const
{
  return this->dataPtr->count;
}

//////////////////////////////////////////////////
ShaderParam &ShaderParam::operator=(const ShaderParam &_other)
{
  *(this->dataPtr) = *(_other.dataPtr);
  return *this;
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
void ShaderParam::InitializeWordBuffer(const uint32_t _count)
{
  this->dataPtr->type = PARAM_WORD_BUFFER;
  this->dataPtr->count = _count;
  this->dataPtr->wordBuffer.reset(new float[_count], std::default_delete<float[]>());
}

//////////////////////////////////////////////////
void ShaderParam::UpdateWordBuffer(float* _wordBuffer, const uint32_t _count)
{
  memcpy(this->dataPtr->wordBuffer.get(), _wordBuffer, sizeof(float) * _count);
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

//////////////////////////////////////////////////
std::shared_ptr<float> ShaderParam::WordBuffer() const
{
  if (this->dataPtr->wordBuffer)
  {
    return this->dataPtr->wordBuffer;
  }
  return nullptr;
}
