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
#ifndef _IGNITION_RENDERING_SCENE_HH_
#define _IGNITION_RENDERING_SCENE_HH_

#include <string>
#include <limits>
#include "gazebo/common/Color.hh"
#include "gazebo/common/Material.hh"
#include "gazebo/common/Time.hh"
#include "ignition/rendering/MeshDescriptor.hh"
#include "ignition/rendering/RenderTypes.hh"
#include "ignition/rendering/Storage.hh"
#include "ignition/rendering/Util.hh"

namespace ignition
{
  namespace rendering
  {
    class RenderEngine;

    class IGNITION_VISIBLE Scene
    {
      public: virtual ~Scene() { }

      public: virtual void Load() = 0;

      public: virtual void Init() = 0;

      public: virtual void Fini() = 0;

      public: virtual bool IsInitialized() const = 0;

      public: virtual unsigned int GetId() const = 0;

      public: virtual std::string GetName() const = 0;

      public: virtual RenderEngine *GetEngine() const = 0;

      public: virtual gazebo::common::Time GetSimTime() const = 0;

      public: virtual void SetSimTime(const gazebo::common::Time &_time) = 0;

      public: virtual VisualPtr GetRootVisual() const = 0;

      public: virtual gazebo::common::Color GetAmbientLight() const = 0;

      public: virtual void SetAmbientLight(const gazebo::common::Color &_color) = 0;

      public: virtual gazebo::common::Color GetBackgroundColor() const = 0;

      public: virtual void SetBackgroundColor(const gazebo::common::Color &_color) = 0;

      public: virtual unsigned int GetNodeCount() const = 0;

      public: virtual bool HasNode(ConstNodePtr _node) const = 0;

      public: virtual bool HasNodeId(unsigned int _id) const = 0;

      public: virtual bool HasNodeName(const std::string &_name) const = 0;

      public: virtual NodePtr GetNodeById(unsigned int _id) const = 0;

      public: virtual NodePtr GetNodeByName(const std::string &_name) const = 0;

      public: virtual NodePtr GetNodeByIndex(unsigned int _index) const = 0;

      public: virtual void DestroyNode(NodePtr _node) = 0;

      public: virtual void DestroyNodeById(unsigned int _id) = 0;

      public: virtual void DestroyNodeByName(const std::string &_name) = 0;

      public: virtual void DestroyNodeByIndex(unsigned int _index) = 0;

      public: virtual void DestroyNodes() = 0;

      public: virtual unsigned int GetLightCount() const = 0;

      public: virtual bool HasLight(ConstLightPtr _light) const = 0;

      public: virtual bool HasLightId(unsigned int _id) const = 0;

      public: virtual bool HasLightName(const std::string &_name) const = 0;

      public: virtual LightPtr GetLightById(unsigned int _id) const = 0;

      public: virtual LightPtr GetLightByName(
                  const std::string &_name) const = 0;

      public: virtual LightPtr GetLightByIndex(unsigned int _index) const = 0;

      public: virtual void DestroyLight(LightPtr _light) = 0;

      public: virtual void DestroyLightById(unsigned int _id) = 0;

      public: virtual void DestroyLightByName(const std::string &_name) = 0;

      public: virtual void DestroyLightByIndex(unsigned int _index) = 0;

      public: virtual void DestroyLights() = 0;

      public: virtual unsigned int GetSensorCount() const = 0;

      public: virtual bool HasSensor(ConstSensorPtr _sensor) const = 0;

      public: virtual bool HasSensorId(unsigned int _id) const = 0;

      public: virtual bool HasSensorName(const std::string &_name) const = 0;

      public: virtual SensorPtr GetSensorById(unsigned int _id) const = 0;

      public: virtual SensorPtr GetSensorByName(
                  const std::string &_name) const = 0;

      public: virtual SensorPtr GetSensorByIndex(unsigned int _index) const = 0;

      public: virtual void DestroySensor(SensorPtr _sensor) = 0;

      public: virtual void DestroySensorById(unsigned int _id) = 0;

      public: virtual void DestroySensorByName(const std::string &_name) = 0;

      public: virtual void DestroySensorByIndex(unsigned int _index) = 0;

      public: virtual void DestroySensors() = 0;

      public: virtual unsigned int GetVisualCount() const = 0;

      public: virtual bool HasVisual(ConstVisualPtr _visual) const = 0;

      public: virtual bool HasVisualId(unsigned int _id) const = 0;

      public: virtual bool HasVisualName(const std::string &_name) const = 0;

      public: virtual VisualPtr GetVisualById(unsigned int _id) const = 0;

