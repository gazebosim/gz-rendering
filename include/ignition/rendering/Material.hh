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
#ifndef _IGNITION_RENDERING_MATERIAL_HH_
#define _IGNITION_RENDERING_MATERIAL_HH_

#include <string>
#include "gazebo/common/Color.hh"
#include "gazebo/common/Material.hh"
#include "ignition/rendering/RenderTypes.hh"
#include "ignition/rendering/Object.hh"
#include "ignition/rendering/ShaderType.hh"
#include "ignition/rendering/Util.hh"

namespace ignition
{
  namespace rendering
  {
    class IGNITION_VISIBLE Material :
      public virtual Object
    {
      public: virtual ~Material() { }

      public: virtual bool GetLightingEnabled() const = 0;

      public: virtual void SetLightingEnabled(bool _enabled) = 0;

      public: virtual gazebo::common::Color GetAmbient() const = 0;

      public: virtual void SetAmbient(double _r, double _g, double _b,
                  double _a = 1.0) = 0;

      public: virtual void SetAmbient(const gazebo::common::Color &_color) = 0;

      public: virtual gazebo::common::Color GetDiffuse() const = 0;

      public: virtual void SetDiffuse(double _r, double _g, double _b,
                  double _a = 1.0) = 0;

      public: virtual void SetDiffuse(const gazebo::common::Color &_color) = 0;

      public: virtual gazebo::common::Color GetSpecular() const = 0;

      public: virtual void SetSpecular(double _r, double _g, double _b,
                  double _a = 1.0) = 0;

      public: virtual void SetSpecular(const gazebo::common::Color &_color) = 0;

      public: virtual gazebo::common::Color GetEmissive() const = 0;

      public: virtual void SetEmissive(double _r, double _g, double _b,
                  double _a = 1.0) = 0;

      public: virtual void SetEmissive(const gazebo::common::Color &_color) = 0;

      public: virtual double GetShininess() const = 0;

      public: virtual void SetShininess(double _shininess) = 0;

      public: virtual double GetTransparency() const = 0;

      public: virtual void SetTransparency(double _transparency) = 0;

      public: virtual double GetReflectivity() const = 0;

      public: virtual void SetReflectivity(double _reflectivity) = 0;

      public: virtual bool GetCastShadows() const = 0;

      public: virtual void SetCastShadows(bool _castShadows) = 0;

      public: virtual bool GetReceiveShadows() const = 0;

      public: virtual void SetReceiveShadows(bool _receiveShadows) = 0;

      public: virtual bool GetReflectionEnabled() const = 0;

      public: virtual void SetReflectionEnabled(bool _enabled) = 0;

      public: virtual bool HasTexture() const = 0;

      public: virtual std::string GetTexture() const = 0;

      public: virtual void SetTexture(const std::string &_name) = 0;

      public: virtual void ClearTexture() = 0;

      public: virtual bool HasNormalMap() const = 0;

      public: virtual std::string GetNormalMap() const = 0;

      public: virtual void SetNormalMap(const std::string &_name) = 0;

      public: virtual void ClearNormalMap() = 0;

      public: virtual ShaderType GetShaderType() const = 0;

      public: virtual void SetShaderType(ShaderType _type) = 0;

      public: virtual MaterialPtr Clone() const = 0;

      public: virtual void CopyFrom(ConstMaterialPtr _material) = 0;

      public: virtual void CopyFrom(const gazebo::common::Material &_material) = 0;
    };
  }
}
#endif
