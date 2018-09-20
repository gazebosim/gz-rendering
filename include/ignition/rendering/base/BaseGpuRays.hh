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
      protected virtual BaseCamera<T>,
      public virtual T
    {
      /// \brief Constructor
      protected: BaseGpuRays();

      /// \brief Destructor
      public: virtual ~BaseGpuRays();

      /// \brief Create the texture which is used to render laser data.
      public: virtual void CreateLaserTexture() override;

      /// \brief All things needed to get back z buffer for laser data.
      /// \return Array of laser data.
      /// \deprecated use LaserDataBegin() and LaserDataEnd() instead
      public: virtual const float *LaserData() const override;

      /// \brief Connect to a laser frame signal
      /// \param[in] _subscriber Callback that is called when a new image is
      /// generated
      /// \return A pointer to the connection. This must be kept in scope.
      public: virtual common::ConnectionPtr ConnectNewLaserFrame(
                  std::function<void(const float *_frame, unsigned int _width,
                  unsigned int _height, unsigned int _depth,
                  const std::string &_format)> _subscriber) override;

      /// \return Pointer to the render target
      public: virtual RenderTargetPtr RenderTarget() const = 0;

      /// \brief Set the number of samples in the width and height for the
      /// first pass texture.
      /// \param[in] _w Number of samples in the horizontal sweep
      /// \param[in] _h Number of samples in the vertical sweep
      public: virtual void SetRangeCount(const unsigned int _w,
          const unsigned int _h = 1) override;

      /// \brief Get (horizontal_max_angle + horizontal_min_angle) * 0.5
      /// \return (horizontal_max_angle + horizontal_min_angle) * 0.5
      public: virtual double HorzHalfAngle() const override;

      /// \brief Get (vertical_max_angle + vertical_min_angle) * 0.5
      /// \return (vertical_max_angle + vertical_min_angle) * 0.5
      public: virtual double VertHalfAngle() const override;

      /// \brief Set the horizontal half angle
      /// \param[in] _angle horizontal half angle
      public: virtual void SetHorzHalfAngle(const double _angle) override;

      /// \brief Set the vertical half angle
      /// \param[in] _angle vertical half angle
      public: virtual void SetVertHalfAngle(const double _angle) override;

      /// \brief Set sensor horizontal or vertical
      /// \param[in] _horizontal True if horizontal, false if not
      public: virtual void SetIsHorizontal(const bool _horizontal) override;

      /// \brief Gets if sensor is horizontal
      /// \return True if horizontal, false if not
      public: virtual bool IsHorizontal() const override;

      /// \brief Get the horizontal field of view of the laser sensor.
      /// \return The horizontal field of view of the laser sensor.
      public: virtual double HorzFOV() const override;

      /// \brief Get Cos Horz field-of-view
      /// \return 2 * atan(tan(this->hfov/2) / cos(this->vfov/2))
      public: virtual double CosHorzFOV() const override;

      /// \brief Set the Cos Horz FOV
      /// \param[in] _chfov Cos Horz FOV
      public: virtual void SetCosHorzFOV(const double _chfov) override;

      /// \brief Get the vertical field-of-view.
      /// \return The vertical field of view of the laser sensor.
      public: virtual double VertFOV() const override;

      /// \brief Get Cos Vert field-of-view
      /// \return 2 * atan(tan(this->vfov/2) / cos(this->hfov/2))
      public: virtual double CosVertFOV() const override;

      /// \brief Set the Cos Horz FOV
      /// \param[in] _cvfov Cos Horz FOV
      public: virtual void SetCosVertFOV(const double _cvfov) override;

      /// \brief Set the horizontal fov
      /// \param[in] _hfov horizontal fov
      public: virtual void SetHorzFOV(const double _hfov) override;

      /// \brief Set the vertical fov
      /// \param[in] _vfov vertical fov
      public: virtual void SetVertFOV(const double _vfov) override;

      /// \brief Get the number of cameras required
      /// \return Number of cameras needed to generate the rays
      public: virtual unsigned int CameraCount() const override;

      /// \brief Set the number of cameras required
      /// \param[in] _cameraCount The number of cameras required to generate
      /// the rays
      public: virtual void SetCameraCount(
                  const unsigned int _cameraCount) override;

      /// \brief Get the ray count ratio (equivalent to aspect ratio)
      /// \return The ray count ratio (equivalent to aspect ratio)
      public: virtual double RayCountRatio() const override;

      /// \brief Sets the ray count ratio (equivalent to aspect ratio)
      /// \param[in] _rayCountRatio ray count ratio (equivalent to aspect ratio)
      public: virtual void SetRayCountRatio(
                  const double _rayCountRatio) override;

      /// \brief Horizontal half angle.
      protected: double horzHalfAngle = 0;

      /// \brief Vertical half angle.
      protected: double vertHalfAngle = 0;

      /// \brief Ray count ratio.
      protected: double rayCountRatio = 0;

      /// \brief Horizontal field-of-view.
      protected: double hfov = 0;

      /// \brief Vertical field-of-view.
      protected: double vfov = 0;

      /// \brief Cos horizontal field-of-view.
      protected: double chfov = 0;

      /// \brief Cos vertical field-of-view.
      protected: double cvfov = 0;

      /// \brief Near clip plane.
      protected: double nearClip = 0;

      /// \brief Far clip plane.
      protected: double farClip = 0;

      /// \brief True if the sensor is horizontal only.
      protected: bool isHorizontal = true;

      /// \brief Number of cameras needed to generate the rays.
      protected: unsigned int cameraCount = 0;

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
    void BaseGpuRays<T>::CreateLaserTexture()
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    const float *BaseGpuRays<T>::LaserData() const
    {
      return nullptr;
    }

    //////////////////////////////////////////////////
    template <class T>
    ignition::common::ConnectionPtr BaseGpuRays<T>::ConnectNewLaserFrame(
          std::function<void(const float *, unsigned int, unsigned int,
          unsigned int, const std::string &)>)
    {
      return nullptr;
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseGpuRays<T>::SetRangeCount(const unsigned int /*_w*/, const unsigned int /*_h*/)
    {
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
    double BaseGpuRays<T>::HorzFOV() const
    {
      return this->hfov;
    }

    //////////////////////////////////////////////////
    template <class T>
    double BaseGpuRays<T>::VertFOV() const
    {
      return this->vfov;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseGpuRays<T>::SetHorzFOV(const double _hfov)
    {
      this->hfov = _hfov;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseGpuRays<T>::SetVertFOV(const double _vfov)
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

    //////////////////////////////////////////////////
    template <class T>
    void BaseGpuRays<T>::SetCosVertFOV(const double _cvfov)
    {
      this->cvfov = _cvfov;
    }
    }
  }
}
#endif
