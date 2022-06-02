/*
 * Copyright (C) 2022 Open Source Robotics Foundation
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
#ifndef IGNITION_RENDERING_LUXCOREENGINE_LUXCOREENGINESCENE_HH_
#define IGNITION_RENDERING_LUXCOREENGINE_LUXCOREENGINESCENE_HH_

#include <luxcore/luxcore.h>

#include "ignition/rendering/base/BaseScene.hh"

#include "LuxCoreEngineCamera.hh"
#include "LuxCoreEngineLight.hh"
#include "LuxCoreEngineMaterial.hh"
#include "LuxCoreEngineMeshFactory.hh"
#include "LuxCoreEngineObject.hh"
#include "LuxCoreEngineRenderTypes.hh"
#include "LuxCoreEngineVisual.hh"

namespace ignition {
namespace rendering {
inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {

class LuxCoreEngineScene : public BaseScene {

  public: static void LogHandler(const char *msg);
  
  protected: LuxCoreEngineScene(unsigned int _id, const std::string &_name);
  
  public: virtual ~LuxCoreEngineScene();
  
  public: virtual void Fini();
  
  public: virtual RenderEngine *Engine() const;
  
  public: virtual VisualPtr RootVisual() const;
  
  public: virtual math::Color AmbientLight() const;
  
  public: virtual void SetAmbientLight(const math::Color &_color);
  
  protected: virtual DirectionalLightPtr
      CreateDirectionalLightImpl(unsigned int _id, const std::string &_name);
  
  protected: virtual PointLightPtr CreatePointLightImpl(unsigned int _id,
                                               const std::string &_name);
  
  protected: virtual SpotLightPtr CreateSpotLightImpl(unsigned int _id,
                                                      const std::string &_name);
  
  protected: virtual CameraPtr CreateCameraImpl(unsigned int _id,
                                                const std::string &_name);
  
  protected: virtual DepthCameraPtr
      CreateDepthCameraImpl(unsigned int _id, const std::string &_name)
                                                              override;
  
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
                                            const std::string &_name,
                                            const std::string &_meshName);
  
  protected: virtual MeshPtr CreateMeshImpl(unsigned int _id,
                                            const std::string &_name,
                                            const MeshDescriptor &_desc);
  
  protected: virtual CapsulePtr CreateCapsuleImpl(unsigned int _id,
                                                  const std::string &_name);
  
  protected: virtual GridPtr CreateGridImpl(unsigned int _id,
                                            const std::string &_name);
  
  protected: virtual MarkerPtr CreateMarkerImpl(unsigned int _id,
                                                const std::string &_name);
  
  protected: virtual LidarVisualPtr CreateLidarVisualImpl(unsigned int _id,
                                                 const std::string &_name);
  
  protected: virtual HeightmapPtr CreateHeightmapImpl(unsigned int _id,
                                             const std::string &_name,
                                             const HeightmapDescriptor &_desc);
  
  protected: virtual WireBoxPtr CreateWireBoxImpl(unsigned int _id,
                                                  const std::string &_name);
  
  protected: virtual MaterialPtr CreateMaterialImpl(unsigned int _id,
                                                    const std::string &_name);
  
  protected: virtual RenderTexturePtr CreateRenderTextureImpl(unsigned int _id,
                                                     const std::string &_name);
  
  protected: virtual RenderWindowPtr CreateRenderWindowImpl(unsigned int _id,
                                                   const std::string &_name);
  
  protected: virtual RayQueryPtr CreateRayQueryImpl(unsigned int _id,
                                           const std::string &_name);
  
  protected: virtual COMVisualPtr CreateCOMVisualImpl(unsigned int _id,
                                             const std::string &_name);
  
  protected: virtual InertiaVisualPtr CreateInertiaVisualImpl(unsigned int _id,
                                                     const std::string &_name);
  
  protected: virtual JointVisualPtr CreateJointVisualImpl(unsigned int _id,
                                                 const std::string &_name);
  
  protected: virtual LightStorePtr Lights() const;
  
  protected: virtual SensorStorePtr Sensors() const;
  
  protected: virtual VisualStorePtr Visuals() const;
  
  protected: virtual MaterialMapPtr Materials() const;
  
  protected: virtual bool LoadImpl();
  
  protected: virtual bool InitImpl();
  
  public: virtual bool IsInitialized();
  
  public: virtual unsigned int Id();
  
  public: virtual std::string Name();
  
  public: ignition::rendering::LightVisualPtr
      CreateLightVisualImpl(unsigned int _id, const std::string &_name);
  
  protected: void CreateMeshFactory();
  
  protected: void CreateStores();
  
  protected: bool InitObject(LuxCoreEngineObjectPtr _object, unsigned int _id,
                                                    const std::string &_name);
  
  private: LuxCoreEngineScenePtr SharedThis();
  
  public: luxcore::Scene *SceneLux();
  
  protected: luxcore::Scene *sceneLux;
  
  protected: unsigned int id_;
  
  protected: std::string name_;
  
  protected: LuxCoreEngineMeshFactoryPtr meshFactory;
  
  protected: LuxCoreEngineMaterialMapPtr materials;
  
  protected: LuxCoreEngineSensorStorePtr sensors;
  
  protected: LuxCoreEngineVisualStorePtr visuals;
  
  protected: LuxCoreEngineLightStorePtr lights;
  
  private: friend class LuxCoreEngineRenderEngine;
};

} // namespace IGNITION_RENDERING_VERSION_NAMESPACE
} // namespace rendering
} // namespace ignition

#endif
