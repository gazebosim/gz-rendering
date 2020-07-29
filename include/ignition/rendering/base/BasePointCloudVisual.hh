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
#ifndef IGNITION_RENDERING_BASEPOINTCLOUDVISUAL_HH_
#define IGNITION_RENDERING_BASEPOINTCLOUDVISUAL_HH_

#include <vector>

#include "ignition/rendering/PointCloudVisual.hh"
#include "ignition/rendering/base/BaseObject.hh"
#include "ignition/rendering/base/BaseRenderTypes.hh"
#include "ignition/rendering/Scene.hh"

namespace ignition
{
  namespace rendering
  {
    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
    /// \brief Base implementation of a PointCloudVisual
    template <class T>
    class BasePointCloudVisual :
      public virtual PointCloudVisual,
      public virtual T
    {
      // Documentation inherited
      protected: BasePointCloudVisual();

      // Documentation inherited
      public: virtual ~BasePointCloudVisual();

      // Documentation inherited
      public: virtual void PreRender() override;

      // Documentation inherited
      public: virtual void Destroy() override;

      // Documentation inherited
      public: virtual void ClearPoints() override;

      // Documentation inherited
      public: virtual void SetPoints(
              const std::vector<math::Vector3d> &_points) override;

      // Documentation inherited
      public: virtual void Update() override;

      // Documentation inherited
      public: virtual void Init() override;

     // Documentation inherited
      public: virtual unsigned int PointCount() const override;

      // Documentation inherited
      public: virtual std::vector<math::Vector3d> Points() const override;

      /// \brief Create predefined materials for lidar visual
      public: virtual void CreateMaterials();
    };

    /////////////////////////////////////////////////
    // BasePointCloudVisual
    /////////////////////////////////////////////////
    template <class T>
    BasePointCloudVisual<T>::BasePointCloudVisual()
    {
    }

    /////////////////////////////////////////////////
    template <class T>
    BasePointCloudVisual<T>::~BasePointCloudVisual()
    {
    }

    /////////////////////////////////////////////////
    template <class T>
    void BasePointCloudVisual<T>::PreRender()
    {
      T::PreRender();
    }

    /////////////////////////////////////////////////
    template <class T>
    void BasePointCloudVisual<T>::Destroy()
    {
      T::Destroy();
    }

    /////////////////////////////////////////////////
    template <class T>
    void BasePointCloudVisual<T>::ClearPoints()
    {
        // no op
    }

    /////////////////////////////////////////////////
    template <class T>
    unsigned int BasePointCloudVisual<T>::PointCount() const
    {
        return 0u;
    }

    /////////////////////////////////////////////////
    template <class T>
    std::vector<math::Vector3d> BasePointCloudVisual<T>::Points() const
    {
        std::vector<math::Vector3d> d;
        return d;
    }

    /////////////////////////////////////////////////
    template <class T>
    void BasePointCloudVisual<T>::Update()
    {
      // no op
    }

    /////////////////////////////////////////////////
    template <class T>
    void BasePointCloudVisual<T>::SetPoints(const std::vector<math::Vector3d> &)
    {
      // no op
    }

    /////////////////////////////////////////////////
    template <class T>
    void BasePointCloudVisual<T>::Init()
    {
      T::Init();
      this->CreateMaterials();
    }

    /////////////////////////////////////////////////
    template <class T>
    void BasePointCloudVisual<T>::CreateMaterials()
    {
      MaterialPtr mtl;

      if (!this->Scene()->MaterialRegistered("Lidar/BlueStrips"))
      {
        mtl = this->Scene()->CreateMaterial("Lidar/BlueStrips");
        mtl->SetAmbient(0.0, 0.0, 1.0);
        mtl->SetDiffuse(0.0, 0.0, 1.0);
        mtl->SetTransparency(0.4);
        mtl->SetCastShadows(false);
        mtl->SetReceiveShadows(false);
        mtl->SetLightingEnabled(false);
      }

      if (!this->Scene()->MaterialRegistered("Lidar/LightBlueStrips"))
      {
        mtl = this->Scene()->CreateMaterial("Lidar/LightBlueStrips");
        mtl->SetAmbient(0.5, 0.5, 1.0);
        mtl->SetDiffuse(0.5, 0.5, 1.0);
        mtl->SetEmissive(0.5, 0.5, 1.0);
        mtl->SetTransparency(0.8);
        mtl->SetCastShadows(false);
        mtl->SetReceiveShadows(false);
        mtl->SetLightingEnabled(false);
      }

      if (!this->Scene()->MaterialRegistered("Lidar/TransBlack"))
      {
        mtl = this->Scene()->CreateMaterial("Lidar/TransBlack");
        mtl->SetAmbient(0.0, 0.0, 0.0);
        mtl->SetDiffuse(0.0, 0.0, 0.0);
        mtl->SetEmissive(0.0, 0.0, 0.0);
        mtl->SetTransparency(0.7);
        mtl->SetCastShadows(false);
        mtl->SetReceiveShadows(false);
        mtl->SetLightingEnabled(false);
      }

      if (!this->Scene()->MaterialRegistered("Lidar/BlueRay"))
      {
        mtl = this->Scene()->CreateMaterial("Lidar/BlueRay");
        mtl->SetAmbient(1.0, 0.0, 1.0);
        mtl->SetDiffuse(1.0, 0.0, 1.0);
        mtl->SetEmissive(1.0, 0.0, 1.0);
        mtl->SetSpecular(1.0, 0.0, 1.0);
        mtl->SetTransparency(0.0);
        mtl->SetCastShadows(false);
        mtl->SetReceiveShadows(false);
        mtl->SetLightingEnabled(false);
        mtl->SetMetalness(0.1);
        mtl->SetReflectivity(0.2);
      }
      return;
    }
    }
  }
}
#endif
