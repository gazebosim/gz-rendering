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

#include <vector>
#include "ignition/rendering/LidarVisual.hh"
#include "ignition/rendering/base/BaseObject.hh"
#include "ignition/rendering/base/BaseRenderTypes.hh"
#include "ignition/rendering/Material.hh"

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
      public: virtual void SetPoints(std::vector<double> &_msg) override;

      // Documentation inherited
      public: virtual void Update() override;

      // Documentation inherited
      public: virtual void Init() override;

      // Documentation inherited
      public: virtual void SetMinVerticalAngle(
                  const double _minVerticalAngle) override;

      // Documentation inherited
      public: virtual double MinVerticalAngle() const override;

      // Documentation inherited
      public: virtual void SetMaxVerticalAngle(
                  const double _maxVerticalAngle) override;

      // Documentation inherited
      public: virtual double MaxVerticalAngle() const override;

      // Documentation inherited
      public: virtual void SetMinHorizontalAngle(
                  const double _minHorizontalAngle) override;

      // Documentation inherited
      public: virtual double MinHorizontalAngle() const override;

      // Documentation inherited
      public: virtual void SetMaxHorizontalAngle(
                  const double _maxHorizontalAngle) override;

      // Documentation inherited
      public: virtual double MaxHorizontalAngle() const override;

      // Documentation inherited
      public: virtual void SetVerticalRayCount(
                  const unsigned int _verticalRayCount) override;

      // Documentation inherited
      public: virtual unsigned int VerticalRayCount() const override;

      // Documentation inherited
      public: virtual void SetHorizontalRayCount(
                  const unsigned int _horizontalRayCount) override;

      // Documentation inherited
      public: virtual unsigned int HorizontalRayCount() const override;

      // Documentation inherited
      public: virtual void SetMinRange(const double _minRange) override;

      // Documentation inherited
      public: virtual double MinRange() const override;

      // Documentation inherited
      public: virtual void SetMaxRange(const double _maxRange) override;

      // Documentation inherited
      public: virtual double MaxRange() const override;

      // Documentation inherited
      public: virtual void SetVerticalAngleStep(
                  const double _verticalAngleStep) override;

      // Documentation inherited
      public: virtual double VerticalAngleStep() const override;

      // Documentation inherited
      public: virtual void SetHorizontalAngleStep(
                  const double _horizontalAngleStep) override;

      // Documentation inherited
      public: virtual double HorizontalAngleStep() const override;

      // Documentation inherited
      public: virtual void SetOffset(
                  const ignition::math::Pose3d _offset) override;

      // Documentation inherited
      public: virtual ignition::math::Pose3d  Offset() const override;

      // Documentation inherited
      public: virtual unsigned int PointCount() const override;

      /// \brief Create predefined materials for lidar visual
      public: virtual void CreateMaterials();

      /// \brief Vertical minimal angle
      protected: double minVerticalAngle = 0;

      /// \brief Vertical maximum angle
      protected: double maxVerticalAngle = 0;

      /// \brief Vertical laser count
      protected: unsigned int verticalCount = 0u;

      /// \brief Angle between two vertical rays
      protected: double verticalAngleStep = 0;

      /// \brief Horizontal minimal angle
      protected: double minHorizontalAngle = 0;

      /// \brief Horizontal maximum angle
      protected: double maxHorizontalAngle = 0;

      /// \brief Horizontal laser count
      protected: unsigned int horizontalCount = 0u;

      /// \brief Angle between two horizontal rays
      protected: double horizontalAngleStep = 0;

      /// \brief Minimum Range
      protected: double minRange = 0;

      /// \brief Maximum Range
      protected: double maxRange = 0;

      /// \brief Offset of visual
      protected: ignition::math::Pose3d offset = ignition::math::Pose3d::Zero;
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
    unsigned int BaseLidarVisual<T>::PointCount() const
    {
        return 0u;
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseLidarVisual<T>::Update()
    {
      // no op
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseLidarVisual<T>::SetPoints(std::vector<double> &)
    {
      // no op
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseLidarVisual<T>::Init()
    {
      T::Init();
      this->CreateMaterials();
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseLidarVisual<T>::SetMinVerticalAngle(
          const double _minVerticalAngle)
    {
      this->minVerticalAngle = _minVerticalAngle;
    }

    /////////////////////////////////////////////////
    template <class T>
    double BaseLidarVisual<T>::MinVerticalAngle() const
    {
      return this->minVerticalAngle;
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseLidarVisual<T>::SetMaxVerticalAngle(
                  const double _maxVerticalAngle)
    {
      this->maxVerticalAngle = _maxVerticalAngle;
    }

    /////////////////////////////////////////////////
    template <class T>
    double BaseLidarVisual<T>::MaxVerticalAngle() const
    {
      return this->maxVerticalAngle;
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseLidarVisual<T>::SetVerticalRayCount(
          const unsigned int _verticalRayCount)
    {
      this->verticalCount = _verticalRayCount;
    }

    /////////////////////////////////////////////////
    template <class T>
    unsigned int BaseLidarVisual<T>::VerticalRayCount() const
    {
      return this->verticalCount;
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseLidarVisual<T>::SetVerticalAngleStep(
          const double _verticalAngleStep)
    {
      this->verticalAngleStep = _verticalAngleStep;
    }

    /////////////////////////////////////////////////
    template <class T>
    double BaseLidarVisual<T>::VerticalAngleStep() const
    {
      return this->verticalAngleStep;
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseLidarVisual<T>::SetMinHorizontalAngle(
          const double _minHorizontalAngle)
    {
      this->minHorizontalAngle = _minHorizontalAngle;
    }

    /////////////////////////////////////////////////
    template <class T>
    double BaseLidarVisual<T>::MinHorizontalAngle() const
    {
      return this->minHorizontalAngle;
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseLidarVisual<T>::SetMaxHorizontalAngle(
          const double _maxHorizontalAngle)
    {
      this->maxHorizontalAngle = _maxHorizontalAngle;
    }

    /////////////////////////////////////////////////
    template <class T>
    double BaseLidarVisual<T>::MaxHorizontalAngle() const
    {
      return this->maxHorizontalAngle;
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseLidarVisual<T>::SetHorizontalRayCount(
          const unsigned int _horizontalRayCount)
    {
      this->horizontalCount = _horizontalRayCount;
    }

    /////////////////////////////////////////////////
    template <class T>
    unsigned int BaseLidarVisual<T>::HorizontalRayCount() const
    {
      return this->horizontalCount;
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseLidarVisual<T>::SetHorizontalAngleStep(
          const double _horizontalAngleStep)
    {
      this->horizontalAngleStep = _horizontalAngleStep;
    }

    /////////////////////////////////////////////////
    template <class T>
    double BaseLidarVisual<T>::HorizontalAngleStep() const
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
    double BaseLidarVisual<T>::MinRange() const
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
    double BaseLidarVisual<T>::MaxRange() const
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
    ignition::math::Pose3d BaseLidarVisual<T>::Offset() const
    {
      return this->offset;
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseLidarVisual<T>::CreateMaterials()
    {
      MaterialPtr mtl;

      mtl = T::Scene()->CreateMaterial("Lidar/Blue");
      mtl->SetAmbient(0.0, 0.0, 1.0);
      mtl->SetDiffuse(0.0, 0.0, 1.0);
      mtl->SetEmissive(0.0, 0.0, 1.0);
      mtl->SetTransparency(0.6);
      mtl->SetCastShadows(false);
      mtl->SetReceiveShadows(false);
      mtl->SetLightingEnabled(false);

      mtl = T::Scene()->CreateMaterial("Lidar/LightBlue");
      mtl->SetAmbient(0.5, 0.5, 1.0);
      mtl->SetDiffuse(0.5, 0.5, 1.0);
      mtl->SetEmissive(0.5, 0.5, 1.0);
      mtl->SetTransparency(0.7);
      mtl->SetCastShadows(false);
      mtl->SetReceiveShadows(false);
      mtl->SetLightingEnabled(false);

      mtl = T::Scene()->CreateMaterial("Lidar/TransBlack");
      mtl->SetAmbient(0.0, 0.0, 0.0);
      mtl->SetDiffuse(0.0, 0.0, 0.0);
      mtl->SetEmissive(0.0, 0.0, 0.0);
      mtl->SetTransparency(0.7);
      mtl->SetCastShadows(false);
      mtl->SetReceiveShadows(false);
      mtl->SetLightingEnabled(false);

      mtl = T::Scene()->CreateMaterial("Lidar/BlueRay");
      mtl->SetAmbient(0.0, 0.0, 1.0);
      mtl->SetDiffuse(0.0, 0.0, 1.0);
      mtl->SetEmissive(0.0, 0.0, 1.0);
      mtl->SetSpecular(0.1, 0.1, 1);
      mtl->SetTransparency(0.0);
      mtl->SetCastShadows(false);
      mtl->SetReceiveShadows(false);
      mtl->SetLightingEnabled(false);
      return;
    }
    }
  }
}
#endif
