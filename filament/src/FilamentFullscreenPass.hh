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
#ifndef GZ_RENDERING_FILAMENT_FILAMENTFULLSCREENPASS_HH_
#define GZ_RENDERING_FILAMENT_FILAMENTFULLSCREENPASS_HH_

#include <cstdint>

#include <filament/Engine.h>
#include <filament/IndexBuffer.h>
#include <filament/Material.h>
#include <filament/MaterialInstance.h>
#include <filament/RenderableManager.h>
#include <filament/Scene.h>
#include <filament/VertexBuffer.h>
#include <filament/View.h>
#include <math/vec4.h>
#include <utils/EntityManager.h>

#include "gz/rendering/config.hh"
#include "gz/rendering/filament/FilamentRenderEngine.hh"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    //
    /// \brief A material drawn over a whole render target: one triangle
    /// covering clip space (device-domain vertices) in a scene of its own,
    /// rendered through `view`. Used by the sensor second passes and the
    /// render passes.
    class FilamentFullscreenPass
    {
      public: explicit FilamentFullscreenPass(filament::Material &_material)
      {
        auto *engine = FilamentRenderEngine::Instance()->FilamentEngine();
        this->instance = _material.createInstance();
        this->scene = engine->createScene();
        this->view = engine->createView();
        this->view->setScene(this->scene);
        this->view->setPostProcessingEnabled(false);
        this->view->setShadowingEnabled(false);
        // a view needs a camera; device-domain vertices ignore it
        this->cameraEntity = ::utils::EntityManager::get().create();
        this->view->setCamera(engine->createCamera(this->cameraEntity));

        static const filament::math::float4 kVerts[3] = {
            {-1.0f, -1.0f, 0.5f, 1.0f}, {3.0f, -1.0f, 0.5f, 1.0f},
            {-1.0f, 3.0f, 0.5f, 1.0f}};
        static const uint16_t kIndices[3] = {0, 1, 2};
        this->vb = filament::VertexBuffer::Builder()
            .vertexCount(3).bufferCount(1)
            .attribute(filament::VertexAttribute::POSITION, 0,
                filament::VertexBuffer::AttributeType::FLOAT4)
            .build(*engine);
        this->vb->setBufferAt(*engine, 0,
            filament::VertexBuffer::BufferDescriptor(kVerts, sizeof(kVerts)));
        this->ib = filament::IndexBuffer::Builder()
            .indexCount(3).bufferType(filament::IndexBuffer::IndexType::USHORT)
            .build(*engine);
        this->ib->setBuffer(*engine,
            filament::IndexBuffer::BufferDescriptor(kIndices,
            sizeof(kIndices)));
        this->triangle = ::utils::EntityManager::get().create();
        filament::RenderableManager::Builder(1)
            .boundingBox({{-1.0f, -1.0f, -1.0f}, {1.0f, 1.0f, 1.0f}})
            .culling(false)
            .castShadows(false)
            .receiveShadows(false)
            .material(0, this->instance)
            .geometry(0, filament::RenderableManager::PrimitiveType::TRIANGLES,
                this->vb, this->ib)
            .build(*engine, this->triangle);
        this->scene->addEntity(this->triangle);
      }

      public: ~FilamentFullscreenPass()
      {
        auto *engine = FilamentRenderEngine::Instance()->FilamentEngine();
        if (!engine)
          return;
        engine->destroy(this->view);
        // the renderable goes before its material instance
        engine->getRenderableManager().destroy(this->triangle);
        ::utils::EntityManager::get().destroy(this->triangle);
        engine->destroy(this->instance);
        engine->destroy(this->vb);
        engine->destroy(this->ib);
        engine->destroy(this->scene);
        engine->destroyCameraComponent(this->cameraEntity);
        ::utils::EntityManager::get().destroy(this->cameraEntity);
      }

      public: FilamentFullscreenPass(const FilamentFullscreenPass &) = delete;

      public: FilamentFullscreenPass &operator=(
          const FilamentFullscreenPass &) = delete;

      /// \brief Instance of the material, for its parameters
      public: filament::MaterialInstance *instance{nullptr};

      /// \brief View to render the pass with
      public: filament::View *view{nullptr};

      private: filament::Scene *scene{nullptr};

      private: ::utils::Entity cameraEntity;

      private: ::utils::Entity triangle;

      private: filament::VertexBuffer *vb{nullptr};

      private: filament::IndexBuffer *ib{nullptr};
    };
    }
  }
}
#endif
