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
#ifndef IGNITION_RENDERING_EXAMPLES_SCENEMANAGERPRIVATE_HH
#define IGNITION_RENDERING_EXAMPLES_SCENEMANAGERPRIVATE_HH

#include <map>
#include <string>
#include <vector>

#include <gz/common/Time.hh>

#include <gz/math/Color.hh>
#include <gz/math/Pose3.hh>

#include "gz/rendering/RenderTypes.hh"
#include "gz/rendering/ShaderType.hh"
#include "gazebo/transport/Node.hh"

namespace gz
{
  namespace rendering
  {
    class CurrentSceneManager;

    class NewSceneManager;

    class SceneManagerPrivate
    {
      private: typedef std::map<unsigned int, std::string> RemovalMap;

      public: SceneManagerPrivate();

      public: ~SceneManagerPrivate();

      public: void Load();

      public: void Init();

      public: void Fini();

      public: unsigned int SceneCount() const;

      public: bool HasScene(unsigned int _id) const;

      public: bool HasScene(const std::string &_name) const;

      public: bool HasScene(ConstScenePtr _scene) const;

      public: ScenePtr Scene(unsigned int _id) const;

      public: ScenePtr Scene(const std::string &_name) const;

      public: ScenePtr SceneAt(unsigned int _index) const;

      public: void AddScene(ScenePtr _scene);

      public: ScenePtr RemoveScene(unsigned int _id);

      public: ScenePtr RemoveScene(const std::string &_name);

      public: ScenePtr RemoveScene(ScenePtr _scene);

      public: ScenePtr RemoveSceneAt(unsigned int _index);

      public: void RemoveScenes();

      public: void UpdateScenes();

      public: void OnRequest(::ConstRequestPtr &_requestMsg);

      public: void OnResponse(::ConstResponsePtr &_responseMsg);

      public: void OnSceneResponse(::ConstResponsePtr &_responseMsg);

      public: void OnRemovalResponse(::ConstResponsePtr &_responseMsg);

      public: void OnSceneUpdate(::ConstScenePtr _sceneMsg);

      public: void OnLightUpdate(::ConstLightPtr &_lightMsg);

      public: void OnModelUpdate(::ConstModelPtr &_modelMsg);

      public: void OnJointUpdate(::ConstJointPtr &_jointMsg);

      public: void OnVisualUpdate(::ConstVisualPtr &_visualMsg);

      public: void OnSensorUpdate(::ConstSensorPtr &_sensorMsg);

      public: void OnPoseUpdate(::ConstPosesStampedPtr &_posesMsg);

      private: void OnRemovalUpdate(const std::string &_name);

      private: void SendSceneRequest();

      private: void PromoteNewScenes();

      private: void DemoteCurrentScenes();

      private: CurrentSceneManager *currentSceneManager;

      private: NewSceneManager *newSceneManager;

      private: sim::transport::NodePtr transportNode;

      private: sim::transport::PublisherPtr requestPub;

      private: sim::transport::SubscriberPtr requestSub;

      private: sim::transport::SubscriberPtr responseSub;

      private: sim::transport::SubscriberPtr lightSub;

      private: sim::transport::SubscriberPtr modelSub;

      private: sim::transport::SubscriberPtr jointSub;

      private: sim::transport::SubscriberPtr visualSub;

      private: sim::transport::SubscriberPtr sensorSub;

      private: sim::transport::SubscriberPtr poseSub;

      private: sim::event::ConnectionPtr preRenderConn;

      private: int sceneRequestId;

      private: bool promotionNeeded;

      private: RemovalMap requestedRemovals;

      private: std::mutex generalMutex;

      private: std::mutex poseMutex;
    };

    class SubSceneManager
    {
      protected: typedef sim::msgs::Geometry::Type GeomType;

      protected: typedef void (SubSceneManager::*GeomFunc)
                     (const sim::msgs::Geometry&, VisualPtr);

      public: SubSceneManager();

      public: virtual ~SubSceneManager();

      public: virtual unsigned int SceneCount() const;

      public: virtual bool HasScene(unsigned int _id) const;

      public: virtual bool HasScene(const std::string &_name) const;

      public: virtual bool HasScene(ConstScenePtr _scene) const;

      public: virtual ScenePtr Scene(unsigned int _id) const;

      public: virtual ScenePtr Scene(const std::string &_name) const;

      public: virtual ScenePtr SceneAt(unsigned int _index) const;

      public: virtual void AddScene(ScenePtr _scene);

      public: virtual ScenePtr RemoveScene(unsigned int _id);

      public: virtual ScenePtr RemoveScene(const std::string &_name);

      public: virtual ScenePtr RemoveScene(ScenePtr _scene);

      public: virtual ScenePtr RemoveSceneAt(unsigned int _index);

