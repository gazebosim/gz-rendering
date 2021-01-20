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
#ifndef IGNITION_RENDERING_BASE_BASESCENE_HH_
#define IGNITION_RENDERING_BASE_BASESCENE_HH_

#include <array>
#include <set>
#include <string>

#include <ignition/common/Console.hh>
#include <ignition/common/SuppressWarning.hh>

#include "ignition/rendering/RenderEngine.hh"
#include "ignition/rendering/Scene.hh"
#include "ignition/rendering/base/BaseRenderTypes.hh"

namespace ignition
{
  namespace rendering
  {
    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
    //
    class IGNITION_RENDERING_VISIBLE BaseScene :
      IGN_COMMON_WARN_IGNORE__DLL_INTERFACE_MISSING
      public std::enable_shared_from_this<BaseScene>,
      IGN_COMMON_WARN_RESUME__DLL_INTERFACE_MISSING
      public virtual Scene
    {
      protected: BaseScene(unsigned int _id, const std::string &_name);

      public: virtual ~BaseScene();

      public: virtual void Load() override;

      public: virtual void Init() override;

      public: virtual void Fini() override;

      public: virtual bool IsLoaded() const;

      public: virtual bool IsInitialized() const override;

      public: virtual unsigned int Id() const override;

      public: virtual std::string Name() const override;

      public: virtual common::Time SimTime() const override;

      public: virtual void SetSimTime(const common::Time &_time) override;

      public: virtual void SetAmbientLight(double _r, double _g, double _b,
                  double _a = 1.0) override;

      public: virtual void SetAmbientLight(const math::Color &_color)
                      override = 0;

      public: virtual math::Color BackgroundColor() const override;

      public: virtual void SetBackgroundColor(double _r, double _g, double _b,
                  double _a = 1.0) override;

      public: virtual void SetBackgroundColor(const math::Color &_color)
                      override;

      // Documentation inherited.
      public: virtual bool IsGradientBackgroundColor() const override;

      // Documentation inherited.
      public: virtual std::array<math::Color, 4> GradientBackgroundColor()
                  const override;

      // Documentation inherited.
      public: virtual void SetGradientBackgroundColor(
                  const std::array<math::Color, 4> &_colors) override;

      // Documentation inherited.
      public: virtual void RemoveGradientBackgroundColor() override;

      public: virtual unsigned int NodeCount() const override;

      public: virtual bool HasNode(ConstNodePtr _node) const override;

      public: virtual bool HasNodeId(unsigned int _id) const override;

      public: virtual bool HasNodeName(const std::string &_name) const override;

      public: virtual NodePtr NodeById(unsigned int _id) const override;

      public: virtual NodePtr NodeByName(const std::string &_name) const
                      override;

      public: virtual NodePtr NodeByIndex(unsigned int _index) const override;

      // Documentation inherited.
      public: virtual void DestroyNode(NodePtr _node, bool _recursive = false)
                      override;

      public: virtual void DestroyNodeById(unsigned int _id) override;

      public: virtual void DestroyNodeByName(const std::string &_name) override;

      public: virtual void DestroyNodeByIndex(unsigned int _index) override;

      public: virtual void DestroyNodes() override;

      public: virtual unsigned int LightCount() const override;

      public: virtual bool HasLight(ConstLightPtr _light) const override;

      public: virtual bool HasLightId(unsigned int _id) const override;

      public: virtual bool HasLightName(const std::string &_name) const
                      override;

      public: virtual LightPtr LightById(unsigned int _id) const override;

      public: virtual LightPtr LightByName(const std::string &_name) const
                      override;

      public: virtual LightPtr LightByIndex(unsigned int _index) const override;

      // Documentation inherited.
      public: virtual void DestroyLight(LightPtr _light,
          bool _recursive = false) override;

      public: virtual void DestroyLightById(unsigned int _id) override;

      public: virtual void DestroyLightByName(const std::string &_name)
                      override;

      public: virtual void DestroyLightByIndex(unsigned int _index) override;

      public: virtual void DestroyLights() override;

      public: virtual unsigned int SensorCount() const override;

      public: virtual bool HasSensor(ConstSensorPtr _sensor) const override;

      public: virtual bool HasSensorId(unsigned int _id) const override;

      public: virtual bool HasSensorName(const std::string &_name) const
                      override;

      public: virtual SensorPtr SensorById(unsigned int _id) const override;

      public: virtual SensorPtr SensorByName(const std::string &_name) const
                      override;

      public: virtual SensorPtr SensorByIndex(unsigned int _index) const
                      override;

      // Documentation inherited.
      public: virtual void DestroySensor(SensorPtr _sensor,
          bool _recursive = false) override;

      public: virtual void DestroySensorById(unsigned int _id) override;

      public: virtual void DestroySensorByName(const std::string &_name)
                      override;

      public: virtual void DestroySensorByIndex(unsigned int _index) override;

      public: virtual void DestroySensors() override;

      public: virtual unsigned int VisualCount() const override;

      public: virtual bool HasVisual(ConstVisualPtr _visual) const override;

      public: virtual bool HasVisualId(unsigned int _id) const override;

      public: virtual bool HasVisualName(const std::string &_name) const
                      override;

      public: virtual VisualPtr VisualById(unsigned int _id) const override;

      public: virtual VisualPtr VisualByName(const std::string &_name) const
                      override;

      public: virtual VisualPtr VisualByIndex(unsigned int _index) const
                      override;

      // Documentation inherited
      public: virtual VisualPtr VisualAt(const CameraPtr &_camera,
                          const ignition::math::Vector2i &_mousePos) override;

      // Documentation inherited.
      public: virtual void DestroyVisual(VisualPtr _visual,
          bool _recursive = false) override;

      public: virtual void DestroyVisualById(unsigned int _id) override;

      public: virtual void DestroyVisualByName(const std::string &_name)
                      override;

      public: virtual void DestroyVisualByIndex(unsigned int _index) override;

      public: virtual void DestroyVisuals() override;

      public: virtual bool MaterialRegistered(const std::string &_name) const
                      override;

      public: virtual MaterialPtr Material(const std::string &_name) const
                      override;

      public: virtual void RegisterMaterial(const std::string &_name,
                  MaterialPtr _material) override;

      public: virtual void UnregisterMaterial(const std::string &_name)
                      override;

      public: virtual void UnregisterMaterials() override;

      // Documentation inherited
      public: virtual void DestroyMaterial(MaterialPtr _material) override;

      // Documentation inherited
      public: virtual void DestroyMaterials() override;

      public: virtual DirectionalLightPtr CreateDirectionalLight() override;

      public: virtual DirectionalLightPtr CreateDirectionalLight(
                  unsigned int _id) override;

      public: virtual DirectionalLightPtr CreateDirectionalLight(
                  const std::string &_name) override;

      public: virtual DirectionalLightPtr CreateDirectionalLight(
                  unsigned int _id, const std::string &_name) override;

      public: virtual PointLightPtr CreatePointLight() override;

      public: virtual PointLightPtr CreatePointLight(unsigned int _id) override;

      public: virtual PointLightPtr CreatePointLight(const std::string &_name)
                      override;

      public: virtual PointLightPtr CreatePointLight(unsigned int _id,
                  const std::string &_name) override;

      public: virtual SpotLightPtr CreateSpotLight() override;

      public: virtual SpotLightPtr CreateSpotLight(unsigned int _id) override;

      public: virtual SpotLightPtr CreateSpotLight(const std::string &_name)
                      override;

      public: virtual SpotLightPtr CreateSpotLight(unsigned int _id,
                  const std::string &_name) override;

      public: virtual CameraPtr CreateCamera() override;

      public: virtual CameraPtr CreateCamera(unsigned int _id) override;

      public: virtual CameraPtr CreateCamera(const std::string &_name) override;

      public: virtual CameraPtr CreateCamera(unsigned int _id,
                  const std::string &_name) override;

      public: virtual DepthCameraPtr CreateDepthCamera() override;

      public: virtual DepthCameraPtr CreateDepthCamera(const unsigned int _id)
                      override;

      public: virtual DepthCameraPtr CreateDepthCamera(
                  const std::string &_name) override;

      public: virtual DepthCameraPtr CreateDepthCamera(const unsigned int _id,
                  const std::string &_name) override;

      // Documentation inherited.
      public: virtual ThermalCameraPtr CreateThermalCamera() override;

      // Documentation inherited.
      public: virtual ThermalCameraPtr CreateThermalCamera(
                  const unsigned int _id) override;

      // Documentation inherited.
      public: virtual ThermalCameraPtr CreateThermalCamera(
                  const std::string &_name) override;

      // Documentation inherited.
      public: virtual ThermalCameraPtr CreateThermalCamera(
                  const unsigned int _id, const std::string &_name) override;

      // Documentation inherited.
      public: virtual GpuRaysPtr CreateGpuRays() override;

      // Documentation inherited.
      public: virtual GpuRaysPtr CreateGpuRays(const unsigned int _id) override;

      // Documentation inherited.
      public: virtual GpuRaysPtr CreateGpuRays(
                  const std::string &_name) override;

      // Documentation inherited.
      public: virtual GpuRaysPtr CreateGpuRays(const unsigned int _id,
                  const std::string &_name) override;

      public: virtual VisualPtr CreateVisual() override;

      public: virtual VisualPtr CreateVisual(unsigned int _id) override;

      public: virtual VisualPtr CreateVisual(const std::string &_name) override;

      public: virtual VisualPtr CreateVisual(unsigned int _id,
                  const std::string &_name) override;

      public: virtual ArrowVisualPtr CreateArrowVisual() override;

      public: virtual ArrowVisualPtr CreateArrowVisual(unsigned int _id)
                      override;

      public: virtual ArrowVisualPtr CreateArrowVisual(
                  const std::string &_name) override;

      public: virtual ArrowVisualPtr CreateArrowVisual(unsigned int _id,
                  const std::string &_name) override;

      public: virtual AxisVisualPtr CreateAxisVisual() override;

      public: virtual AxisVisualPtr CreateAxisVisual(unsigned int _id) override;

      public: virtual AxisVisualPtr CreateAxisVisual(const std::string &_name)
                      override;

      public: virtual AxisVisualPtr CreateAxisVisual(unsigned int _id,
                  const std::string &_name) override;


      // Documentation inherited
      public: virtual GizmoVisualPtr CreateGizmoVisual() override;

      // Documentation inherited
      public: virtual GizmoVisualPtr CreateGizmoVisual(unsigned int _id)
                      override;

      // Documentation inherited
      public: virtual GizmoVisualPtr CreateGizmoVisual(const std::string &_name)
                      override;

      // Documentation inherited
      public: virtual GizmoVisualPtr CreateGizmoVisual(unsigned int _id,
                  const std::string &_name) override;


      public: virtual GeometryPtr CreateBox() override;

      public: virtual GeometryPtr CreateCone() override;

      public: virtual GeometryPtr CreateCylinder() override;

      public: virtual GeometryPtr CreatePlane() override;

      public: virtual GeometryPtr CreateSphere() override;

      public: virtual MeshPtr CreateMesh(const std::string &_meshName) override;

      public: virtual MeshPtr CreateMesh(const common::Mesh *_mesh) override;

      public: virtual MeshPtr CreateMesh(const MeshDescriptor &_desc) override;

      // Documentation inherited.
      public: virtual GridPtr CreateGrid() override;

      // Documentation inherited.
      public: virtual MarkerPtr CreateMarker() override;

      // Documentation inherited.
      public: virtual TextPtr CreateText() override;

      public: virtual MaterialPtr CreateMaterial(const std::string &_name = "")
                      override;

      public: virtual MaterialPtr CreateMaterial(
                  const common::Material &_material) override;

      public: virtual RenderTexturePtr CreateRenderTexture() override;

      // Documentation inherited.
      public: virtual RenderWindowPtr CreateRenderWindow() override;

      public: virtual RayQueryPtr CreateRayQuery() override;

      public: virtual void PreRender() override;

      public: virtual void Clear() override;

      public: virtual void Destroy() override;

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

      protected: virtual DepthCameraPtr CreateDepthCameraImpl(unsigned int _id,
                     const std::string &_name) = 0;

      /// \brief Implementation for creating a thermal camera.
      /// \param[in] _id Unique id
      /// \param[in] _name Name of thermal camera
      protected: virtual ThermalCameraPtr CreateThermalCameraImpl(
                     unsigned int /*_id*/, const std::string &/*_name*/)
                 {
                   ignerr << "Thermal camera not supported by: "
                          << this->Engine()->Name() << std::endl;
                   return ThermalCameraPtr();
                 }

      /// \brief Implementation for creating GpuRays sensor.
      /// \param[in] _id Unique id
      /// \param[in] _name Name of GpuRays sensor
      protected: virtual GpuRaysPtr CreateGpuRaysImpl(unsigned int /*_id*/,
                     const std::string & /*_name*/)
                 {
                   ignerr << "GpuRays not supported by: "
                          << this->Engine()->Name() << std::endl;
                   return GpuRaysPtr();
                 }

      protected: virtual VisualPtr CreateVisualImpl(unsigned int _id,
                     const std::string &_name) = 0;

      protected: virtual ArrowVisualPtr CreateArrowVisualImpl(unsigned int _id,
                     const std::string &_name) = 0;

      protected: virtual AxisVisualPtr CreateAxisVisualImpl(unsigned int _id,
                     const std::string &_name) = 0;

      /// \brief Implementation for creating a GizmoVisual.
      /// \param[in] _id Unique id
      /// \param[in] _name Name of GizmoVisual
      protected: virtual GizmoVisualPtr CreateGizmoVisualImpl(unsigned int,
                     const std::string &)
                 {
                   ignerr << "GizmoVisual not supported by: "
                          << this->Engine()->Name() << std::endl;
                   return GizmoVisualPtr();
                 }

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
                     const std::string &_name,
                     const MeshDescriptor &_desc) = 0;

