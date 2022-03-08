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
#ifndef IGNITION_RENDERING_BASE_BASESONARCONE_HH_
#define IGNITION_RENDERING_BASE_BASESONARCONE_HH_

#include <string>

#include <ignition/common/Event.hh>

#include "ignition/rendering/base/BaseCamera.hh"
#include "ignition/rendering/SonarCone.hh"

namespace ignition
{
  namespace rendering
  {
    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
    template <class T>
    class BaseSonarCone :
      public virtual SonarCone,
      public virtual BaseCamera<T>,
      public virtual T
    {
      protected: BaseSonarCone();

      public: virtual ~BaseSonarCone();

      public: virtual void CreateDepthTexture();

      public: virtual const float *DepthData() const;

      public: virtual ignition::common::ConnectionPtr ConnectNewDepthFrame(
          std::function<void(const float *, unsigned int, unsigned int,
          unsigned int, const std::string &)>  _subscriber);

      public: virtual ignition::common::ConnectionPtr ConnectNewRGBPointCloud(
          std::function<void(const float *, unsigned int, unsigned int,
          unsigned int, const std::string &)>  _subscriber);
    };

    //////////////////////////////////////////////////
    template <class T>
    BaseSonarCone<T>::BaseSonarCone()
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    BaseSonarCone<T>::~BaseSonarCone()
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    const float *BaseSonarCone<T>::DepthData() const
    {
      return nullptr;
    }

    //////////////////////////////////////////////////
    template <class T>
    ignition::common::ConnectionPtr BaseSonarCone<T>::ConnectNewDepthFrame(
          std::function<void(const float *, unsigned int, unsigned int,
          unsigned int, const std::string &)>)
    {
      return nullptr;
    }

    //////////////////////////////////////////////////
    template <class T>
    ignition::common::ConnectionPtr BaseSonarCone<T>::ConnectNewRGBPointCloud(
          std::function<void(const float *, unsigned int, unsigned int,
          unsigned int, const std::string &)>)
    {
      return nullptr;
    }
  }
  }
}
#endif
