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
#ifndef IGNITION_RENDERING_SCENE_HH_
#define IGNITION_RENDERING_SCENE_HH_

#include <array>
#include <string>
#include <limits>

#include <ignition/common/Material.hh>
#include <ignition/common/Mesh.hh>
#include <ignition/common/Time.hh>

#include <ignition/math/Color.hh>

#include "ignition/rendering/config.hh"
#include "ignition/rendering/MeshDescriptor.hh"
#include "ignition/rendering/RenderTypes.hh"
#include "ignition/rendering/Storage.hh"
#include "ignition/rendering/Export.hh"

namespace ignition
{
  namespace rendering
  {
    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
    //
    class RenderEngine;

    /// \class Scene Scene.hh ignition/rendering/Scene.hh
    /// \brief Manages a single scene-graph. This class updates scene-wide
    /// properties and holds the root scene node. A Scene also serves as a
    /// factory for all scene objects.
    class IGNITION_RENDERING_VISIBLE Scene
    {
      /// \brief Destructor
      public: virtual ~Scene() { }

      /// \brief Load scene-specific resources
      public: virtual void Load() = 0;

      /// \brief Initialize the scene
      public: virtual void Init() = 0;

      // TODO(anyone): merge with Destroy
      public: virtual void Fini() = 0;

      /// \brief Determine if the scene is initialized
      /// \return True if the scene is initialized
      public: virtual bool IsInitialized() const = 0;

      /// \brief Get the ID of the scene
      /// \return The scene ID
      public: virtual unsigned int Id() const = 0;

      /// \brief Get the name of the scene
      /// \return The scene name
      public: virtual std::string Name() const = 0;

      /// \brief Get the creating render-engine of the scene
      /// \return The creating render-engine
      public: virtual RenderEngine *Engine() const = 0;

      /// \brief Get the last simulation update time
      /// \return The last simulation update time
      public: virtual common::Time IGN_DEPRECATED(4) SimTime() const = 0;

      /// \brief Get the last simulation update time
      /// \return The last simulation update time
      public: virtual std::chrono::steady_clock::duration
        Time() const = 0;

      /// \brief Set the last simulation update time
      /// \param[in] _time Latest simulation update time
      public: virtual void IGN_DEPRECATED(4)
        SetSimTime(const common::Time &_time) = 0;

      /// \brief Set the last simulation update time
      /// \param[in] _time Latest simulation update time
      public: virtual void SetTime(
        const std::chrono::steady_clock::duration &_time) = 0;

      /// \brief Get root Visual node. All nodes that are desired to be
      /// rendered in a scene should be added to this Visual or one of its
      /// ancestors in the scene-graph. Nodes created by this Scene will not be
      /// added to the scene by default.
      /// \return The root Visual node
      public: virtual VisualPtr RootVisual() const = 0;

      /// \brief Get a visual at a mouse position.
      /// \param[in] _camera Camera pointer being used
      /// \param[in] _mousePos position of the mouse in pixels
      /// \return Pointer to the visual, NULL if none found.
      public: virtual VisualPtr VisualAt(const CameraPtr &_camera,
                  const math::Vector2i &_mousePos) = 0;

      /// \brief Get the scene ambient light color
      /// \return The scene ambient light color
      public: virtual math::Color AmbientLight() const = 0;

      /// \brief Set the scene ambient light color
      /// \param[in] _r Red color
      /// \param[in] _g Green color
      /// \param[in] _b Blue color
      /// \param[in] _a Alpha color
      public: virtual void SetAmbientLight(double _r, double _g, double _b,
                  double _a = 1.0) = 0;

      /// \brief Set the scene ambient light color
      /// \param[in] _color The scene ambient light color
      public: virtual void SetAmbientLight(const math::Color &_color) = 0;

      /// \brief Get the scene background color
      /// \return The scene background color
      public: virtual math::Color BackgroundColor() const = 0;

      /// \brief Set the scene background color
      /// \param[in] _r Red color
      /// \param[in] _g Green color
      /// \param[in] _b Blue color
      /// \param[in] _a Alpha color
      public: virtual void SetBackgroundColor(double _r, double _g, double _b,
                  double _a = 1.0) = 0;

      /// \brief Set the scene background color
      /// \param[in] _color The scene background color
      public: virtual void SetBackgroundColor(const math::Color &_color) = 0;

      /// \brief Whether the scene has a gradient background or not (solid)
      /// \return True if the scene has a gradient background or false otherwise
      public: virtual bool IsGradientBackgroundColor() const = 0;

      /// \brief Get the scene gradient background color
      /// \return The scene gradient background color
      /// \sa bool IsGradientBackgroundColor() const
      public: virtual std::array<math::Color, 4>
                  GradientBackgroundColor() const = 0;

