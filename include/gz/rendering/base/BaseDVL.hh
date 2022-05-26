/*
 * Copyright (C) 2022 Open Source Robotics Foundation
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
#ifndef GZ_RENDERING_BASE_BASEDVL_HH_
#define GZ_RENDERING_BASE_BASEDVL_HH_

#include <string>

#include <gz/common/Event.hh>

#include "gz/rendering/base/BaseCamera.hh"
#include "gz/rendering/DVL.hh"

namespace ignition
{
  namespace rendering
  {
    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {

    template<class T>
    class BaseDVL :
      public virtual DopplerVelocityLog,
      public virtual BaseCamera<T>,
      public virtual T
    {
      /// \brief Constructor
      protected: BaseDVL();

      /// \brief Destructor
      public: virtual ~BaseDVL();

      // Documentation inherited
      public: virtual void CreateDopplerVelocityLogTexture() override = 0;

      // Documentation inherited
      public: virtual uint8_t *VisibilityDepthData() const override;

      // Documentation inherited
      public: virtual ignition::common::ConnectionPtr
        ConnectNewDepthVisibilitymapFrame(
          std::function<void(const uint8_t *, unsigned int, unsigned int,
          unsigned int, const std::string &)>  _subscriber) override;

    };

    }
  }
}
#endif