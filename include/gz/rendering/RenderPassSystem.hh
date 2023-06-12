/*
 * Copyright (C) 2019 Open Source Robotics Foundation
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
#ifndef GZ_RENDERING_RENDERPASSSYSTEM_HH_
#define GZ_RENDERING_RENDERPASSSYSTEM_HH_

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <typeinfo>

#include <gz/utils/SuppressWarning.hh>

#include "gz/rendering/config.hh"
#include "gz/rendering/Export.hh"
#include "gz/rendering/RenderPass.hh"
#include "gz/rendering/RenderTypes.hh"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    //
    // forward declaration
    class RenderPassSystemPrivate;

    /* \class RenderPassFactory RenderPassSystem.hh \
     * gz/rendering/RenderPassSystem.hh
     */
    /// \brief A factory interface for creating render passes
    class GZ_RENDERING_VISIBLE RenderPassFactory
    {
      /// \brief Instantiate new render pass
      /// \return New render pass
      public: virtual RenderPass *New() const = 0;
    };

    class GZ_RENDERING_VISIBLE BaseRenderPassSystem
    {
      public: using RenderPassFactoryFn = std::function<RenderPass*(void)>;

      public: BaseRenderPassSystem();

      public: virtual ~BaseRenderPassSystem();

      /// \brief Register a render pass factory to the system
      /// \param[in] _type Render pass type, i.e. type id of render pass class
      /// \param[in] _factory Factory function used to create the render pass
      public: void Register(const std::string &_type,
          RenderPassFactoryFn _factoryFn);

      /// \brief Implementation for creating render passes
      /// \param[in] _type Render pass type, i.e. type id of render pass class
      /// \return Pointer to the render pass created
      public: RenderPassPtr Create(const std::string &_type);

      public: template<typename T> RenderPassPtr Create()
              {
                return this->Create(typeid(T).name());
              }

      GZ_UTILS_UNIQUE_IMPL_PTR(dataPtr);
    };

    /* \class RenderPassSystem RenderPassSystem.hh \
     * gz/rendering/RenderPassSystem.hh
     */
    /// \brief A class for creating and managing render passes
    class GZ_RENDERING_VISIBLE RenderPassSystem
    {
      private: static BaseRenderPassSystem& Implementation();

      /// \brief Templated function for creating render passes
      /// \return Pointer to the render pass created
      public: template<typename T> RenderPassPtr Create()
              {
                return this->Implementation().Create(typeid(T).name());
              }

      /// \brief Register a render pass factory to the system
      /// \param[in] _type Render pass type, i.e. type id of render pass class
      /// \param[in] _factory Factory used to create the render pass
      public: static void Register(const std::string &_type,
          RenderPassFactory *_factory);
    };

    /// \brief Render pass registration macro
    ///
    /// Use this macro to register render pass with the render pass factory.
    #define GZ_RENDERING_REGISTER_RENDER_PASS(classname, interface) \
    class classname##Factory : public gz::rendering::RenderPassFactory \
    { \
      public: classname##Factory() \
              { \
                gz::rendering::RenderPassSystem::Register( \
                    typeid(interface).name(), this); \
              } \
      public: RenderPass *New() const override \
              { \
                return new classname(); \
              } \
    }; \
    static classname##Factory global_##classname##Factory;
    }
  }
}
#endif