      /// \brief Implementation for creating a grid geometry object
      /// \param[in] _id unique object id.
      /// \param[in] _name unique object name.
      /// \return Pointer to a grid geometry object
      protected: virtual GridPtr CreateGridImpl(unsigned int _id,
                     const std::string &_name) = 0;

      /// \brief Implementation for creating a marker geometry object
      /// \param[in] _id unique object id.
      /// \param[in] _name unique object name.
      /// \return Pointer to a marker geometry object
      protected: virtual MarkerPtr CreateMarkerImpl(unsigned int _id,
                     const std::string &_name) = 0;

      /// \brief Implementation for creating a text's geometry object
      /// \param[in] _id unique object id.
      /// \param[in] _name unique object name.
      /// \return Pointer to a text geometry object
      protected: virtual TextPtr CreateTextImpl(unsigned int _id,
                     const std::string &_name);

      protected: virtual MaterialPtr CreateMaterialImpl(unsigned int _id,
                     const std::string &_name) = 0;

      protected: virtual RenderTexturePtr CreateRenderTextureImpl(
                     unsigned int _id, const std::string &_name) = 0;

      /// \brief Render engine specific implementation for creating a render
      /// window
      /// \param[in] _id unique object id
      /// \param[in] _name object name
      /// \return  Pointer to the created render window.
      protected: virtual RenderWindowPtr CreateRenderWindowImpl(
                     unsigned int _id, const std::string &_name) = 0;