      /// \brief Set a custom gradient background color on top of the regular
      /// background. Default should be black.
      ///
      /// Important note: Keep in mind that this object will always be rendered
      /// on top of any existing scene or camera background.
      ///
      /// \param[in] _colors The scene gradient background color.
      /// Next is the description of how to interpret each value of the array:
      /// 0: Top left corner color.
      /// 1: Bottom left corner color.
      /// 2: Top right corner color.
      /// 3: Bottom right corner color.
      /// \sa void RemoveGradientBackgroundColor()
      public: virtual void SetGradientBackgroundColor(
                  const std::array<math::Color, 4> &_colors) = 0;

      /// \brief Remove the scene gradient background color
      /// \sa void SetGradientBackgroundColor(
      ///            const std::array<math::Color, 4> &_colors)
      public: virtual void RemoveGradientBackgroundColor() = 0;

      /// \brief Get the number of nodes managed by this scene. Note these
      /// nodes may not be directly or indirectly attached to the root node.
      /// \return The number of nodes managed by this scene
      public: virtual unsigned int NodeCount() const = 0;

      /// \brief Determine if the given node is managed by this Scene
      /// \param[in] _node Node in question
      /// \return True if a node is managed by this scene
      public: virtual bool HasNode(ConstNodePtr _node) const = 0;

      /// \brief Determine if a given node with the given id is managed by
      /// this Scene
      /// \param[in] _id ID of the node in question
      /// \return True if a node is managed by this scene
      public: virtual bool HasNodeId(unsigned int _id) const = 0;

      /// \brief Determine if a given node with the given name is managed by
      /// this Scene
      /// \param[in] _name Name of the node in question
      /// \return True if a node is managed by this scene
      public: virtual bool HasNodeName(const std::string &_name) const = 0;

      /// \brief Get node with the given id. If no node exists with the given
      /// id, NULL will be returned.
      /// \param[in] _id ID of the desired node
      /// \return The desired node
      public: virtual NodePtr NodeById(unsigned int _id) const = 0;

      /// \brief Get node with the given name. If no node exists with the given
      /// name, NULL will be returned.
      /// \param[in] _name Name of the desired node
      /// \return The desired node
      public: virtual NodePtr NodeByName(const std::string &_name) const = 0;

      /// \brief Get node at the given index. If no node exists at the given
      /// index, NULL will be returned.
      /// \param[in] _index Index of the desired node
      /// \return The desired node
      public: virtual NodePtr NodeByIndex(unsigned int _index) const = 0;

      /// \brief Destroy given node. If the given node is not managed by this
      /// scene, no work will be done. Depending on the _recursive argument,
      /// this function will either detach all child nodes from the scene graph
      /// or recursively destroy them.
      /// \param[in] _id ID of the node to destroy
      /// \param[in] _recursive True to recursively destroy the node and its
      /// children, false to destroy only this node and detach the children
      public: virtual void DestroyNode(NodePtr _node,
          bool _recursive = false) = 0;

      /// \brief Destroy node with the given id. If no node exists with the
      /// given id, no work will be done. All children of the node will
      /// consequently be detached from the scene graph, but not destroyed.
      /// \param[in] _id ID of the node to destroy
      public: virtual void DestroyNodeById(unsigned int _id) = 0;

      /// \brief Destroy node with the given name. If no node exists with the
      /// given name, no work will be done. All children of the node will
      /// consequently be detached from the scene graph, but not destroyed.
      /// \param[in] _name Name of the node to destroy
      public: virtual void DestroyNodeByName(const std::string &_name) = 0;

      /// \brief Destroy node at the given index. If no node exists at the
      /// given index, no work will be done. All children of the node will
      /// consequently be detached from the scene graph, but not destroyed.
      /// \param[in] _index Index of the node to destroy
      public: virtual void DestroyNodeByIndex(unsigned int _index) = 0;

      /// \brief Destroy all nodes manages by this scene.
      public: virtual void DestroyNodes() = 0;

      /// \brief Get the number of lights managed by this scene. Note these
      /// lights may not be directly or indirectly attached to the root light.
      /// \return The number of lights managed by this scene
      public: virtual unsigned int LightCount() const = 0;

      /// \brief Determine if the given light is managed by this Scene
      /// \param[in] _light Light in question
      /// \return True if a light is managed by this scene
      public: virtual bool HasLight(ConstLightPtr _light) const = 0;

      /// \brief Determine if a given light with the given id is managed by
      /// this Scene
      /// \param[in] _id ID of the light in question
      /// \return True if a light is managed by this scene
      public: virtual bool HasLightId(unsigned int _id) const = 0;

      /// \brief Determine if a given light with the given name is managed by
      /// this Scene
      /// \param[in] _name Name of the light in question
      /// \return True if a light is managed by this scene
      public: virtual bool HasLightName(const std::string &_name) const = 0;

      /// \brief Get light with the given id. If no light exists with the given
      /// id, NULL will be returned.
      /// \param[in] _id ID of the desired light
      /// \return The desired light
      public: virtual LightPtr LightById(unsigned int _id) const = 0;