      public: virtual void RemoveScenes();

      public: virtual void UpdateScenes();

      public: virtual void OnLightUpdate(::ConstLightPtr &_lightMsg);

      public: virtual void OnModelUpdate(::ConstModelPtr &_modelMsg);

      public: virtual void OnJointUpdate(::ConstJointPtr &_jointMsg);

      public: virtual void OnVisualUpdate(::ConstVisualPtr &_visualMsg);

      public: virtual void OnSensorUpdate(::ConstSensorPtr &_sensorMsg);

      public: virtual void OnPoseUpdate(::ConstPosesStampedPtr &_posesMsg) = 0;

      public: virtual void OnRemovalUpdate(const std::string &_name);

      public: virtual void Clear();

      protected: virtual void ProcessMessages();

      protected: virtual void ClearMessages();

      protected: virtual void ProcessLights() = 0;

      protected: virtual void ProcessLight(
          const sim::msgs::Light &_lightMsg);

      public: virtual void ProcessLight(
          const sim::msgs::Light &_lightMsg, VisualPtr _parent);

      protected: virtual void ProcessDirectionalLight(
                     const sim::msgs::Light &_lightMsg, VisualPtr _parent);

      protected: virtual void ProcessDirectionalLightImpl(
          const sim::msgs::Light &_lightMsg, DirectionalLightPtr _light);

      protected: virtual DirectionalLightPtr DirectionalLight(
                     const sim::msgs::Light &_lightMsg, VisualPtr _parent);

      protected: virtual DirectionalLightPtr CreateDirectionalLight(
                     const sim::msgs::Light &_lightMsg);

      protected: virtual void ProcessPointLight(
          const sim::msgs::Light &_lightMsg, VisualPtr _parent);

      protected: virtual void ProcessPointLightImpl(
          const sim::msgs::Light &_lightMsg, PointLightPtr _light);

      protected: virtual PointLightPtr PointLight(
                     const sim::msgs::Light &_lightMsg, VisualPtr _parent);

      protected: virtual PointLightPtr CreatePointLight(
                     const sim::msgs::Light &_lightMsg);

      protected: virtual void ProcessSpotLight(
          const sim::msgs::Light &_lightMsg, VisualPtr _parent);

      protected: virtual void ProcessSpotLightImpl(
          const sim::msgs::Light &_lightMsg, SpotLightPtr _light);

      protected: virtual SpotLightPtr SpotLight(
          const sim::msgs::Light &_lightMsg, VisualPtr _parent);

      protected: virtual SpotLightPtr CreateSpotLight(
                     const sim::msgs::Light &_lightMsg);

      protected: virtual void ProcessLightImpl(
          const sim::msgs::Light &_lightMsg, LightPtr _light);

      protected: virtual void ProcessSensors() = 0;

      protected: virtual void ProcessSensor(
          const sim::msgs::Sensor &_sensorMsg);

      protected: virtual void ProcessSensor(
          const sim::msgs::Sensor &_sensorMsg, VisualPtr _parent);

      protected: virtual void ProcessCamera(
          const sim::msgs::Sensor &_sensorMsg, VisualPtr _parent);

      protected: virtual CameraPtr Camera(
          const sim::msgs::Sensor &_sensorMsg, VisualPtr _parent);

      protected: virtual CameraPtr CreateCamera(
         const sim::msgs::Sensor &_sensorMsg);

      protected: virtual void ProcessModels() = 0;

      protected: virtual void ProcessModel(
          const sim::msgs::Model &_modelMsg);

      protected: virtual void ProcessModel(
          const sim::msgs::Model &_modelMsg, VisualPtr _parent);

      protected: virtual VisualPtr Model(const sim::msgs::Model &_modelMsg,
                     VisualPtr _parent);

      protected: virtual void ProcessJoints() = 0;

      protected: virtual void ProcessJoint(
          const sim::msgs::Joint &_jointMsg);

      protected: virtual void ProcessJoint(
          const sim::msgs::Joint &_jointMsg, VisualPtr _parent);

      protected: virtual VisualPtr Joint(const sim::msgs::Joint &_jointMsg,
                     VisualPtr _parent);

      protected: virtual void ProcessVisuals() = 0;

      protected: virtual void ProcessVisual(
          const sim::msgs::Visual &_visualMsg);

      protected: virtual void ProcessVisual(
          const sim::msgs::Visual &_visualMsg, VisualPtr _parent);

      protected: virtual VisualPtr Visual(
          const sim::msgs::Visual &_visualMsg, VisualPtr _parent);

      protected: virtual void ProcessLink(const sim::msgs::Link &_linkMsg,
                     VisualPtr _parent);

      protected: virtual VisualPtr Link(const sim::msgs::Link &_linkMsg,
                     VisualPtr _parent);

