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
    /// \class Material Material.hh ignition/rendering/Material.hh
    /// \brief Represents a surface material of a Geometry
    class IGNITION_VISIBLE Material :
      public virtual Object
    {
      /// \brief Deconstructor
      public: virtual ~Material() { }

      /// \brief Determine if lighting affects this material
      /// \return True if lighting affects this material
      public: virtual bool GetLightingEnabled() const = 0;

      /// \brief Specify if lighting affects this material
      /// \param[in] _enabled True if lighting affects this material
      public: virtual void SetLightingEnabled(bool _enabled) = 0;

      /// \brief Get the ambient color
      /// \return The ambient color
      public: virtual gazebo::common::Color GetAmbient() const = 0;

      /// \brief Set the ambient color
      /// \param[in] _r Red value
      /// \param[in] _g Green value
      /// \param[in] _b Blue value
      /// \param[in] _a Alpha value
      public: virtual void SetAmbient(double _r, double _g, double _b,
                  double _a = 1.0) = 0;

      /// \brief Set the ambient color
      /// \param[in] _color New ambient color
      public: virtual void SetAmbient(const gazebo::common::Color &_color) = 0;

      /// \brief Get the diffuse color
      /// \return The diffuse color
      public: virtual gazebo::common::Color GetDiffuse() const = 0;

      /// \brief Set the diffuse color
      /// \param[in] _r Red value
      /// \param[in] _g Green value
      /// \param[in] _b Blue value
      /// \param[in] _a Alpha value
      public: virtual void SetDiffuse(double _r, double _g, double _b,
                  double _a = 1.0) = 0;

      /// \brief Set the diffuse color
      /// \param[in] _color New diffuse color
      public: virtual void SetDiffuse(const gazebo::common::Color &_color) = 0;

      /// \brief Get the specular color
      /// \return The specular color
      public: virtual gazebo::common::Color GetSpecular() const = 0;

      /// \brief Set the specular color
      /// \param[in] _r Red value
      /// \param[in] _g Green value
      /// \param[in] _b Blue value
      /// \param[in] _a Alpha value
      public: virtual void SetSpecular(double _r, double _g, double _b,
                  double _a = 1.0) = 0;

      /// \brief Set the specular color
      /// \param[in] _color New specular color
      public: virtual void SetSpecular(const gazebo::common::Color &_color) = 0;

      /// \brief Get the emissive color
      /// \return The emissive color
      public: virtual gazebo::common::Color GetEmissive() const = 0;

      /// \brief Set the emissive color
      /// \param[in] _r Red value
      /// \param[in] _g Green value
      /// \param[in] _b Blue value
      /// \param[in] _a Alpha value
      public: virtual void SetEmissive(double _r, double _g, double _b,
                  double _a = 1.0) = 0;

      /// \brief Set the emissive color
      /// \param[in] _color New emissive color
      public: virtual void SetEmissive(const gazebo::common::Color &_color) = 0;

      /// \brief Get the shininess value
      /// \return The shininess value
      public: virtual double GetShininess() const = 0;

      /// \brief Set the shininess value
      /// \param[in] _shininess New shininess value
      public: virtual void SetShininess(double _shininess) = 0;

      /// \brief Get the transparency value
      /// \return The transparency value
      public: virtual double GetTransparency() const = 0;

      /// \brief Set the transparency value
      /// \param[in] _transparency New transparency value
      public: virtual void SetTransparency(double _transparency) = 0;

      /// \brief Get the reflectivity value
      /// \return The reflectivity value
      public: virtual double GetReflectivity() const = 0;

      /// \brief Set the reflectivity value
      /// \param[in] _reflectivity New reflectivity value
      public: virtual void SetReflectivity(double _reflectivity) = 0;

      /// \brief Determine if this material casts shadows
      /// \return True if this material casts shadows
      public: virtual bool GetCastShadows() const = 0;

      /// \brief Specify if this material casts shadows
      /// \param[in] _castShadows True if this material casts shadows
      public: virtual void SetCastShadows(bool _castShadows) = 0;

      /// \brief Determine if this material receives shadows
      /// \return True if this material receives shadows
      public: virtual bool GetReceiveShadows() const = 0;

      /// \brief Specify if this material receives shadows
      /// \param[in] _receiveShadows True if this material receives shadows
      public: virtual void SetReceiveShadows(bool _receiveShadows) = 0;

      /// \brief Determine if this material has a reflection
      /// \return True if this material has a reflection
      public: virtual bool GetReflectionEnabled() const = 0;

      /// \brief Specify if this material has a reflection
      /// \param[in] _enabled True if this material has a reflection
      public: virtual void SetReflectionEnabled(bool _enabled) = 0;

      /// \brief Determine if this material has a texture
      /// \return True if this material has a texture
      public: virtual bool HasTexture() const = 0;

      /// \brief Get the URI of the texture file
      /// \return URI of the texture file
      public: virtual std::string GetTexture() const = 0;

      /// \brief Set the material texture
      /// \param[in] _name URI of the new texture file
      public: virtual void SetTexture(const std::string &_name) = 0;

      /// \brief Removes any texture mapped to this material
      public: virtual void ClearTexture() = 0;

      /// \brief Determine if this material has a normal map
      /// \return True if this material has a normal map
      public: virtual bool HasNormalMap() const = 0;

      /// \brief Get the URI of the normal map file
      /// \return URI of the normal map file
      public: virtual std::string GetNormalMap() const = 0;

      /// \brief Set the material normal map
      /// \param[in] _name URI of the new normal map file
      public: virtual void SetNormalMap(const std::string &_name) = 0;

      /// \brief Removes any normal map mapped to this material
      public: virtual void ClearNormalMap() = 0;

      /// \brief Get the ShaderType value
      /// \return The ShaderType value
      public: virtual ShaderType GetShaderType() const = 0;

      /// \brief Set the ShaderType value
      /// \param[in] _type New ShaderType value
      public: virtual void SetShaderType(ShaderType _type) = 0;

      /// \brief Clone this material
      /// \return New cloned material
      public: virtual MaterialPtr Clone() const = 0;

      /// \brief Copy properties from given Material
      /// \param[in] _material Source Material to be copied from
      public: virtual void CopyFrom(ConstMaterialPtr _material) = 0;

      /// \brief Copy properties from given Material
      /// \param[in] _material Source Material to be copied from
      public: virtual void CopyFrom(const gazebo::common::Material &_material) = 0;
    };
  }
}
#endif