      /// \brief Get light with the given name. If no light exists with the
      /// given name, NULL will be returned.
      /// \param[in] _name Name of the desired light
      /// \return The desired light
      public: virtual LightPtr LightByName(
                  const std::string &_name) const = 0;

      /// \brief Get light at the given index. If no light exists at the given
      /// index, NULL will be returned.
      /// \param[in] _index Index of the desired light
      /// \return The desired light
      public: virtual LightPtr LightByIndex(unsigned int _index) const = 0;

      /// \brief Destroy given light. If the given light is not managed by this
      /// scene, no work will be done. Depending on the _recursive argument,
      /// this function will either detach all child nodes from the scene graph
      /// or recursively destroy them.
      /// \param[in] _id ID of the light to destroy
      /// \param[in] _recursive True to recursively destroy the node and its
      /// children, false to destroy only this node and detach the children
      public: virtual void DestroyLight(LightPtr _light,
          bool _recursive = false) = 0;

      /// \brief Destroy light with the given id. If no light exists with the
      /// given id, no work will be done. All children of the light will
      /// consequently be detached from the scene graph, but not destroyed.
      /// \param[in] _id ID of the light to destroy
      public: virtual void DestroyLightById(unsigned int _id) = 0;

      /// \brief Destroy light with the given name. If no light exists with the
      /// given name, no work will be done. All children of the light will
      /// consequently be detached from the scene graph, but not destroyed.
      /// \param[in] _name Name of the light to destroy
      public: virtual void DestroyLightByName(const std::string &_name) = 0;

      /// \brief Destroy light at the given index. If no light exists at the
      /// given index, no work will be done. All children of the light will
      /// consequently be detached from the scene graph, but not destroyed.
      /// \param[in] _index Index of the light to destroy
      public: virtual void DestroyLightByIndex(unsigned int _index) = 0;

      /// \brief Destroy all lights manages by this scene.
      public: virtual void DestroyLights() = 0;

      /// \brief Get the number of sensors managed by this scene. Note these
      /// sensors may not be directly or indirectly attached to the root sensor.
      /// \return The number of sensors managed by this scene
      public: virtual unsigned int SensorCount() const = 0;

      /// \brief Determine if the given sensor is managed by this Scene
      /// \param[in] _sensor Sensor in question
      /// \return True if a sensor is managed by this scene
      public: virtual bool HasSensor(ConstSensorPtr _sensor) const = 0;

      /// \brief Determine if a given sensor with the given id is managed by
      /// this Scene
      /// \param[in] _id ID of the sensor in question
      /// \return True if a sensor is managed by this scene
      public: virtual bool HasSensorId(unsigned int _id) const = 0;

      /// \brief Determine if a given sensor with the given name is managed by
      /// this Scene
      /// \param[in] _name Name of the sensor in question
      /// \return True if a sensor is managed by this scene
      public: virtual bool HasSensorName(const std::string &_name) const = 0;

      /// \brief Get sensor with the given id. If no sensor exists with the
      /// given id, NULL will be returned.
      /// \param[in] _id ID of the desired sensor
      /// \return The desired sensor
      public: virtual SensorPtr SensorById(unsigned int _id) const = 0;

      /// \brief Get sensor with the given name. If no sensor exists with the
      /// given name, NULL will be returned.
      /// \param[in] _name Name of the desired sensor
      /// \return The desired sensor
      public: virtual SensorPtr SensorByName(
                  const std::string &_name) const = 0;

      /// \brief Get sensor at the given index. If no sensor exists at the given
      /// index, NULL will be returned.
      /// \param[in] _index Index of the desired sensor
      /// \return The desired sensor
      public: virtual SensorPtr SensorByIndex(unsigned int _index) const = 0;

      /// \brief Destroy given sensor. If the given sensor is not managed by
      /// this scene, no work will be done. Depending on the _recursive
      /// argument, this function will either detach all child nodes from the
      /// scene graph or recursively destroy them.
      /// \param[in] _id ID of the sensor to destroy
      /// \param[in] _recursive True to recursively destroy the node and its
      /// children, false to destroy only this node and detach the children
      public: virtual void DestroySensor(SensorPtr _sensor,
          bool _recursive = false) = 0;

      /// \brief Destroy sensor with the given id. If no sensor exists with the
      /// given id, no work will be done. All children of the sensor will
      /// consequently be detached from the scene graph, but not destroyed.
      /// \param[in] _id ID of the sensor to destroy
      public: virtual void DestroySensorById(unsigned int _id) = 0;

      /// \brief Destroy sensor with the given name. If no sensor exists with
      /// the given name, no work will be done. All children of the sensor will
      /// consequently be detached from the scene graph, but not destroyed.
      /// \param[in] _name Name of the sensor to destroy
      public: virtual void DestroySensorByName(const std::string &_name) = 0;

      /// \brief Destroy sensor at the given index. If no sensor exists at the
      /// given index, no work will be done. All children of the sensor will
      /// consequently be detached from the scene graph, but not destroyed.
      /// \param[in] _index Index of the sensor to destroy
      public: virtual void DestroySensorByIndex(unsigned int _index) = 0;

