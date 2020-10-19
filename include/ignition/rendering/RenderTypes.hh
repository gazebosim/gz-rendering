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
#ifndef IGNITION_RENDERING_RENDERTYPES_HH_
#define IGNITION_RENDERING_RENDERTYPES_HH_

#include <memory>
#include <ignition/rendering/config.hh>


/// \def IGN_VISIBILITY_ALL
/// \brief Render everything visibility mask.
#define IGN_VISIBILITY_ALL             0x0FFFFFFF

/// \def IGN_VISIBILITY_SELECTION
/// \brief Renders only objects that can be selected.
#define IGN_VISIBILITY_SELECTION       0x10000000

/// \def IGN_VISIBILITY_GUI
/// \brief Render GUI visuals mask.
#define IGN_VISIBILITY_GUI             0x00000001

/// \def IGN_VISIBILITY_SELECTABLE
/// \brief Render visuals that are selectable mask.
#define IGN_VISIBILITY_SELECTABLE      0x00000002

namespace ignition
{
  namespace rendering
  {
    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
    //
    template <class T>
    using shared_ptr = std::shared_ptr<T>;

    class ArrowVisual;
    class AxisVisual;
    class Camera;
    class DepthCamera;
    class DirectionalLight;
    class GaussianNoisePass;
    class Geometry;
    class GizmoVisual;
    class GpuRays;
    class Grid;
    class JointVisual;
    class Image;
    class Light;
    class LidarVisual;
    class Material;
    class Marker;
    class Mesh;
    class Node;
    class Object;
    class ObjectFactory;
    class ParticleEmitter;
    class PointLight;
    class RayQuery;
    class RenderEngine;
    class RenderPass;
    class RenderPassSystem;
    class RenderTarget;
    class RenderTexture;
    class RenderWindow;
    class Scene;
    class Sensor;
    class ShaderParams;
    class SpotLight;
    class SubMesh;
    class Text;
    class ThermalCamera;
    class Visual;
    class WireBox;

    /// \def ArrowVisualPtr
    /// \brief Shared pointer to ArrowVisual
    typedef shared_ptr<ArrowVisual> ArrowVisualPtr;

    /// \def AxisVisualPtr
    /// \brief Shared pointer to AxisVisual
    typedef shared_ptr<AxisVisual> AxisVisualPtr;

    /// \def CameraPtr
    /// \brief Shared pointer to Camera
    typedef shared_ptr<Camera> CameraPtr;

    /// \def DepthCameraPtr
    /// \brief Shared pointer to DepthCamera
    typedef shared_ptr<DepthCamera> DepthCameraPtr;

    /// \def ThermalCameraPtr
    /// \brief Shared pointer to ThermalCamera
    typedef shared_ptr<ThermalCamera> ThermalCameraPtr;

    /// \def GpuRaysPtr
    /// \brief Shared pointer to GpuRays
    typedef shared_ptr<GpuRays> GpuRaysPtr;

    /// \def DirectionalLightPtr
    /// \brief Shared pointer to DirectionalLight
    typedef shared_ptr<DirectionalLight> DirectionalLightPtr;

    /// \def GaussianNoisePass
    /// \brief Shared pointer to GaussianNoisePass
    typedef shared_ptr<GaussianNoisePass> GaussianNoisePassPtr;

    /// \def GeometryPtr
    /// \brief Shared pointer to Geometry
    typedef shared_ptr<Geometry> GeometryPtr;

    /// \def GizmoVisualPtr
    /// \brief Shared pointer to GizmoVisual
    typedef shared_ptr<GizmoVisual> GizmoVisualPtr;

    /// \def GridPtr
    /// \brief Shared pointer to Grid
    typedef shared_ptr<Grid> GridPtr;

    /// \def JointVisualPtr
    /// \brief Shared pointer to JointVisual
    typedef shared_ptr<JointVisual> JointVisualPtr;

    /// \def ImagePtr
    /// \brief Shared pointer to Image
    typedef shared_ptr<Image> ImagePtr;

    /// \def LightPtr
    /// \brief Shared pointer to Light
    typedef shared_ptr<Light> LightPtr;

    /// \def LidarVisualPtr
    /// \brief Shared pointer to LidarVisual
    typedef shared_ptr<LidarVisual> LidarVisualPtr;

    /// \def MaterialPtr
    /// \brief Shared pointer to Material
    typedef shared_ptr<Material> MaterialPtr;

