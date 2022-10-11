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

  //Documentation Inherited
  public: static void LogHandler(const char *msg);
  
  /// @brief Constructor
  /// @param _id scene id
  /// @param _name scene name
  protected: LuxCoreEngineScene(unsigned int _id, const std::string &_name);
  
  public: virtual ~LuxCoreEngineScene();
  
  //Documentation inherited.
  public: virtual void Fini();
  
  //Documentation Inherited
  public: virtual RenderEngine *Engine() const override;
  
  //Documentation Inherited
  public: virtual VisualPtr RootVisual() const override;
  
  /// @brief Get the scene ambient light color.
  /// @return color black
  public: virtual math::Color AmbientLight() const;

  //Documentation Inherited
  public: virtual void SetAmbientLight(const math::Color &_color) override;
  
  /// @brief Creates directional light pointer, 
  ///        parses light from scene and sets its properties
  /// @param _id light id
  /// @param _name light name
  /// @return true is light object is initiated, false otherwise
  protected: virtual DirectionalLightPtr
      CreateDirectionalLightImpl(unsigned int _id, const std::string &_name);
  
  /// @brief Creates point light, parses it in the scene and sets its properties 
  /// @param _id light id
  /// @param _name light name
  /// @return true if light is generated, false otherwise
  protected: virtual PointLightPtr CreatePointLightImpl(unsigned int _id,
                                               const std::string &_name);
  

  //Documentation Inherited
  protected: virtual SpotLightPtr CreateSpotLightImpl(unsigned int _id,
                                                      const std::string &_name) override;
  
  /// @brief Creates camera, parses it in the scene and sets its properties 
  /// @param _id camera id
  /// @param _name camera name
  /// @return true if camera is created, false otherwise
  protected: virtual CameraPtr CreateCameraImpl(unsigned int _id,
                                                const std::string &_name);
  
  //Documentation inherited
  protected: virtual DepthCameraPtr
      CreateDepthCameraImpl(unsigned int _id, const std::string &_name) override;
  /// @brief Creates visual, parses it in the scene and sets its properties 
  /// @param _id camera id
  /// @param _name camera name
  /// @return true if camera is created, false otherwise
  protected: virtual VisualPtr CreateVisualImpl(unsigned int _id,
                                                const std::string &_name);
  //Documentation inherited
  protected: virtual ArrowVisualPtr CreateArrowVisualImpl(unsigned int _id,
                                                 const std::string &_name) override;
  //Documentation inherited
  protected: virtual AxisVisualPtr CreateAxisVisualImpl(unsigned int _id,
                                               const std::string &_name) override;
  /// @brief Creates box implementation scene
  /// @param _id box id
  /// @param _name box name
  /// @return geometry pointer to box mesh implementation 
  protected: virtual GeometryPtr CreateBoxImpl(unsigned int _id,
                                               const std::string &_name);
  //Documentation inherited
  protected: virtual GeometryPtr CreateConeImpl(unsigned int _id,
                                                const std::string &_name) override;
  //Documentation inherited
  protected: virtual GeometryPtr CreateCylinderImpl(unsigned int _id,
                                                    const std::string &_name) override;
  /// @brief Creates plane implementaion in the scene
  /// @param _id plane id
  /// @param _name plane name
  /// @return geometry pointer to plane mesh implementation
  protected: virtual GeometryPtr CreatePlaneImpl(unsigned int _id,
                                                 const std::string &_name);
  //Documentation inherited
  protected: virtual GeometryPtr CreateSphereImpl(unsigned int _id,
                                                  const std::string &_name) override;
  //Documentation inherited
  protected: virtual MeshPtr CreateMeshImpl(unsigned int _id,
                                            const std::string &_name,
                                            const std::string &_meshName);
  /// @brief create mesh descriptor and implement mesh in the scene
  /// @param _id mesh id
  /// @param _name mesh name
  /// @param _desc mesh descriptor
  /// @return geometry pointer to mesh implementation
  protected: virtual MeshPtr CreateMeshImpl(unsigned int _id,
                                            const std::string &_name,
                                            const MeshDescriptor &_desc);
  //Documentation inherited
  protected: virtual CapsulePtr CreateCapsuleImpl(unsigned int _id,
                                                  const std::string &_name) override;
  //Documentation inherited
  protected: virtual GridPtr CreateGridImpl(unsigned int _id,
                                            const std::string &_name) override;
  //Documentation inherited
  protected: virtual MarkerPtr CreateMarkerImpl(unsigned int _id,
                                                const std::string &_name) override;
  //Documentation inherited
  protected: virtual LidarVisualPtr CreateLidarVisualImpl(unsigned int _id,
                                                 const std::string &_name) override;
  //Documentation inherited
  protected: virtual HeightmapPtr CreateHeightmapImpl(unsigned int _id,
                                             const std::string &_name,
                                             const HeightmapDescriptor &_desc) override;
  //Documentation inherited
  protected: virtual WireBoxPtr CreateWireBoxImpl(unsigned int _id,
                                                  const std::string &_name) override;
  /// @brief Generates material pointer, parses it in the scene and sets its property
  /// @param _id material id
  /// @param _name material name
  /// @return true if material is created, false otherwise
  protected: virtual MaterialPtr CreateMaterialImpl(unsigned int _id,
                                                    const std::string &_name);
  //Documentation inherited
  protected: virtual RenderTexturePtr CreateRenderTextureImpl(unsigned int _id,
                                                     const std::string &_name) override;
  //Documentation inherited
  protected: virtual RenderWindowPtr CreateRenderWindowImpl(unsigned int _id,
                                                   const std::string &_name) override;
  //Documentation inherited
  protected: virtual RayQueryPtr CreateRayQueryImpl(unsigned int _id,
                                           const std::string &_name) override;
  //Documentation inherited
  protected: virtual COMVisualPtr CreateCOMVisualImpl(unsigned int _id,
                                             const std::string &_name) override;
  //Documentation inherited
  protected: virtual InertiaVisualPtr CreateInertiaVisualImpl(unsigned int _id,
                                                     const std::string &_name) override;
  //Documentation inherited
  protected: virtual JointVisualPtr CreateJointVisualImpl(unsigned int _id,
                                                 const std::string &_name)override;
  /// @brief Get lights in the scene
  /// @return pointer to lights
  protected: virtual LightStorePtr Lights() const;

  /// @brief Get sensors in the scene
  /// @return pointer to sensors
  protected: virtual SensorStorePtr Sensors() const;
  
  /// @brief Get visuals in the scene
  /// @return pointer to visuals
  protected: virtual VisualStorePtr Visuals() const;
  
  /// @brief Get Materials in the scene
  /// @return pointer to materials
  protected: virtual MaterialMapPtr Materials() const;
  /// @brief Load implementation
  /// @return true 
  protected: virtual bool LoadImpl();
  /// @brief Initialize log handler and create a sccene along with storage and mesh
  /// @return true when successful
  protected: virtual bool InitImpl();
  
  /// @brief Check if scene is initialized
  /// @return true 
  public: virtual bool IsInitialized();
  
  /// @brief Get id of the scene
  /// @return id of the scene
  public: virtual unsigned int Id();
  
  /// @brief Get name of the scene
  /// @return name of the scene
  public: virtual std::string Name();
  
  //Documentation inherited
  public: ignition::rendering::LightVisualPtr
      CreateLightVisualImpl(unsigned int _id, const std::string &_name);
  /// @brief Create mesh factory
  protected: void CreateMeshFactory();
  /// @brief Create lights, sensors, visuals and material stores
  protected: void CreateStores();
  
  /// @brief Initialize object by assigning variables and loading it
  /// @param _object object pointer 
  /// @param _id  object id
  /// @param _name object name
  /// @return true when initialized
  protected: bool InitObject(LuxCoreEngineObjectPtr _object, unsigned int _id,
                                                    const std::string &_name);
  /// @brief Get scene from the shared pointer
  /// @return return scene pointer
  private: LuxCoreEngineScenePtr SharedThis();
  /// @brief Get LuxCore scene
  /// @return lux scene
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
