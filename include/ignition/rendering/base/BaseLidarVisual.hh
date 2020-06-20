/*
 * Copyright (C) 2020 Open Source Robotics Foundation
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
    /// \brief Base implementation of a Lidar Visual
    template <class T>
    class BaseLidarVisual :
      public virtual LidarVisual,
      public virtual T
    {
      // Documentation inherited
      protected: BaseLidarVisual();

      // Documentation inherited
      public: virtual ~BaseLidarVisual();

      // Documentation inherited
      public: virtual void PreRender() override;

      // Documentation inherited
      public: virtual void Destroy() override;

      // Documentation inherited
      public: virtual void ClearPoints() override;

      // Documentation inherited
      public: virtual void SetLidarMessage(std::vector<double> &msg);

      // Documentation inherited
      public: virtual void Update();

      // Documentation inherited
      public: virtual void Init();

      // Documentation inherited
      public: virtual void SetMinVerticalAngle(const double );

      // Documentation inherited
      public: virtual double MinVerticalAngle();

      // Documentation inherited
      public: virtual void SetMaxVerticalAngle(const double );

      // Documentation inherited
      public: virtual double MaxVerticalAngle();

      // Documentation inherited
      public: virtual void SetMinHorizontalAngle(const double );

      // Documentation inherited
      public: virtual double MinHorizontalAngle();

      // Documentation inherited
      public: virtual void SetMaxHorizontalAngle(const double );

      // Documentation inherited
      public: virtual double MaxHorizontalAngle();

      // Documentation inherited
      public: virtual void SetVerticalRayCount(const unsigned int );

      // Documentation inherited
      public: virtual unsigned int VerticalRayCount();

      // Documentation inherited
      public: virtual void SetHorizontalRayCount(const unsigned int );

      // Documentation inherited
      public: virtual unsigned int HorizontalRayCount();

      // Documentation inherited
      public: virtual void SetMinRange(const double );

      // Documentation inherited
      public: virtual double MinRange();

      // Documentation inherited
      public: virtual void SetMaxRange(const double );

      // Documentation inherited
      public: virtual double MaxRange();

      // Documentation inherited
      public: virtual void SetVerticalAngleStep(const double );

      // Documentation inherited
      public: virtual double VerticalAngleStep();

      // Documentation inherited
      public: virtual void SetHorizontalAngleStep(const double );

      // Documentation inherited
      public: virtual double HorizontalAngleStep();

      // Documentation inherited
      public: virtual void SetOffset(const ignition::math::Pose3d );

      // Documentation inherited
      public: virtual ignition::math::Pose3d  Offset();

      // Documentation inherited
      public: virtual unsigned int GetPointCount();

      /// \brief Vertical minimal angle
      protected: double minVerticalAngle;

      /// \brief Vertical maximum angle
      protected: double maxVerticalAngle;

      /// \brief Vertical laser count
      protected: unsigned int verticalCount;

      /// \brief Angle between two vertical rays
      protected: double verticalAngleStep;

      /// \brief Horizontal minimal angle
      protected: double minHorizontalAngle;

      /// \brief Horizontal maximum angle
      protected: double maxHorizontalAngle;

      /// \brief Horizontal laser count
      protected: unsigned int horizontalCount;

      /// \brief Angle between two horizontal rays
      protected: double horizontalAngleStep;

      /// \brief Minimum Range
      protected: double minRange;

      /// \brief Maximum Range
      protected: double maxRange;

      /// \brief Offset of visual
      protected: ignition::math::Pose3d offset;

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
    unsigned int BaseLidarVisual<T>::GetPointCount()
    {
        // no op
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseLidarVisual<T>::Update()
    {
      // no op
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseLidarVisual<T>::SetLidarMessage(std::vector<double> &)
    {
      // no op
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseLidarVisual<T>::Init()
    {
      T::Init();
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseLidarVisual<T>::SetMinVerticalAngle(const double _minVerticalAngle)
    {
      this->minVerticalAngle = _minVerticalAngle;
    }

    /////////////////////////////////////////////////
    template <class T>
    double BaseLidarVisual<T>::MinVerticalAngle()
    {
      return this->minVerticalAngle;
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseLidarVisual<T>::SetMaxVerticalAngle(const double _maxVerticalAngle)
    {
      this->maxVerticalAngle = _maxVerticalAngle;
    }

    /////////////////////////////////////////////////
    template <class T>
    double BaseLidarVisual<T>::MaxVerticalAngle()
    {
      return this->maxVerticalAngle;
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseLidarVisual<T>::SetVerticalRayCount(const unsigned int _verticalRayCount)
    {
      this->verticalCount = _verticalRayCount;
    }

    /////////////////////////////////////////////////
    template <class T>
    unsigned int BaseLidarVisual<T>::VerticalRayCount()
    {
      return this->verticalCount;
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseLidarVisual<T>::SetVerticalAngleStep(const double _verticalAngleStep)
    {
      this->verticalAngleStep = _verticalAngleStep;
    }

    /////////////////////////////////////////////////
    template <class T>
    double BaseLidarVisual<T>::VerticalAngleStep()
    {
      return this->verticalAngleStep;
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseLidarVisual<T>::SetMinHorizontalAngle(const double _minHorizontalAngle)
    {
      this->minHorizontalAngle = _minHorizontalAngle;
    }

    /////////////////////////////////////////////////
    template <class T>
    double BaseLidarVisual<T>::MinHorizontalAngle()
    {
      return this->minHorizontalAngle;
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseLidarVisual<T>::SetMaxHorizontalAngle(const double _maxHorizontalAngle)
    {
      this->maxHorizontalAngle = _maxHorizontalAngle;
    }

    /////////////////////////////////////////////////
    template <class T>
    double BaseLidarVisual<T>::MaxHorizontalAngle()
    {
      return this->maxHorizontalAngle;
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseLidarVisual<T>::SetHorizontalRayCount(const unsigned int _horizontalRayCount)
    {
      this->horizontalCount = _horizontalRayCount;
    }

    /////////////////////////////////////////////////
    template <class T>
    unsigned int BaseLidarVisual<T>::HorizontalRayCount()
    {
      return this->horizontalCount;
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseLidarVisual<T>::SetHorizontalAngleStep(const double _horizontalAngleStep)
    {
      this->horizontalAngleStep = _horizontalAngleStep;
    }

    /////////////////////////////////////////////////
    template <class T>
    double BaseLidarVisual<T>::HorizontalAngleStep()
    {
      return this->horizontalAngleStep;
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseLidarVisual<T>::SetMinRange(const double _minRange)
    {
      this->minRange = _minRange;
    }

    /////////////////////////////////////////////////
    template <class T>
    double BaseLidarVisual<T>::MinRange()
    {
      return this->minRange;
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseLidarVisual<T>::SetMaxRange(const double _maxRange)
    {
      this->maxRange = _maxRange;
    }

    /////////////////////////////////////////////////
    template <class T>
    double BaseLidarVisual<T>::MaxRange()
    {
      return this->maxRange;
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseLidarVisual<T>::SetOffset(const ignition::math::Pose3d _offset)
    {
      this->offset = _offset;
    }

    /////////////////////////////////////////////////
    template <class T>
    ignition::math::Pose3d BaseLidarVisual<T>::Offset()
    {
      return this->offset;
    }

    }
  }
}
#endif
