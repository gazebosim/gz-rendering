/*
 * Copyright (C) 2015 Open Source Robotics Foundation
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
#ifndef _IGNITION_RENDERING_SHADERTYPE_HH_
#define _IGNITION_RENDERING_SHADERTYPE_HH_

#include <string>
#include "ignition/rendering/Util.hh"

namespace ignition
{
  namespace rendering
  {
    enum IGNITION_VISIBLE ShaderType
    {
      ST_UNKNOWN  = 0,
      ST_PIXEL    = 1,
      ST_VERTEX   = 2,
      ST_NORM_OBJ = 3,
      ST_NORM_TAN = 4,
      ST_COUNT    = 5,
    };

    class IGNITION_VISIBLE ShaderUtil
    {
      public: static bool IsValid(ShaderType _type);

      public: static ShaderType Sanitize(ShaderType _type);

      public: static std::string GetName(ShaderType _type);

      public: static ShaderType GetEnum(const std::string &_name);

      private: static const char *names[ST_COUNT];
    };
  }
}
#endif