      protected: virtual VisualPtr Visual(bool _hasId, unsigned int _id,
                     const std::string &_name, VisualPtr _parent);

      protected: virtual VisualPtr CreateVisual(bool _hasId, unsigned int _id,
                     const std::string &_name);

      protected: virtual void ProcessGeometry(
          const sim::msgs::Geometry &_geometryMsg, VisualPtr _parent);

      protected: virtual void ProcessBox(
          const sim::msgs::Geometry &_geometryMsg, VisualPtr _parent);

      protected: virtual void ProcessCone(
          const sim::msgs::Geometry &_geometryMsg, VisualPtr _parent);

      protected: virtual void ProcessCylinder(
          const sim::msgs::Geometry &_geometryMsg, VisualPtr _parent);

      protected: virtual void ProcessEmpty(
          const sim::msgs::Geometry &_geometryMsg, VisualPtr _parent);

      protected: virtual void ProcessMesh(
          const sim::msgs::Geometry &_geometryMsg, VisualPtr _parent);

      protected: virtual void ProcessPlane(
          const sim::msgs::Geometry &_geometryMsg, VisualPtr _parent);

      protected: virtual void ProcessSphere(
          const sim::msgs::Geometry &_geometryMsg, VisualPtr _parent);

      protected: virtual MaterialPtr CreateMaterial(
                  const sim::msgs::Material &_materialMsg);

      protected: virtual void ProcessPoses() = 0;

      protected: virtual void ProcessPose(const sim::msgs::Pose &_poseMsg);

      protected: virtual void SetPose(NodePtr _node,
                     const sim::msgs::Pose &_poseMsg);

      protected: virtual void SetScale(VisualPtr _visual,
                     const sim::msgs::Vector3d &_scaleMsg);

      protected: virtual void ProcessRemovals() = 0;

      protected: virtual void ProcessRemoval(const std::string &_name);

      protected: virtual VisualPtr Parent(const std::string &_name);

      protected: static math::Color Convert(
          const sim::msgs::Color &_colorMsg);

      protected: static math::Pose3d Convert(
          const sim::msgs::Pose &_poseMsg);

      protected: static math::Vector3d Convert(
          const sim::msgs::Vector3d &_vecMsg);

      protected: static math::Quaterniond Convert(
                     const sim::msgs::Quaternion &_quatMsg);

      protected: static ShaderType Convert(
          sim::msgs::Material::ShaderType _type);

      private: void CreateGeometryFunctionMap();

      protected: ScenePtr activeScene;

      protected: std::vector<ScenePtr> scenes;

      protected: common::Time timePosesReceived;

      protected: std::vector<sim::msgs::Light> lightMsgs;

      protected: std::vector<sim::msgs::Model> modelMsgs;

      protected: std::vector<sim::msgs::Joint> jointMsgs;

      protected: std::vector<sim::msgs::Visual> visualMsgs;

      protected: std::vector<sim::msgs::Sensor> sensorMsgs;

      protected: std::vector<std::string> approvedRemovals;

      protected: std::map<GeomType, GeomFunc> geomFunctions;
    };

    class CurrentSceneManager :
      public virtual SubSceneManager
    {
      public: CurrentSceneManager();

      public: virtual ~CurrentSceneManager();

      public: virtual void OnPoseUpdate(::ConstPosesStampedPtr &_posesMsg);

      protected: virtual void ClearMessages();

      protected: virtual void ProcessLights();

      protected: virtual void ProcessModels();

      protected: virtual void ProcessJoints();

      protected: virtual void ProcessVisuals();

      protected: virtual void ProcessSensors();

      protected: virtual void ProcessPoses();

      protected: virtual void ProcessRemovals();

      private: std::map<std::string, sim::msgs::Pose> poseMsgs;
    };

    class NewSceneManager :
      public virtual SubSceneManager
    {
      public: NewSceneManager();

      public: virtual ~NewSceneManager();

      public: virtual void SetSceneData(const std::string &_data);

      public: virtual void OnPoseUpdate(::ConstPosesStampedPtr &_posesMsg);

      protected: virtual void ProcessMessages();

      protected: virtual void ClearMessages();

      protected: virtual void ProcessScene();

      protected: virtual void ProcessLights();

      protected: virtual void ProcessModels();

      protected: virtual void ProcessJoints();

      protected: virtual void ProcessVisuals();

      protected: virtual void ProcessSensors();

      protected: virtual void ProcessPoses();

      protected: virtual void ProcessPoses(
          const sim::msgs::PosesStamped &_posesMsg);

      protected: virtual void ProcessRemovals();

      protected: bool sceneReceived;

      protected: sim::msgs::Scene sceneMsg;

      private: std::vector<sim::msgs::PosesStamped> posesMsgs;
    };
  }
}
#endif
