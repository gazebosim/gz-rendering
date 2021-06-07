/*
 * Copyright (C) 2021 Open Source Robotics Foundation
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
#ifndef IGNITION_RENDERING_BASE_BASEBOUNDINGBOXCAMERA_HH_
#define IGNITION_RENDERING_BASE_BASEBOUNDINGBOXCAMERA_HH_

#include <string>
#include <vector>

#include <ignition/common/Event.hh>

#include "ignition/rendering/base/BaseCamera.hh"
#include "ignition/rendering/BoundingBoxCamera.hh"

namespace ignition
{
  namespace rendering
  {
    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {

    template <class T>
    class BaseBoundingBoxCamera :
      public virtual BoundingBoxCamera,
      public virtual BaseCamera<T>,
      public virtual T
    {
      protected: BaseBoundingBoxCamera();

      public: virtual ~BaseBoundingBoxCamera();

      // Documentation inherited
      public: virtual std::vector<BoundingBox> BoundingBoxData() const;

      // Documentation inherited
      public: virtual ignition::common::ConnectionPtr
        ConnectNewBoundingBoxes(
          std::function<void(const std::vector<BoundingBox> &)>  _subscriber);

      // Documentation inherited
      public: virtual void SetBoundingBoxType(BoundingBoxType _type);

      // Documentation inherited
      public: virtual void DrawBoundingBox(unsigned char *_data,
        BoundingBox &_box);
    };

    //////////////////////////////////////////////////
    template <class T>
    BaseBoundingBoxCamera<T>::BaseBoundingBoxCamera()
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    BaseBoundingBoxCamera<T>::~BaseBoundingBoxCamera()
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    std::vector<BoundingBox> BaseBoundingBoxCamera<T>::BoundingBoxData() const
    {
      return std::vector<BoundingBox>();
    }

    //////////////////////////////////////////////////
    template <class T>
    ignition::common::ConnectionPtr BaseBoundingBoxCamera<T>::
      ConnectNewBoundingBoxes(
        std::function<void(const std::vector<BoundingBox> &)>)
    {
      return nullptr;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseBoundingBoxCamera<T>::SetBoundingBoxType(BoundingBoxType)
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseBoundingBoxCamera<T>::DrawBoundingBox(unsigned char *,
      BoundingBox &)
    {
    }
  }
  }
}
#endif