    /// \def MarkerPtr
    /// \brief Shared pointer to Marker
    typedef shared_ptr<Marker> MarkerPtr;

    /// \def MeshPtr
    /// \brief Shared pointer to Mesh
    typedef shared_ptr<Mesh> MeshPtr;

    /// \def NodePtr
    /// \brief Shared pointer to Node
    typedef shared_ptr<Node> NodePtr;

    /// \def ObjectPtr
    /// \brief Shared pointer to Object
    typedef shared_ptr<Object> ObjectPtr;

    /// \def ObjectFactoryPtr
    /// \brief Shared pointer to ObjectFactory
    typedef shared_ptr<ObjectFactory> ObjectFactoryPtr;

    /// \def ParticleEmitterPtr
    /// \brief Shared pointer to ParticleEmitter
    typedef shared_ptr<ParticleEmitter> ParticleEmitterPtr;

    /// \def PointLightPtr
    /// \brief Shared pointer to PointLight
    typedef shared_ptr<PointLight> PointLightPtr;

    /// \def RayQueryPtr
    /// \brief Shared pointer to RayQuery
    typedef shared_ptr<RayQuery> RayQueryPtr;

    /// \def RenderPassPtr
    /// \brief Shared pointer to RenderPass
    typedef shared_ptr<RenderPass> RenderPassPtr;

    /// \def RenderPassSystemPtr
    /// \brief Shared pointer to RenderPassSystem
    typedef shared_ptr<RenderPassSystem> RenderPassSystemPtr;

    /// \def RenderTargetPtr
    /// \brief Shared pointer to RenderTarget
    typedef shared_ptr<RenderTarget> RenderTargetPtr;

    /// \def RenderTexturePtr
    /// \brief Shared pointer to RenderTexture
    typedef shared_ptr<RenderTexture> RenderTexturePtr;

    /// \def RenderWindowPtr
    /// \brief Shared pointer to RenderWindow
    typedef shared_ptr<RenderWindow> RenderWindowPtr;
    /// \def ScenePtr
    /// \brief Shared pointer to Scene
    typedef shared_ptr<Scene> ScenePtr;

    /// \def SensorPtr
    /// \brief Shared pointer to Sensor
    typedef shared_ptr<Sensor> SensorPtr;

    /// \brief Shared pointer to ShaderParams
    typedef shared_ptr<ShaderParams> ShaderParamsPtr;

    /// \def SpotLightPtr
    /// \brief Shared pointer to SpotLight
    typedef shared_ptr<SpotLight> SpotLightPtr;

    /// \def SubMeshPtr
    /// \brief Shared pointer to SubMesh
    typedef shared_ptr<SubMesh> SubMeshPtr;

    /// \def TextPtr
    /// \brief Shared pointer to Text
    typedef shared_ptr<Text> TextPtr;

    /// \def VisualPtr
    /// \brief Shared pointer to Visual
    typedef shared_ptr<Visual> VisualPtr;

    /// \def WireBoxPtr
    /// \brief Shared pointer to WireBox
    typedef shared_ptr<WireBox> WireBoxPtr;

    /// \def const ArrowVisualPtr
    /// \brief Shared pointer to const ArrowVisual
    typedef shared_ptr<const ArrowVisual> ConstArrowVisualPtr;

    /// \def const AxisVisualPtr
    /// \brief Shared pointer to const AxisVisual
    typedef shared_ptr<const AxisVisual> ConstAxisVisualPtr;

    /// \def const CameraPtr
    /// \brief Shared pointer to const Camera
    typedef shared_ptr<const Camera> ConstCameraPtr;

    /// \def const DepthCameraPtr
    /// \brief Shared pointer to const DepthCamera
    typedef shared_ptr<const DepthCamera> ConstDepthCameraPtr;

    /// \def const ThermalCameraPtr
    /// \brief Shared pointer to const ThermalCamera
    typedef shared_ptr<const ThermalCamera> ConstThermalCameraPtr;

    /// \def const GpuRaysPtr
    /// \brief Shared pointer to const GpuRays
    typedef shared_ptr<const GpuRays> ConstGpuRaysPtr;

    /// \def const DirectionalLightPtr
    /// \brief Shared pointer to const DirectionalLight
    typedef shared_ptr<const DirectionalLight> ConstDirectionalLightPtr;

