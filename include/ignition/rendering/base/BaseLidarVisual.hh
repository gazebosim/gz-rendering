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
#ifndef IGNITION_RENDERING_BASELIDARVISUAL_HH_
#define IGNITION_RENDERING_BASELIDARVISUAL_HH_

#include "ignition/rendering/LidarVisual.hh"
#include "ignition/rendering/base/BaseObject.hh"
#include "ignition/rendering/base/BaseRenderTypes.hh"

namespace ignition
{
  namespace rendering
  {
    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
    /// \brief Base implementation of a LidarVisual geometry
    template <class T>
    class BaseLidarVisual :
      public virtual LidarVisual,
      public virtual T
    {
      /// \brief Constructor.
      protected: BaseLidarVisual();

      /// \brief Destructor.
      public: virtual ~BaseLidarVisual();

      /// \brief PreRender function
      public: virtual void PreRender() override;

      /// \brief Destroy function
      public: virtual void Destroy() override;

      // Documentation inherited
      public: virtual void SetLifetime(const
                  std::chrono::steady_clock::duration &_lifetime) override;

      // Documentation inherited
      public: virtual std::chrono::steady_clock::duration Lifetime()
                  const override;

      // Documentation inherited
      public: virtual void SetType(const LidarVisualType _lidarVisualType) override;

      // Documentation inherited
      public: virtual LidarVisualType Type() const override;

      // Documentation inherited
      public: virtual void SetLayer(int32_t _layer) override;

      // Documentation inherited
      public: virtual int32_t Layer() const override;

      // Documentation inherited
      public: virtual void ClearPoints() override;

      // Documentation inherited
      public: virtual void AddPoint(double _x,
                  double _y, double _z,
                  const ignition::math::Color &_color) override;

      // Documentation inherited
      public: virtual void AddPoint(const ignition::math::Vector3d &_pt,
                  const ignition::math::Color &_color) override;

      // Documentation inherited
      public: virtual void SetPoint(unsigned int _index,
                  const ignition::math::Vector3d &_value) override;

      /// \brief Life time of a LidarVisual
      protected: std::chrono::steady_clock::duration lifetime =
          std::chrono::steady_clock::duration::zero();

      /// \brief Layer at which the LidarVisual will reside
      protected: int32_t layer = 0;

      /// \brief Flag to indicate if visual needs to be updated
      protected: bool lidarVisualDirty = true;

      /// \brief LidarVisual type
      protected: LidarVisualType lidarVisualType =
          ignition::rendering::LidarVisualType::MT_NONE;
    };

    /////////////////////////////////////////////////
    // BaseLidarVisual
    /////////////////////////////////////////////////
    template <class T>
    BaseLidarVisual<T>::BaseLidarVisual()
    {
    }

    /////////////////////////////////////////////////
    template <class T>
    BaseLidarVisual<T>::~BaseLidarVisual()
    {
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseLidarVisual<T>::SetLifetime(
        const std::chrono::steady_clock::duration &_lifetime)
    {
      this->lifetime = _lifetime;
      this->lidarVisualDirty = true;
    }

    /////////////////////////////////////////////////
    template <class T>
    std::chrono::steady_clock::duration BaseLidarVisual<T>::Lifetime() const
    {
      return this->lifetime;
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseLidarVisual<T>::SetLayer(int32_t _layer)
    {
      this->layer = _layer;
      this->lidarVisualDirty = true;
    }

    /////////////////////////////////////////////////
    template <class T>
    int32_t BaseLidarVisual<T>::Layer() const
    {
      return this->layer;
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseLidarVisual<T>::SetType(const LidarVisualType _lidarVisualType)
    {
      this->lidarVisualType = _lidarVisualType;
      this->lidarVisualDirty = true;
    }

    /////////////////////////////////////////////////
    template <class T>
    LidarVisualType BaseLidarVisual<T>::Type() const
    {
      return this->lidarVisualType;
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseLidarVisual<T>::PreRender()
    {
      T::PreRender();
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseLidarVisual<T>::Destroy()
    {
      T::Destroy();
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseLidarVisual<T>::ClearPoints()
    {
        // no op
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseLidarVisual<T>::AddPoint(const ignition::math::Vector3d &,
                                 const ignition::math::Color &)
    {
        // no op
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseLidarVisual<T>::AddPoint(double _x, double _y, double _z,
                  const ignition::math::Color &_color)
    {
      this->AddPoint(ignition::math::Vector3d(_x, _y, _z), _color);
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseLidarVisual<T>::SetPoint(unsigned int,
                  const ignition::math::Vector3d &)
    {
      // no op
    }
    }
  }
}
#endif