      /// \brief Destroy all sensors manages by this scene.
      public: virtual void DestroySensors() = 0;

      /// \brief Get the number of nodes managed by this scene. Note these
      /// nodes may not be directly or indirectly attached to the root node.
      /// \return The number of nodes managed by this scene
      public: virtual unsigned int VisualCount() const = 0;

      /// \brief Determine if the given node is managed by this Scene
      /// \param[in] _node Visual in question
      /// \return True if a node is managed by this scene
      public: virtual bool HasVisual(ConstVisualPtr _node) const = 0;

      /// \brief Determine if a given node with the given id is managed by
      /// this Scene
      /// \param[in] _id ID of the node in question
      /// \return True if a node is managed by this scene
      public: virtual bool HasVisualId(unsigned int _id) const = 0;

      /// \brief Determine if a given node with the given name is managed by
      /// this Scene
      /// \param[in] _name Name of the node in question
      /// \return True if a node is managed by this scene
      public: virtual bool HasVisualName(const std::string &_name) const = 0;

      /// \brief Get node with the given id. If no node exists with the given
      /// id, NULL will be returned.
      /// \param[in] _id ID of the desired node
      /// \return The desired node
      public: virtual VisualPtr VisualById(unsigned int _id) const = 0;

      /// \brief Get node with the given name. If no node exists with the given
      /// name, NULL will be returned.
      /// \param[in] _name Name of the desired node
      /// \return The desired node
      public: virtual VisualPtr VisualByName(
                  const std::string &_name) const = 0;

      /// \brief Get node at the given index. If no node exists at the given
      /// index, NULL will be returned.
      /// \param[in] _index Index of the desired node
      /// \return The desired node
      public: virtual VisualPtr VisualByIndex(unsigned int _index) const = 0;

      /// \brief Destroy given node. If the given node is not managed by this
      /// scene, no work will be done. Depending on the _recursive argument,
      /// this function will either detach all child nodes from the scene graph
      /// or recursively destroy them.
      /// consequently be detached from the scene graph, but not destroyed.
      /// \param[in] _id ID of the node to destroy
      /// \param[in] _recursive True to recursively destroy the node and its
      /// children, false to destroy only this node and detach the children
      public: virtual void DestroyVisual(VisualPtr _node,
          bool _recursive = false) = 0;

      /// \brief Destroy node with the given id. If no node exists with the
      /// given id, no work will be done. All children of the node will
      /// consequently be detached from the scene graph, but not destroyed.
      /// \param[in] _id ID of the node to destroy
      public: virtual void DestroyVisualById(unsigned int _id) = 0;

      /// \brief Destroy node with the given name. If no node exists with the
      /// given name, no work will be done. All children of the node will
      /// consequently be detached from the scene graph, but not destroyed.
      /// \param[in] _name Name of the node to destroy
      public: virtual void DestroyVisualByName(const std::string &_name) = 0;

      /// \brief Destroy node at the given index. If no node exists at the
      /// given index, no work will be done. All children of the node will
      /// consequently be detached from the scene graph, but not destroyed.
      /// \param[in] _index Index of the node to destroy
      public: virtual void DestroyVisualByIndex(unsigned int _index) = 0;

      /// \brief Destroy all nodes manages by this scene.
      public: virtual void DestroyVisuals() = 0;

      /// \brief Determine if a material is registered under the given name
      /// \param[in] _name Name of the material in question
      /// \return True if a material is registered under the given name
      public: virtual bool MaterialRegistered(
                  const std::string &_name) const = 0;

      /// \brief Get material registered under the given name. If no material
      /// is registered under the given name, NULL will be returned.
      /// \param[in] _name Name of the desired material
      /// \return The specified material
      public: virtual MaterialPtr Material(
                  const std::string &_name) const = 0;

      /// \brief Register a new material under the given name. If the name is
      /// already in use, no work will be done.
      /// \param[in] _name Name which the material will be registered under
      /// \param[in] _material Material to register
      public: virtual void RegisterMaterial(const std::string &_name,
                  MaterialPtr _material) = 0;

      /// \brief Unregister material registered under the given name. If no
      /// material is registered under this name, no work will be done.
      /// \param[in] _name Name of the material to unregistered
      public: virtual void UnregisterMaterial(const std::string &_name) = 0;

      /// \brief Unregister all registered materials
      public: virtual void UnregisterMaterials() = 0;

      /// \brief Unregister and destroy a material
      /// \param[in] _material Material to be unregistered and destroyed
      public: virtual void DestroyMaterial(MaterialPtr _material) = 0;

      /// \brief Unregister and destroys all registered materials
      public: virtual void DestroyMaterials() = 0;

      /// \brief Create new directional light. A unique ID and name will
      /// automatically be assigned to the light.
      /// \return The created light
      public: virtual DirectionalLightPtr CreateDirectionalLight() = 0;

