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
#ifndef IGNITION_RENDERING_BASE_BASERENDERPASS_HH_
#define IGNITION_RENDERING_BASE_BASERENDERPASS_HH_

#include <string>
#include "ignition/rendering/RenderPass.hh"

namespace ignition
{
  namespace rendering
  {
    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
    //
    /// \class BaseRenderPass BaseRenderPass.hh
    /// ignition/rendering/base/BaseRenderPass.hh
    /// \brief Base render pass that can be applied to a camera
    template <class T>
    class BaseRenderPass:
      public virtual RenderPass,
      public T
    {
      /// \brief Constructor
      protected: BaseRenderPass();

      /// \brief Destructor
      public: virtual ~BaseRenderPass();
    };

    //////////////////////////////////////////////////
    // BaseRenderPass
    //////////////////////////////////////////////////
    template <class T>
    BaseRenderPass<T>::BaseRenderPass()
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    BaseRenderPass<T>::~BaseRenderPass()
    {
    }
    }
  }
}
#endif