      protected: virtual RayQueryPtr CreateRayQueryImpl(
                     unsigned int _id, const std::string &_name) = 0;

      protected: virtual LightStorePtr Lights() const = 0;

      protected: virtual SensorStorePtr Sensors() const = 0;

      protected: virtual VisualStorePtr Visuals() const = 0;

      protected: virtual MaterialMapPtr Materials() const = 0;

      protected: virtual bool LoadImpl() = 0;

      protected: virtual bool InitImpl() = 0;

      private: virtual void CreateNodeStore();

      private: virtual void CreateMaterials();

      /// \brief Helper function to recursively destory nodes while checking
      /// for loops.
      /// \param[in] _node Node to be destroyed
      /// \param[in] _nodeId Holds all node ids that have been visited in the
      /// tree during the destroy process. Used for loop detection.
      private: void DestroyNodeRecursive(NodePtr _node,
          std::set<unsigned int> &_nodeIds);

      protected: unsigned int id;

      IGN_COMMON_WARN_IGNORE__DLL_INTERFACE_MISSING
      protected: std::string name;
      IGN_COMMON_WARN_RESUME__DLL_INTERFACE_MISSING

      protected: common::Time simTime;

      protected: bool loaded;

      protected: bool initialized;

      /// \brief Scene background color. Default should be black.
      protected: math::Color backgroundColor;

      /// \brief The four corners of the gradient background color.
      /// Next is the description of how to interpret each value of the array:
      /// 0: Top left corner color.
      /// 1: Bottom left corner color.
      /// 2: Top right corner color.
      /// 3: Bottom right corner color.
      /// Default should be black.
      protected: std::array<math::Color, 4> gradientBackgroundColor;

      /// \brief Whether the scene has a gradient background.
      protected: bool isGradientBackgroundColor = false;

      private: unsigned int nextObjectId;

      IGN_COMMON_WARN_IGNORE__DLL_INTERFACE_MISSING
      private: NodeStorePtr nodes;
      IGN_COMMON_WARN_RESUME__DLL_INTERFACE_MISSING
    };
    }
  }
}
#endif