      /// \brief Create new directional light with the given ID. A unique name
      /// will automatically be assigned to the light. If the given ID is
      /// already in use, NULL will be returned.
      /// \param[in] _id ID of the new light
      /// \return The created light
      public: virtual DirectionalLightPtr CreateDirectionalLight(
                  unsigned int _id) = 0;

      /// \brief Create new directional light with the given name. A unique ID
      /// will automatically be assigned to the light. If the given name is
      /// already in use, NULL will be returned.
      /// \param[in] _name Name of the new light
      /// \return The created light
      public: virtual DirectionalLightPtr CreateDirectionalLight(
                  const std::string &_name) = 0;

      /// \brief Create new directional light with the given name. If either the
      /// given ID or name is already in use, NULL will be returned.
      /// \param[in] _id ID of the new light
      /// \param[in] _name Name of the new light
      /// \return The created light
      public: virtual DirectionalLightPtr CreateDirectionalLight(
                  unsigned int _id, const std::string &_name) = 0;

      /// \brief Create new point light. A unique ID and name will
      /// automatically be assigned to the light.
      /// \return The created light
      public: virtual PointLightPtr CreatePointLight() = 0;

      /// \brief Create new point light with the given ID. A unique name
      /// will automatically be assigned to the light. If the given ID is
      /// already in use, NULL will be returned.
      /// \param[in] _id ID of the new light
      /// \return The created light
      public: virtual PointLightPtr CreatePointLight(
                  unsigned int _id) = 0;

      /// \brief Create new point light with the given name. A unique ID
      /// will automatically be assigned to the light. If the given name is
      /// already in use, NULL will be returned.
      /// \param[in] _name Name of the new light
      /// \return The created light
      public: virtual PointLightPtr CreatePointLight(
                  const std::string &_name) = 0;

      /// \brief Create new point light with the given name. If either the
      /// given ID or name is already in use, NULL will be returned.
      /// \param[in] _id ID of the new light
      /// \param[in] _name Name of the new light
      /// \return The created light
      public: virtual PointLightPtr CreatePointLight(
                  unsigned int _id, const std::string &_name) = 0;

      /// \brief Create new spotlight. A unique ID and name will
      /// automatically be assigned to the light.
      /// \return The created light
      public: virtual SpotLightPtr CreateSpotLight() = 0;

      /// \brief Create new spotlight with the given ID. A unique name
      /// will automatically be assigned to the light. If the given ID is
      /// already in use, NULL will be returned.
      /// \param[in] _id ID of the new light
      /// \return The created light
      public: virtual SpotLightPtr CreateSpotLight(
                  unsigned int _id) = 0;

      /// \brief Create new spotlight with the given name. A unique ID
      /// will automatically be assigned to the light. If the given name is
      /// already in use, NULL will be returned.
      /// \param[in] _name Name of the new light
      /// \return The created light
      public: virtual SpotLightPtr CreateSpotLight(
                  const std::string &_name) = 0;

      /// \brief Create new spotlight with the given name. If either the
      /// given ID or name is already in use, NULL will be returned.
      /// \param[in] _id ID of the new light
      /// \param[in] _name Name of the new light
      /// \return The created light
      public: virtual SpotLightPtr CreateSpotLight(
                  unsigned int _id, const std::string &_name) = 0;

      /// \brief Create new camera. A unique ID and name will
      /// automatically be assigned to the camera.
      /// \return The created camera
      public: virtual CameraPtr CreateCamera() = 0;

      /// \brief Create new camera with the given ID. A unique name
      /// will automatically be assigned to the camera. If the given ID is
      /// already in use, NULL will be returned.
      /// \param[in] _id ID of the new camera
      /// \return The created camera
      public: virtual CameraPtr CreateCamera(
                  unsigned int _id) = 0;

      /// \brief Create new camera with the given name. A unique ID
      /// will automatically be assigned to the camera. If the given name is
      /// already in use, NULL will be returned.
      /// \param[in] _name Name of the new camera
      /// \return The created camera
      public: virtual CameraPtr CreateCamera(
                  const std::string &_name) = 0;

      /// \brief Create new camera with the given name. If either the
      /// given ID or name is already in use, NULL will be returned.
      /// \param[in] _id ID of the new camera
      /// \param[in] _name Name of the new camera
      /// \return The created camera
      public: virtual CameraPtr CreateCamera(
                  unsigned int _id, const std::string &_name) = 0;

      /// \brief Create new depth camera. A unique ID and name will
      /// automatically be assigned to the camera.
      /// \return The created camera
      public: virtual DepthCameraPtr CreateDepthCamera() = 0;

      /// \brief Create new depth camera with the given ID. A unique name
      /// will automatically be assigned to the camera. If the given ID is
      /// already in use, NULL will be returned.
      /// \param[in] _id ID of the new camera
      /// \return The created camera
      public: virtual DepthCameraPtr CreateDepthCamera(
                  unsigned int _id) = 0;

