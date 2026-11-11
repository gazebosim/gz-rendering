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
#ifndef GZ_RENDERING_FILAMENT_FILAMENTUTILS_HH_
#define GZ_RENDERING_FILAMENT_FILAMENTUTILS_HH_

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <unordered_map>

#include <filament/Engine.h>
#include <filament/Material.h>
#include <filament/MaterialInstance.h>
#include <math/vec3.h>
#include <math/vec4.h>

#include <gz/common/Console.hh>
#include <gz/math/Color.hh>

#include "gz/rendering/Scene.hh"
#include "gz/rendering/filament/FilamentMaterial.hh"
#include "gz/rendering/filament/FilamentRenderEngine.hh"

// Helpers shared by the filament engine's sources
namespace gz::rendering::filament_utils
{
  /// \brief Buffer descriptor callback freeing a new[] array of T once
  /// filament has uploaded it
  template <typename T>
  void FreeBuffer(void *_buf, size_t, void *)
  {
    delete[] static_cast<T *>(_buf);
  }

  /// \brief gz_flat instance drawing _color, created on first use and kept
  /// in _cache by its 8 bit color (0xRRGGBB). Null if the material is
  /// missing.
  inline filament::MaterialInstance *FlatInstance(
      std::unordered_map<uint32_t, filament::MaterialInstance *> &_cache,
      const math::Color &_color)
  {
    const uint32_t rgb =
        (static_cast<uint32_t>(std::lround(_color.R() * 255)) << 16) |
        (static_cast<uint32_t>(std::lround(_color.G() * 255)) << 8) |
        static_cast<uint32_t>(std::lround(_color.B() * 255));
    auto it = _cache.find(rgb);
    if (it != _cache.end())
      return it->second;
    filament::Material *mat =
        FilamentRenderEngine::Instance()->Material("gz_flat");
    if (!mat)
      return nullptr;
    filament::MaterialInstance *mi = mat->createInstance();
    mi->setParameter("color", filament::math::float3{
        _color.R(), _color.G(), _color.B()});
    _cache[rgb] = mi;
    return mi;
  }

  /// \brief Clear color of a packed float target that reads +inf.
  /// Float sensor outputs (depth, lidar ranges, depth noise) go through
  /// RGBA8 targets as the 4 bytes of their float bits, least significant in
  /// red (gz_depth.mat): filament's GLES shaders write their color at
  /// mediump, which drivers may round to half precision (Mesa's llvmpipe
  /// does), and every byte value survives that. Vulkan's readPixels also
  /// copies RGBA8 rows as they are, but converts R32F texel by texel.
  constexpr filament::math::float4 kPackedInf{
      0.0f, 0.0f, 128.0f / 255.0f, 127.0f / 255.0f};

  /// \brief The color whose 8 bit channels are _rgb (0xRRGGBB)
  inline math::Color ColorFromRgb(uint32_t _rgb)
  {
    return math::Color(((_rgb >> 16) & 0xff) / 255.0f,
        ((_rgb >> 8) & 0xff) / 255.0f, (_rgb & 0xff) / 255.0f);
  }

  /// \brief Destroy the instances of a FlatInstance cache and clear it
  inline void DestroyFlatInstances(
      std::unordered_map<uint32_t, filament::MaterialInstance *> &_cache)
  {
    auto *engine = FilamentRenderEngine::Instance()->FilamentEngine();
    if (engine)
    {
      for (auto &kv : _cache)
        engine->destroy(kv.second);
    }
    _cache.clear();
  }

  /// \brief The material SetMaterial(_material, _unique) of a visual or
  /// geometry assigns: a clone when _unique. Null (logged) when _material
  /// is null or belongs to another render engine.
  inline MaterialPtr MaterialToAssign(MaterialPtr _material, bool _unique)
  {
    if (!_material)
    {
      gzerr << "Cannot assign null material" << std::endl;
      return nullptr;
    }
    _material = _unique ? _material->Clone() : _material;
    if (!std::dynamic_pointer_cast<FilamentMaterial>(_material))
    {
      gzerr << "Cannot assign material created by another render-engine"
            << std::endl;
      return nullptr;
    }
    return _material;
  }

  /// \brief Store _material in _slot, owned (destroyed when replaced) if
  /// _own, and destroy the material _slot owned before. Call it once
  /// nothing renders with the old material any more: filament panics on
  /// destroying a material in use. A null _material releases the slot.
  inline void ReplaceMaterial(const ScenePtr &_scene, MaterialPtr &_slot,
      bool &_owned, MaterialPtr _material, bool _own)
  {
    MaterialPtr old = _owned ? _slot : nullptr;
    _slot = std::move(_material);
    _owned = _own;
    if (old && old != _slot && _scene)
      _scene->DestroyMaterial(old);
  }
}

#endif
