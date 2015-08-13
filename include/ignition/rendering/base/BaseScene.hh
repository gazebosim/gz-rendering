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
#ifndef _IGNITION_RENDERING_BASESCENE_HH_
#define _IGNITION_RENDERING_BASESCENE_HH_

#include <boost/enable_shared_from_this.hpp>
#include "ignition/rendering/Scene.hh"
#include "ignition/rendering/base/BaseRenderTypes.hh"

namespace ignition
{
  namespace rendering
  {
    class IGNITION_VISIBLE BaseScene :
      public boost::enable_shared_from_this<BaseScene>,
      public virtual Scene
    {
      protected: BaseScene(unsigned int _id, const std::string &_name);

      public: virtual ~BaseScene();

      public: virtual void Load();

      public: virtual void Init();

      public: virtual void Fini();

      public: virtual bool IsLoaded() const;

      public: virtual bool IsInitialized() const;

      public: virtual unsigned int GetId() const;

      public: virtual std::string GetName() const;

      public: virtual gazebo::common::Time GetSimTime() const;

      public: virtual void SetSimTime(const gazebo::common::Time &_time);

      public: virtual void SetAmbientLight(double _r, double _g, double _b,
                  double _a = 1.0);

      public: virtual void SetAmbientLight(const gazebo::common::Color &_color) = 0;

      public: virtual void SetBackgroundColor(double _r, double _g, double _b,
                  double _a = 1.0);

      public: virtual void SetBackgroundColor(const gazebo::common::Color &_color) = 0;

      public: virtual unsigned int GetNodeCount() const;

      public: virtual bool HasNode(ConstNodePtr _node) const;

      public: virtual bool HasNodeId(unsigned int _id) const;

      public: virtual bool HasNodeName(const std::string &_name) const;

      public: virtual NodePtr GetNodeById(unsigned int _id) const;

      public: virtual NodePtr GetNodeByName(const std::string &_name) const;

      public: virtual NodePtr GetNodeByIndex(unsigned int _index) const;

      public: virtual void DestroyNode(NodePtr _node);

      public: virtual void DestroyNodeById(unsigned int _id);

      public: virtual void DestroyNodeByName(const std::string &_name);

      public: virtual void DestroyNodeByIndex(unsigned int _index);

      public: virtual void DestroyNodes();

      public: virtual unsigned int GetLightCount() const;

      public: virtual bool HasLight(ConstLightPtr _light) const;

      public: virtual bool HasLightId(unsigned int _id) const;

      public: virtual bool HasLightName(const std::string &_name) const;

      public: virtual LightPtr GetLightById(unsigned int _id) const;

      public: virtual LightPtr GetLightByName(const std::string &_name) const;

      public: virtual LightPtr GetLightByIndex(unsigned int _index) const;

      public: virtual void DestroyLight(LightPtr _light);

      public: virtual void DestroyLightById(unsigned int _id);

      public: virtual void DestroyLightByName(const std::string &_name);

      public: virtual void DestroyLightByIndex(unsigned int _index);

      public: virtual void DestroyLights();

      public: virtual unsigned int GetSensorCount() const;

      public: virtual bool HasSensor(ConstSensorPtr _sensor) const;

      public: virtual bool HasSensorId(unsigned int _id) const;

      public: virtual bool HasSensorName(const std::string &_name) const;

      public: virtual SensorPtr GetSensorById(unsigned int _id) const;

      public: virtual SensorPtr GetSensorByName(const std::string &_name) const;

      public: virtual SensorPtr GetSensorByIndex(unsigned int _index) const;

      public: virtual void DestroySensor(SensorPtr _sensor);

      public: virtual void DestroySensorById(unsigned int _id);

      public: virtual void DestroySensorByName(const std::string &_name);

      public: virtual void DestroySensorByIndex(unsigned int _index);

      public: virtual void DestroySensors();

      public: virtual unsigned int GetVisualCount() const;

      public: virtual bool HasVisual(ConstVisualPtr _visual) const;

      public: virtual bool HasVisualId(unsigned int _id) const;

      public: virtual bool HasVisualName(const std::string &_name) const;

      public: virtual VisualPtr GetVisualById(unsigned int _id) const;

      public: virtual VisualPtr GetVisualByName(const std::string &_name) const;

      public: virtual VisualPtr GetVisualByIndex(unsigned int _index) const;

      public: virtual void DestroyVisual(VisualPtr _visual);

      public: virtual void DestroyVisualById(unsigned int _id);

      public: virtual void DestroyVisualByName(const std::string &_name);

      public: virtual void DestroyVisualByIndex(unsigned int _index);

      public: virtual void DestroyVisuals();

      public: virtual bool MaterialRegistered(const std::string &_name) const;

      public: virtual MaterialPtr GetMaterial(const std::string &_name) const;

      public: virtual void RegisterMaterial(const std::string &_name,
                  MaterialPtr _material);

      public: virtual void UnregisterMaterial(const std::string &_name);

      public: virtual void UnregisterMaterials();

      public: virtual DirectionalLightPtr CreateDirectionalLight();

      public: virtual DirectionalLightPtr CreateDirectionalLight(
                  unsigned int _id);

      public: virtual DirectionalLightPtr CreateDirectionalLight(
                  const std::string &_name);

      public: virtual DirectionalLightPtr CreateDirectionalLight(
                  unsigned int _id, const std::string &_name);

      public: virtual PointLightPtr CreatePointLight();

      public: virtual PointLightPtr CreatePointLight(unsigned int _id);

      public: virtual PointLightPtr CreatePointLight(const std::string &_name);

      public: virtual PointLightPtr CreatePointLight(unsigned int _id,
                  const std::string &_name);

      public: virtual SpotLightPtr CreateSpotLight();

      public: virtual SpotLightPtr CreateSpotLight(unsigned int _id);

      public: virtual SpotLightPtr CreateSpotLight(const std::string &_name);

      public: virtual SpotLightPtr CreateSpotLight(unsigned int _id,
                  const std::string &_name);

      public: virtual CameraPtr CreateCamera();

      public: virtual CameraPtr CreateCamera(unsigned int _id);

      public: virtual CameraPtr CreateCamera(const std::string &_name);

      public: virtual CameraPtr CreateCamera(unsigned int _id,
                  const std::string &_name);

      public: virtual VisualPtr CreateVisual();

      public: virtual VisualPtr CreateVisual(unsigned int _id);

      public: virtual VisualPtr CreateVisual(const std::string &_name);

      public: virtual VisualPtr CreateVisual(unsigned int _id,
                  const std::string &_name);

      public: virtual ArrowVisualPtr CreateArrowVisual();

      public: virtual ArrowVisualPtr CreateArrowVisual(unsigned int _id);

      public: virtual ArrowVisualPtr CreateArrowVisual(
                  const std::string &_name);

      public: virtual ArrowVisualPtr CreateArrowVisual(unsigned int _id,
                  const std::string &_name);

      public: virtual AxisVisualPtr CreateAxisVisual();

      public: virtual AxisVisualPtr CreateAxisVisual(unsigned int _id);

      public: virtual AxisVisualPtr CreateAxisVisual(const std::string &_name);

      public: virtual AxisVisualPtr CreateAxisVisual(unsigned int _id,
                  const std::string &_name);

      public: virtual GeometryPtr CreateBox();

      public: virtual GeometryPtr CreateCone();

      public: virtual GeometryPtr CreateCylinder();

      public: virtual GeometryPtr CreatePlane();

      public: virtual GeometryPtr CreateSphere();

      public: virtual MeshPtr CreateMesh(const std::string &_meshName);

      public: virtual MeshPtr CreateMesh(const gazebo::common::Mesh *_mesh);

      public: virtual MeshPtr CreateMesh(const MeshDescriptor &_desc);

      public: virtual MaterialPtr CreateMaterial();

      public: virtual MaterialPtr CreateMaterial(
                  const gazebo::common::Material &_material);

      public: virtual RenderTexturePtr CreateRenderTexture();

      public: virtual void PreRender();

      public: virtual void Clear();

      public: virtual void Destroy();

      protected: virtual unsigned int CreateObjectId();

      protected: virtual std::string CreateObjectName(unsigned int _id,
                  const std::string &_prefix);

      protected: virtual bool RegisterLight(LightPtr _light);

      protected: virtual bool RegisterSensor(SensorPtr _vensor);

      protected: virtual bool RegisterVisual(VisualPtr _visual);

      protected: virtual DirectionalLightPtr CreateDirectionalLightImpl(
                  unsigned int _id, const std::string &_name) = 0;

      protected: virtual PointLightPtr CreatePointLightImpl(unsigned int _id,
                     const std::string &_name) = 0;

      protected: virtual SpotLightPtr CreateSpotLightImpl(unsigned int _id,
                     const std::string &_name) = 0;

      protected: virtual CameraPtr CreateCameraImpl(unsigned int _id,
                     const std::string &_name) = 0;

      protected: virtual VisualPtr CreateVisualImpl(unsigned int _id,
                     const std::string &_name) = 0;

      protected: virtual ArrowVisualPtr CreateArrowVisualImpl(unsigned int _id,
                     const std::string &_name) = 0;

      protected: virtual AxisVisualPtr CreateAxisVisualImpl(unsigned int _id,
                     const std::string &_name) = 0;

      protected: virtual GeometryPtr CreateBoxImpl(unsigned int _id,
                     const std::string &_name) = 0;

      protected: virtual GeometryPtr CreateConeImpl(unsigned int _id,
                     const std::string &_name) = 0;

      protected: virtual GeometryPtr CreateCylinderImpl(unsigned int _id,
                     const std::string &_name) = 0;

      protected: virtual GeometryPtr CreatePlaneImpl(unsigned int _id,
                     const std::string &_name) = 0;

      protected: virtual GeometryPtr CreateSphereImpl(unsigned int _id,
                     const std::string &_name) = 0;

      protected: virtual MeshPtr CreateMeshImpl(unsigned int _id,
                     const std::string &_name, const MeshDescriptor &_desc) = 0;

      protected: virtual MaterialPtr CreateMaterialImpl(unsigned int _id,
                     const std::string &_name) = 0;

      protected: virtual RenderTexturePtr CreateRenderTextureImpl(
                     unsigned int _id, const std::string &_name) = 0;

      protected: virtual LightStorePtr GetLights() const = 0;

      protected: virtual SensorStorePtr GetSensors() const = 0;

      protected: virtual VisualStorePtr GetVisuals() const = 0;

      protected: virtual MaterialMapPtr GetMaterials() const = 0;

      protected: virtual bool LoadImpl() = 0;

      protected: virtual bool InitImpl() = 0;

      private: virtual void CreateNodeStore();

      protected: unsigned int id;

      protected: std::string name;

      protected: gazebo::common::Time simTime;

      protected: bool loaded;

      protected: bool initialized;

      private: unsigned int nextObjectId;

      private: NodeStorePtr nodes;
    };
  }
}
#endif