      /// \brief Create new depth camera with the given name. A unique ID
      /// will automatically be assigned to the camera. If the given name is
      /// already in use, NULL will be returned.
      /// \param[in] _name Name of the new camera
      /// \return The created camera
      public: virtual DepthCameraPtr CreateDepthCamera(
                  const std::string &_name) = 0;

      /// \brief Create new depth camera with the given name. If either the
      /// given ID or name is already in use, NULL will be returned.
      /// \param[in] _id ID of the new camera
      /// \param[in] _name Name of the new camera
      /// \return The created camera
      public: virtual DepthCameraPtr CreateDepthCamera(
                  unsigned int _id, const std::string &_name) = 0;

      /// \brief Create new thermal camera. A unique ID and name will
      /// automatically be assigned to the camera.
      /// \return The created camera
      public: virtual ThermalCameraPtr CreateThermalCamera() = 0;

      /// \brief Create new thermal camera with the given ID. A unique name
      /// will automatically be assigned to the camera. If the given ID is
      /// already in use, NULL will be returned.
      /// \param[in] _id ID of the new camera
      /// \return The created camera
      public: virtual ThermalCameraPtr CreateThermalCamera(
                  unsigned int _id) = 0;

      /// \brief Create new thermal camera with the given name. A unique ID
      /// will automatically be assigned to the camera. If the given name is
      /// already in use, NULL will be returned.
      /// \param[in] _name Name of the new camera
      /// \return The created camera
      public: virtual ThermalCameraPtr CreateThermalCamera(
                  const std::string &_name) = 0;

      /// \brief Create new thermal camera with the given name. If either the
      /// given ID or name is already in use, NULL will be returned.
      /// \param[in] _id ID of the new camera
      /// \param[in] _name Name of the new camera
      /// \return The created camera
      public: virtual ThermalCameraPtr CreateThermalCamera(
                  unsigned int _id, const std::string &_name) = 0;

      /// \brief Create new gpu rays caster. A unique ID and name will
      /// automatically be assigned to the gpu rays caster.
      /// \return The created gpu rays caster
      public: virtual GpuRaysPtr CreateGpuRays() = 0;

      /// \brief Create new gpu rays caster with the given ID. A unique name
      /// will automatically be assigned to the gpu rays caster. If the given
      /// ID is already in use, NULL will be returned.
      /// \param[in] _id ID of the new gpu rays caster
      /// \return The created gpu rays caster
      public: virtual GpuRaysPtr CreateGpuRays(unsigned int _id) = 0;

      /// \brief Create new gpu rays caster with the given name. A unique ID
      /// will automatically be assigned to the gpu rays caster. If the given
      /// name is already in use, NULL will be returned.
      /// \param[in] _name Name of the new gpu rays caster
      /// \return The created gpu ray caster
      public: virtual GpuRaysPtr CreateGpuRays(
                  const std::string &_name) = 0;

      /// \brief Create new gpu rays caster with the given name. If either
      /// the given ID or name is already in use, NULL will be returned.
      /// \param[in] _id ID of the gpu ray caster
      /// \param[in] _name Name of the new gpu ray caster
      /// \return The created Gpu ray caster
      public: virtual GpuRaysPtr CreateGpuRays(
                  unsigned int _id, const std::string &_name) = 0;

      /// \brief Create new visual. A unique ID and name will
      /// automatically be assigned to the visual.
      /// \return The created visual
      public: virtual VisualPtr CreateVisual() = 0;

      /// \brief Create new visual with the given ID. A unique name
      /// will automatically be assigned to the visual. If the given ID is
      /// already in use, NULL will be returned.
      /// \param[in] _id ID of the new visual
      /// \return The created visual
      public: virtual VisualPtr CreateVisual(
                  unsigned int _id) = 0;

      /// \brief Create new visual with the given name. A unique ID
      /// will automatically be assigned to the visual. If the given name is
      /// already in use, NULL will be returned.
      /// \param[in] _name Name of the new visual
      /// \return The created visual
      public: virtual VisualPtr CreateVisual(
                  const std::string &_name) = 0;

      /// \brief Create new visual with the given name. If either the
      /// given ID or name is already in use, NULL will be returned.
      /// \param[in] _id ID of the new visual
      /// \param[in] _name Name of the new visual
      /// \return The created visual
      public: virtual VisualPtr CreateVisual(
                  unsigned int _id, const std::string &_name) = 0;

      /// \brief Create new arrow visual. A unique ID and name will
      /// automatically be assigned to the visual.
      /// \return The created arrow visual
      public: virtual ArrowVisualPtr CreateArrowVisual() = 0;

      /// \brief Create new arrow visual with the given ID. A unique name
      /// will automatically be assigned to the visual. If the given ID is
      /// already in use, NULL will be returned.
      /// \param[in] _id ID of the new arrow visual
      /// \return The created arrow visual
      public: virtual ArrowVisualPtr CreateArrowVisual(
                  unsigned int _id) = 0;