      public: virtual VisualPtr GetVisualByName(
                  const std::string &_name) const = 0;

      public: virtual VisualPtr GetVisualByIndex(unsigned int _index) const = 0;

      public: virtual void DestroyVisual(VisualPtr _visual) = 0;

      public: virtual void DestroyVisualById(unsigned int _id) = 0;

      public: virtual void DestroyVisualByName(const std::string &_name) = 0;

      public: virtual void DestroyVisualByIndex(unsigned int _index) = 0;

      public: virtual void DestroyVisuals() = 0;

      public: virtual bool MaterialRegistered(
                  const std::string &_name) const = 0;

      public: virtual MaterialPtr GetMaterial(
                  const std::string &_name) const = 0;

      public: virtual void RegisterMaterial(const std::string &_name,
                  MaterialPtr _material) = 0;

      public: virtual void UnregisterMaterial(const std::string &_name) = 0;

      public: virtual void UnregisterMaterials() = 0;

      public: virtual DirectionalLightPtr CreateDirectionalLight() = 0;

      public: virtual DirectionalLightPtr CreateDirectionalLight(
                  unsigned int _id) = 0;

      public: virtual DirectionalLightPtr CreateDirectionalLight(
                  const std::string &_name) = 0;

      public: virtual DirectionalLightPtr CreateDirectionalLight(
                  unsigned int _id, const std::string &_name) = 0;

      public: virtual PointLightPtr CreatePointLight() = 0;

      public: virtual PointLightPtr CreatePointLight(unsigned int _id) = 0;

      public: virtual PointLightPtr CreatePointLight(
                  const std::string &_name) = 0;

      public: virtual PointLightPtr CreatePointLight(unsigned int _id,
                  const std::string &_name) = 0;

      public: virtual SpotLightPtr CreateSpotLight() = 0;

      public: virtual SpotLightPtr CreateSpotLight(unsigned int _id) = 0;

      public: virtual SpotLightPtr CreateSpotLight(
                  const std::string &_name) = 0;

      public: virtual SpotLightPtr CreateSpotLight(unsigned int _id,
                  const std::string &_name) = 0;

      public: virtual CameraPtr CreateCamera() = 0;

      public: virtual CameraPtr CreateCamera(unsigned int _id) = 0;

      public: virtual CameraPtr CreateCamera(const std::string &_name) = 0;

      public: virtual CameraPtr CreateCamera(unsigned int _id,
                  const std::string &_name) = 0;

      public: virtual VisualPtr CreateVisual() = 0;

      public: virtual VisualPtr CreateVisual(unsigned int _id) = 0;

      public: virtual VisualPtr CreateVisual(const std::string &_name) = 0;

      public: virtual VisualPtr CreateVisual(unsigned int _id,
                  const std::string &_name) = 0;

      public: virtual ArrowVisualPtr CreateArrowVisual() = 0;

      public: virtual ArrowVisualPtr CreateArrowVisual(unsigned int _id) = 0;

      public: virtual ArrowVisualPtr CreateArrowVisual(
                  const std::string &_name) = 0;

      public: virtual ArrowVisualPtr CreateArrowVisual(unsigned int _id,
                  const std::string &_name) = 0;

      public: virtual AxisVisualPtr CreateAxisVisual() = 0;

      public: virtual AxisVisualPtr CreateAxisVisual(unsigned int _id) = 0;

      public: virtual AxisVisualPtr CreateAxisVisual(
                  const std::string &_name) = 0;

      public: virtual AxisVisualPtr CreateAxisVisual(unsigned int _id,
                  const std::string &_name) = 0;

      public: virtual GeometryPtr CreateBox() = 0;

      public: virtual GeometryPtr CreateCone() = 0;

      public: virtual GeometryPtr CreateCylinder() = 0;

      public: virtual GeometryPtr CreatePlane() = 0;

      public: virtual GeometryPtr CreateSphere() = 0;

      public: virtual MeshPtr CreateMesh(const std::string &_meshName) = 0;

      public: virtual MeshPtr CreateMesh(const gazebo::common::Mesh *_mesh) = 0;

      public: virtual MeshPtr CreateMesh(const MeshDescriptor &_desc) = 0;

      public: virtual MaterialPtr CreateMaterial() = 0;

      public: virtual MaterialPtr CreateMaterial(
                  const gazebo::common::Material &_material) = 0;

      public: virtual void PreRender() = 0;

      public: virtual void Clear() = 0;

      public: virtual void Destroy() = 0;
    };
  }
}
#endif