    /// \def const GaussianNoisePass
    /// \brief Shared pointer to const GaussianNoisePass
    typedef shared_ptr<const GaussianNoisePass> ConstGaussianNoisePass;

    /// \def const GeometryPtr
    /// \brief Shared pointer to const Geometry
    typedef shared_ptr<const Geometry> ConstGeometryPtr;

    /// \def const GizmoVisualPtr
    /// \brief Shared pointer to const GizmoVisual
    typedef shared_ptr<const GizmoVisual> ConstGizmoVisualPtr;

    /// \def const JointVisualPtr
    /// \brief Shared pointer to const JointVisual
    typedef shared_ptr<const JointVisual> ConstJointVisualPtr;

    /// \def const ImagePtr
    /// \brief Shared pointer to const Image
    typedef shared_ptr<const Image> ConstImagePtr;

    /// \def const LightPtr
    /// \brief Shared pointer to const Light
    typedef shared_ptr<const Light> ConstLightPtr;

    /// \def const LidarVisualPtr
    /// \brief Shared pointer to const LidarVisual
    typedef shared_ptr<const LidarVisual> ConstLidarVisualPtr;

    /// \def const MaterialPtr
    /// \brief Shared pointer to const Material
    typedef shared_ptr<const Material> ConstMaterialPtr;

    /// \def const MeshPtr
    /// \brief Shared pointer to const Mesh
    typedef shared_ptr<const Mesh> ConstMeshPtr;

    /// \def const NodePtr
    /// \brief Shared pointer to const Node
    typedef shared_ptr<const Node> ConstNodePtr;

    /// \def const ObjectPtr
    /// \brief Shared pointer to const Object
    typedef shared_ptr<const Object> ConstObjectPtr;

    /// \def const ObjectFactoryPtr
    /// \brief Shared pointer to const ObjectFactory
    typedef shared_ptr<const ObjectFactory> ConstObjectFactoryPtr;

    /// \def const ParticleEmitterPtr
    /// \brief Shared pointer to const ParticleEmitter
    typedef shared_ptr<const ParticleEmitter> ConstParticleEmitterPtr;

    /// \def const PointLightPtr
    /// \brief Shared pointer to const PointLight
    typedef shared_ptr<const PointLight> ConstPointLightPtr;

    /// \def RayQueryPtr
    /// \brief Shared pointer to RayQuery
    typedef shared_ptr<const RayQuery> ConstRayQueryPtr;

    /// \def const RenderPassPtr
    /// \brief Shared pointer to const RenderPass
    typedef shared_ptr<const RenderPass> ConstRenderPassPtr;

    /// \def const RenderPassSystemPtr
    /// \brief Shared pointer to const RenderPassSystem
    typedef shared_ptr<const RenderPassSystem> ConstRenderPassSystemPtr;

    /// \def const RenderTargetPtr
    /// \brief Shared pointer to const RenderTarget
    typedef shared_ptr<const RenderTarget> ConstRenderTargetPtr;

    /// \def const RenderTexturePtr
    /// \brief Shared pointer to const RenderTexture
    typedef shared_ptr<const RenderTexture> ConstRenderTexturePtr;

    /// \def const RenderWindowPtr
    /// \brief Shared pointer to const RenderWindow
    typedef shared_ptr<const RenderWindow> ConstRenderWindowPtr;

    /// \def const ScenePtr
    /// \brief Shared pointer to const Scene
    typedef shared_ptr<const Scene> ConstScenePtr;

    /// \def const SensorPtr
    /// \brief Shared pointer to const Sensor
    typedef shared_ptr<const Sensor> ConstSensorPtr;

    /// \brief Shared pointer to const ShaderParams
    typedef shared_ptr<const ShaderParams> ConstShaderParamsPtr;

    /// \def const SpotLightPtr
    /// \brief Shared pointer to const SpotLight
    typedef shared_ptr<const SpotLight> ConstSpotLightPtr;

    /// \def const SubMeshPtr
    /// \brief Shared pointer to const SubMesh
    typedef shared_ptr<const SubMesh> ConstSubMeshPtr;

    /// \def const SubMeshPtr
    /// \brief Shared pointer to const SubMesh
    typedef shared_ptr<const Text> ConstTextPtr;

    /// \def const VisualPtr
    /// \brief Shared pointer to const Visual
    typedef shared_ptr<const Visual> ConstVisualPtr;
    }
  }
}
#endif