      /// \brief Create new arrow visual with the given name. A unique ID
      /// will automatically be assigned to the visual. If the given name is
      /// already in use, NULL will be returned.
      /// \param[in] _name Name of the new arrow visual
      /// \return The created arrow visual
      public: virtual ArrowVisualPtr CreateArrowVisual(
                  const std::string &_name) = 0;

      /// \brief Create new arrow visual with the given name. If either the
      /// given ID or name is already in use, NULL will be returned.
      /// \param[in] _id ID of the new arrow visual
      /// \param[in] _name Name of the new arrow visual
      /// \return The created arrow visual
      public: virtual ArrowVisualPtr CreateArrowVisual(
                  unsigned int _id, const std::string &_name) = 0;

      /// \brief Create new axis visual. A unique ID and name will
      /// automatically be assigned to the visual.
      /// \return The created axis visual
      public: virtual AxisVisualPtr CreateAxisVisual() = 0;

      /// \brief Create new axis visual with the given ID. A unique name
      /// will automatically be assigned to the visual. If the given ID is
      /// already in use, NULL will be returned.
      /// \param[in] _id ID of the new axis visual
      /// \return The created axis visual
      public: virtual AxisVisualPtr CreateAxisVisual(
                  unsigned int _id) = 0;

      /// \brief Create new axis visual with the given name. A unique ID
      /// will automatically be assigned to the visual. If the given name is
      /// already in use, NULL will be returned.
      /// \param[in] _name Name of the new axis visual
      /// \return The created axis visual
      public: virtual AxisVisualPtr CreateAxisVisual(
                  const std::string &_name) = 0;

      /// \brief Create new axis visual with the given name. If either the
      /// given ID or name is already in use, NULL will be returned.
      /// \param[in] _id ID of the new axis visual
      /// \param[in] _name Name of the new axis visual
      /// \return The created axis visual
      public: virtual AxisVisualPtr CreateAxisVisual(
                  unsigned int _id, const std::string &_name) = 0;

      /// \brief Create new gizmo visual. A unique ID and name will
      /// automatically be assigned to the visual.
      /// \return The created gizmo visual
      public: virtual GizmoVisualPtr CreateGizmoVisual() = 0;

      /// \brief Create new gizmo visual with the given ID. A unique name
      /// will automatically be assigned to the visual. If the given ID is
      /// already in use, NULL will be returned.
      /// \param[in] _id ID of the new gizmo visual
      /// \return The created gizmo visual
      public: virtual GizmoVisualPtr CreateGizmoVisual(
                  unsigned int _id) = 0;

      /// \brief Create new gizmo visual with the given name. A unique ID
      /// will automatically be assigned to the visual. If the given name is
      /// already in use, NULL will be returned.
      /// \param[in] _name Name of the new gizmo visual
      /// \return The created gizmo visual
      public: virtual GizmoVisualPtr CreateGizmoVisual(
                  const std::string &_name) = 0;

      /// \brief Create new gizmo visual with the given name. If either the
      /// given ID or name is already in use, NULL will be returned.
      /// \param[in] _id ID of the new gizmo visual
      /// \param[in] _name Name of the new gizmo visual
      /// \return The created gizmo visual
      public: virtual GizmoVisualPtr CreateGizmoVisual(
                  unsigned int _id, const std::string &_name) = 0;

      /// \brief Create new box geometry
      /// \return The created box
      public: virtual GeometryPtr CreateBox() = 0;

      /// \brief Create new cone geometry
      /// \return The created cone
      public: virtual GeometryPtr CreateCone() = 0;

      /// \brief Create new cylinder geometry
      /// \return The created cylinder
      public: virtual GeometryPtr CreateCylinder() = 0;

      /// \brief Create new plane geometry
      /// \return The created plane
      public: virtual GeometryPtr CreatePlane() = 0;

      /// \brief Create new sphere geometry
      /// \return The created sphere
      public: virtual GeometryPtr CreateSphere() = 0;

      /// \brief Create new mesh geomerty. The rendering::Mesh will be created
      /// from a common::Mesh retrieved from common::MeshManager using the given
      /// mesh name. If no mesh exists by this name, NULL will be returned. All
      /// sub-meshes will be loaded into the created mesh, uncentered.
      /// \param[in] _meshName Name of the reference mesh
      /// \return The created mesh
      public: virtual MeshPtr CreateMesh(const std::string &_meshName) = 0;

      /// \brief Create new mesh geomerty. The rendering::Mesh will be created
      /// from the given common::Mesh. All sub-meshes will be loaded into this
      /// created mesh, uncentered.
      /// \param[in] _mesh Reference mesh
      /// \return The created mesh
      public: virtual MeshPtr CreateMesh(const common::Mesh *_mesh) = 0;

