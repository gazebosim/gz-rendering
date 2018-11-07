/*
 * Copyright (C) 2018 Open Source Robotics Foundation
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
#ifndef IGNITION_RENDERING_BASE_BASEGPURAYS_HH_
#define IGNITION_RENDERING_BASE_BASEGPURAYS_HH_

#include <string>

#include <ignition/common/Event.hh>
#include <ignition/common/Console.hh>

#include "ignition/rendering/GpuRays.hh"
#include "ignition/rendering/Image.hh"
#include "ignition/rendering/RenderEngine.hh"
#include "ignition/rendering/Scene.hh"
#include "ignition/rendering/base/BaseRenderTarget.hh"
#include "ignition/rendering/base/BaseCamera.hh"
#include "ignition/rendering/Visual.hh"
#include "ignition/rendering/RenderTypes.hh"


namespace ignition
{
  namespace rendering
  {
    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
    template <class T>
    class IGNITION_RENDERING_VISIBLE BaseGpuRays :
      public virtual GpuRays,
      public virtual BaseCamera<T>,
      public virtual T
    {
      /// \brief Constructor
      protected: BaseGpuRays();

      /// \brief Destructor
      public: virtual ~BaseGpuRays();

      /// \brief All things needed to get back z buffer for gpu rays data.
      /// \return Array of gpu rays data.
      public: virtual float * Data() const override;

      public: virtual void CopyData(float *_data) override;

      /// \brief Connect to a gpu rays frame signal
      /// \param[in] _subscriber Callback that is called when a new image is
      /// generated
      /// \return A pointer to the connection. This must be kept in scope.
      public: virtual common::ConnectionPtr ConnectNewGpuRaysFrame(
                  std::function<void(const float *_frame, unsigned int _width,
                  unsigned int _height, unsigned int _depth,
                  const std::string &_format)> _subscriber) override;

      /// \return Pointer to the render target
      public: virtual RenderTargetPtr RenderTarget() const override = 0;

      /// \brief Set sensor horizontal or vertical
      /// \param[in] _horizontal True if horizontal, false if not
      public: virtual void SetIsHorizontal(const bool _horizontal) override;

      /// \brief Gets if sensor is horizontal
      /// \return True if horizontal, false if not
      public: virtual bool IsHorizontal() const override;

      /// \brief Get the vertical field-of-view.
      /// \return The vertical field of view of the gpu rays.
      public: virtual math::Angle VFOV() const override;

      /// \brief Get the ray count ratio (equivalent to aspect ratio)
      /// \return The ray count ratio (equivalent to aspect ratio)
      public: virtual double RayCountRatio() const override;

      /// \brief Get the ray count ratio (equivalent to aspect ratio)
      /// \return The ray count ratio (equivalent to aspect ratio)
      public: virtual double RangeCountRatio() const override;

      /// \brief Sets the ray count ratio (equivalent to aspect ratio)
      /// \param[in] _rayCountRatio ray count ratio (equivalent to aspect ratio)
      public: virtual void SetRayCountRatio(
                  const double _rayCountRatio) override;

      // Documentation inherited.
      public: virtual ignition::math::Angle AngleMin() const override;

      // Documentation inherited.
      public: virtual void SetAngleMin(double _angle) override;

      // Documentation inherited.
      public: virtual ignition::math::Angle AngleMax() const override;

      // Documentation inherited.
      public: virtual void SetAngleMax(double _angle) override;

      // Documentation inherited.
      public: virtual void SetVerticalRayCount(int _samples) override;

      // Documentation inherited.
      public: virtual void SetRayCount(int _samples) override;

      // Documentation inherited.
      public: virtual int RayCount() const override;

      // Documentation inherited.
      public: virtual int RangeCount() const override;

      // Documentation inherited.
      public: virtual int VerticalRayCount() const override;

      // Documentation inherited.
      public: virtual int VerticalRangeCount() const override;

      // Documentation inherited.
      public: virtual ignition::math::Angle VerticalAngleMin() const override;

      // Documentation inherited.
      public: virtual void SetVerticalAngleMin(const double _angle) override;

      // Documentation inherited.
      public: virtual ignition::math::Angle VerticalAngleMax() const override;

      // Documentation inherited.
      public: virtual void SetVerticalAngleMax(const double _angle) override;

      /// \internal
      /// \brief Get Cos Horz field-of-view
      /// \return 2 * atan(tan(this->hfov/2) / cos(this->vfov/2))
      public: virtual double CosHorzFOV() const;

      /// \internal
      /// \brief Set the Cos Horz FOV
      /// \param[in] _chfov Cos Horz FOV
      public: virtual void SetCosHorzFOV(const double _chfov);

      /// \internal
      /// \brief Set the vertical fov
      /// \param[in] _vfov vertical fov
      public: virtual void SetVFOV(const math::Angle _vfov) override;

      /// \internal
      /// \brief Get Cos Vert field-of-view
      /// \return 2 * atan(tan(this->vfov/2) / cos(this->hfov/2))
      public: virtual double CosVertFOV() const;

      /// \internal
      /// \brief Set the Cos Horz FOV
      /// \param[in] _cvfov Cos Horz FOV
      public: virtual void SetCosVertFOV(const double _cvfov);

      /// \internal
      /// \brief Get (horizontal_max_angle + horizontal_min_angle) * 0.5
      /// \return (horizontal_max_angle + horizontal_min_angle) * 0.5
      public: virtual double HorzHalfAngle() const;

      /// \internal
      /// \brief Get (vertical_max_angle + vertical_min_angle) * 0.5
      /// \return (vertical_max_angle + vertical_min_angle) * 0.5
      public: virtual double VertHalfAngle() const;

      /// \internal
      /// \brief Set the horizontal half angle
      /// \param[in] _angle horizontal half angle
      public: virtual void SetHorzHalfAngle(const double _angle);

      /// \internal
      /// \brief Set the vertical half angle
      /// \param[in] _angle vertical half angle
      public: virtual void SetVertHalfAngle(const double _angle);

      /// \internal
      /// \brief Get the number of cameras required
      /// \return Number of cameras needed to generate the rays
      public: virtual unsigned int CameraCount() const;

      /// \internal
      /// \brief Set the number of cameras required
      /// \param[in] _cameraCount The number of cameras required to generate
      /// the rays
      public: virtual void SetCameraCount(
                  const unsigned int _cameraCount);

      /// \brief Horizontal half angle.
      protected: double horzHalfAngle = 0;

      /// \brief Vertical half angle.
      protected: double vertHalfAngle = 0;

      /// \brief Ray count ratio.
      protected: double rayCountRatio = 0;

      /// \brief Range count ratio.
      protected: double rangeCountRatio = 0;

      /// \brief Vertical field-of-view.
      protected: math::Angle vfov;

      /// \brief Cos horizontal field-of-view.
      protected: double chfov = 0;

      /// \brief Cos vertical field-of-view.
      protected: double cvfov = 0;

      /// \brief True if the sensor is horizontal only.
      protected: bool isHorizontal = true;

      /// \brief Number of cameras needed to generate the rays.
      protected: unsigned int cameraCount = 1;

      /// \brief Horizontal minimal angle
      protected: double minAngle = 0;

      /// \brief Horizontal maximal angle
      protected: double maxAngle = 0;

      /// \brief Vertical minimal angle
      protected: double vMinAngle = 0;

      /// \brief Vertical maximal angle
      protected: double vMaxAngle = 0;

      /// \brief Quantity of horizontal rays
      protected: int hSamples = 0;

      /// \brief Quantity of verical rays
      protected: int vSamples = 0;

      /// \brief Resolution of horizontal rays
      protected: int hResolution = 1;

      /// \brief Resolution of vertical rays
      protected: int vResolution = 1;

      private: friend class OgreScene;
    };

    //////////////////////////////////////////////////
    template <class T>
    BaseGpuRays<T>::BaseGpuRays()
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    BaseGpuRays<T>::~BaseGpuRays()
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    float *  BaseGpuRays<T>::Data() const
    {
      return nullptr;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseGpuRays<T>::CopyData(float * /*_dataDest*/)
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    ignition::common::ConnectionPtr BaseGpuRays<T>::ConnectNewGpuRaysFrame(
          std::function<void(const float *, unsigned int, unsigned int,
          unsigned int, const std::string &)>)
    {
      return nullptr;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseGpuRays<T>::SetHorzHalfAngle(const double _angle)
    {
      this->horzHalfAngle = _angle;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseGpuRays<T>::SetVertHalfAngle(const double _angle)
    {
      this->vertHalfAngle = _angle;
    }

    //////////////////////////////////////////////////
    template <class T>
    double BaseGpuRays<T>::HorzHalfAngle() const
    {
      return this->horzHalfAngle;
    }

    //////////////////////////////////////////////////
    template <class T>
    double BaseGpuRays<T>::VertHalfAngle() const
    {
      return this->vertHalfAngle;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseGpuRays<T>::SetIsHorizontal(const bool _horizontal)
    {
      this->isHorizontal = _horizontal;
    }

    //////////////////////////////////////////////////
    template <class T>
    bool BaseGpuRays<T>::IsHorizontal() const
    {
      return this->isHorizontal;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseGpuRays<T>::SetCosVertFOV(const double _cvfov)
    {
      this->cvfov = _cvfov;
    }

    //////////////////////////////////////////////////
    template <class T>
    unsigned int BaseGpuRays<T>::CameraCount() const
    {
      return this->cameraCount;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseGpuRays<T>::SetCameraCount(const unsigned int _cameraCount)
    {
      this->cameraCount = _cameraCount;
    }

    //////////////////////////////////////////////////
    template <class T>
    double BaseGpuRays<T>::RayCountRatio() const
    {
      return this->rayCountRatio;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseGpuRays<T>::SetRayCountRatio(const double _rayCountRatio)
    {
      this->rayCountRatio = _rayCountRatio;
    }

    //////////////////////////////////////////////////
    template <class T>
    double BaseGpuRays<T>::RangeCountRatio() const
    {
      return this->rangeCountRatio;
    }

    //////////////////////////////////////////////////
    template <class T>
    math::Angle BaseGpuRays<T>::VFOV() const
    {
      return this->vfov;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseGpuRays<T>::SetVFOV(const math::Angle _vfov)
    {
      this->vfov = _vfov;
    }

    //////////////////////////////////////////////////
    template <class T>
    double BaseGpuRays<T>::CosHorzFOV() const
    {
      return this->chfov;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseGpuRays<T>::SetCosHorzFOV(const double _chfov)
    {
      this->chfov = _chfov;
    }

    //////////////////////////////////////////////////
    template <class T>
    double BaseGpuRays<T>::CosVertFOV() const
    {
      return this->cvfov;
    }

    template <class T>
    //////////////////////////////////////////////////
    ignition::math::Angle BaseGpuRays<T>::AngleMin() const
    {
      return this->minAngle;
    }

    template <class T>
    //////////////////////////////////////////////////
    void BaseGpuRays<T>::SetAngleMin(double _angle)
    {
      this->minAngle = _angle;
    }

    template <class T>
    //////////////////////////////////////////////////
    ignition::math::Angle BaseGpuRays<T>::AngleMax() const
    {
      return this->maxAngle;
    }

    template <class T>
    //////////////////////////////////////////////////
    void BaseGpuRays<T>::SetAngleMax(double _angle)
    {
      this->maxAngle = _angle;
    }

    template <class T>
    //////////////////////////////////////////////////
    int BaseGpuRays<T>::RayCount() const
    {
      return this->hSamples;
    }

    template <class T>
    //////////////////////////////////////////////////
    void BaseGpuRays<T>::SetRayCount(int _samples)
    {
      this->hSamples = _samples;
    }

    template <class T>
    //////////////////////////////////////////////////
    int BaseGpuRays<T>::RangeCount() const
    {
      return this->RayCount() * this->hResolution;
    }

    template <class T>
    //////////////////////////////////////////////////
    int BaseGpuRays<T>::VerticalRayCount() const
    {
      return this->vSamples;
    }

    template <class T>
    //////////////////////////////////////////////////
    void BaseGpuRays<T>::SetVerticalRayCount(int _samples)
    {
      this->vSamples = _samples;
    }

    template <class T>
    //////////////////////////////////////////////////
    int BaseGpuRays<T>::VerticalRangeCount() const
    {
      return this->VerticalRayCount() * this->vResolution;
    }

    template <class T>
    //////////////////////////////////////////////////
    ignition::math::Angle BaseGpuRays<T>::VerticalAngleMin() const
    {
      return this->vMinAngle;
    }

    template <class T>
    //////////////////////////////////////////////////
    void BaseGpuRays<T>::SetVerticalAngleMin(const double _angle)
    {
        this->vMinAngle = _angle;
    }

    template <class T>
    //////////////////////////////////////////////////
    ignition::math::Angle BaseGpuRays<T>::VerticalAngleMax() const
    {
      return this->vMaxAngle;
    }

    template <class T>
    //////////////////////////////////////////////////
    void BaseGpuRays<T>::SetVerticalAngleMax(const double _angle)
    {
        this->vMaxAngle = _angle;
    }
    }
  }
}
#endif
