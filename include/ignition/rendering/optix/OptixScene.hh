/*
 * Copyright (C) 2015 Open Source Robotics Foundation
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
#ifndef _IGNITION_RENDERING_OPTIXSCENE_HH_
#define _IGNITION_RENDERING_OPTIXSCENE_HH_

#include "ignition/rendering/base/BaseScene.hh"
#include "ignition/rendering/optix/OptixRenderTypes.hh"
#include "ignition/rendering/optix/OptixIncludes.hh"

namespace ignition
{
  namespace rendering
  {
    class IGNITION_VISIBLE OptixScene :
      public BaseScene
    {
      protected: OptixScene(unsigned int _id, const std::string &_name);

      public: virtual ~OptixScene();

      public: virtual void Fini();

      public: virtual RenderEngine *GetEngine() const;

      public: virtual VisualPtr GetRootVisual() const;

      public: virtual math::Color GetAmbientLight() const;

      public: virtual void SetAmbientLight(const math::Color &_color);

      public: virtual math::Color GetBackgroundColor() const;

      public: virtual void SetBackgroundColor(const math::Color &_color);

      public: virtual void PreRender();

      public: virtual void Clear();

      public: virtual void Destroy();

      public: virtual OptixLightManagerPtr GetLightManager() const;

      public: virtual optix::Context GetOptixContext() const;

      public: virtual optix::Program CreateOptixProgram(
                  const std::string &_fileBase, const std::string &_function);

      protected: virtual bool LoadImpl();

      protected: virtual bool InitImpl();

      protected: virtual DirectionalLightPtr CreateDirectionalLightImpl(
                     unsigned int _id, const std::string &_name);

      protected: virtual PointLightPtr CreatePointLightImpl(unsigned int _id,
                     const std::string &_name);

      protected: virtual SpotLightPtr CreateSpotLightImpl(unsigned int _id,
                     const std::string &_name);

      protected: virtual CameraPtr CreateCameraImpl(unsigned int _id,
                     const std::string &_name);

      protected: virtual VisualPtr CreateVisualImpl(unsigned int _id,
                     const std::string &_name);

      protected: virtual ArrowVisualPtr CreateArrowVisualImpl(unsigned int _id,
                     const std::string &_name);

      protected: virtual AxisVisualPtr CreateAxisVisualImpl(unsigned int _id,
                     const std::string &_name);

      protected: virtual GeometryPtr CreateBoxImpl(unsigned int _id,
                     const std::string &_name);

      protected: virtual GeometryPtr CreateConeImpl(unsigned int _id,
                     const std::string &_name);

      protected: virtual GeometryPtr CreateCylinderImpl(unsigned int _id,
                     const std::string &_name);

      protected: virtual GeometryPtr CreatePlaneImpl(unsigned int _id,
                     const std::string &_name);

      protected: virtual GeometryPtr CreateSphereImpl(unsigned int _id,
                     const std::string &_name);

      protected: virtual MeshPtr CreateMeshImpl(unsigned int _id,
                     const std::string &_name, const std::string &_meshName);

      protected: virtual MeshPtr CreateMeshImpl(unsigned int _id,
                     const std::string &_name, const MeshDescriptor &_desc);

      protected: virtual MaterialPtr CreateMaterialImpl(unsigned int _id,
                     const std::string &_name);

      protected: virtual RenderTexturePtr CreateRenderTextureImpl(
                     unsigned int _id, const std::string &_name);

      protected: virtual bool InitObject(OptixObjectPtr _object,
                     unsigned int _id, const std::string &_name);

      protected: virtual LightStorePtr GetLights() const;

      protected: virtual SensorStorePtr GetSensors() const;

      protected: virtual VisualStorePtr GetVisuals() const;

      protected: virtual MaterialMapPtr GetMaterials() const;

      protected: virtual unsigned int GetNextEntryId();

      private: void CreateContext();

      private: void CreateRootVisual();

      private: void CreateLightManager();

      private: void CreateMeshFactory();

      private: void CreateStores();

      private: OptixScenePtr SharedThis();

      protected: OptixVisualPtr rootVisual;

      protected: OptixLightManagerPtr lightManager;

      protected: OptixMeshFactoryPtr meshFactory;

      protected: math::Color backgroundColor;

      protected: OptixLightStorePtr lights;

      protected: OptixSensorStorePtr sensors;

      protected: OptixVisualStorePtr visuals;

      protected: OptixMaterialMapPtr materials;

      protected: optix::Context optixContext;

      protected: optix::Group optixRootGroup;

      protected: optix::Acceleration optixRootAccel;

      protected: optix::Program optixMissProgram;

      protected: optix::Geometry optixBoxGeometry;

      protected: optix::Geometry optixConeGeometry;

      protected: optix::Geometry optixCylinderGeometry;

      protected: optix::Geometry optixSphereGeometry;

      protected: math::Color ambientLight;

      private: friend class OptixRenderEngine;
    };
  }
}
#endif