      /// \brief Create new mesh geometry. The rendering::Mesh will be created
      /// from the given common::Mesh specified in the MeshDescriptor.
      /// Sub-meshes will be loaded and centered according to the descriptor.
      /// \param[in] _desc Descriptor of the mesh to load
      /// \return The created mesh
      public: virtual MeshPtr CreateMesh(const MeshDescriptor &_desc) = 0;

      /// \brief Create new grid geometry.
      /// \return The created grid
      public: virtual GridPtr CreateGrid() = 0;

      /// \brief Create new wire box geometry.
      /// \return The created wire box
      public: virtual WireBoxPtr CreateWireBox() = 0;

      /// \brief Create new marker geometry.
      /// \return The created marker
      public: virtual MarkerPtr CreateMarker() = 0;

      /// \brief Create new lidar visual. A unique ID and name will
      /// automatically be assigned to the lidar visual.
      /// \return The created lidar visual
      public: virtual LidarVisualPtr CreateLidarVisual() = 0;

      /// \brief Create new lidar visual with the given ID. A unique name
      /// will automatically be assigned to the lidar visual. If the given
      /// ID is already in use, NULL will be returned.
      /// \param[in] _id ID of the new lidar visual
      /// \return The created lidar visual
      public: virtual LidarVisualPtr CreateLidarVisual(unsigned int _id) = 0;

      /// \brief Create new lidar visual with the given name. A unique ID
      /// will automatically be assigned to the lidar visual. If the given
      /// name is already in use, NULL will be returned.
      /// \param[in] _name Name of the new lidar visual
      /// \return The created lidar visual
      public: virtual LidarVisualPtr CreateLidarVisual(
                  const std::string &_name) = 0;

      /// \brief Create new lidar visual with the given name. If either
      /// the given ID or name is already in use, NULL will be returned.
      /// \param[in] _id ID of the lidar visual.
      /// \param[in] _name Name of the new lidar visual.
      /// \return The created lidar visual
      public: virtual LidarVisualPtr CreateLidarVisual(
                  unsigned int _id, const std::string &_name) = 0;

      /// \brief Create new text geometry.
      /// \return The created text
      public: virtual TextPtr CreateText() = 0;

      /// \brief Create new material with the given name. Created material will
      /// have default properties.
      /// \param[in] _name Name for the new material.
      /// \return The created material
      public: virtual MaterialPtr CreateMaterial(const std::string &_name = "")
                  = 0;

      /// \brief Create new material from the reference common::Material
      /// \param[in] _material Reference material
      /// \return The created material
      public: virtual MaterialPtr CreateMaterial(
                  const common::Material &_material) = 0;

      /// \brief Create new render texture
      /// \return The created render texture
      public: virtual RenderTexturePtr CreateRenderTexture() = 0;

      /// \brief Create new render window. This feature is render engine
      /// dependent. If the engine does not support attaching to a windowing
      /// system then it should behave as a a render texture.
      /// \return The created render window
      public: virtual RenderWindowPtr CreateRenderWindow() = 0;

      /// \brief Create new ray query
      /// \return The created ray query
      public: virtual RayQueryPtr CreateRayQuery() = 0;

      /// \brief Create new particle emitter. A unique ID and name will
      /// automatically be assigned to the visual.
      /// \return The created particle emitter
      public: virtual ParticleEmitterPtr CreateParticleEmitter() = 0;

      /// \brief Create new particle emitter with the given ID. A unique name
      /// will automatically be assigned to the visual. If the given ID is
      /// already in use, NULL will be returned.
      /// \param[in] _id ID of the new particle emitter
      /// \return The created particle emitter
      public: virtual ParticleEmitterPtr CreateParticleEmitter(
                  unsigned int _id) = 0;

      /// \brief Create new particle emitter with the given name. A unique ID
      /// will automatically be assigned to the visual. If the given name is
      /// already in use, NULL will be returned.
      /// \param[in] _name Name of the new particle emitter
      /// \return The created particle emitter
      public: virtual ParticleEmitterPtr CreateParticleEmitter(
                  const std::string &_name) = 0;

      /// \brief Create new particle emitter with the given name. If either the
      /// given ID or name is already in use, NULL will be returned.
      /// \param[in] _id ID of the new particle emitter
      /// \param[in] _name Name of the new particle emitter
      /// \return The created particle emitter
      public: virtual ParticleEmitterPtr CreateParticleEmitter(
                  unsigned int _id, const std::string &_name) = 0;

      /// \brief Prepare scene for rendering. The scene will flushing any scene
      /// changes by traversing scene-graph, calling PreRender on all objects
      public: virtual void PreRender() = 0;

      /// \brief Remove and destroy all objects from the scene graph. This does
      /// not completely destroy scene resources, so new objects can be created
      /// and added to the scene afterwards.
      public: virtual void Clear() = 0;

      /// \brief Completely destroy the scene an all its resources. Continued
      /// use of this scene after its destruction will result in undefined
      /// behavior.
      public: virtual void Destroy() = 0;
    };
    }
  }
}
#endif
