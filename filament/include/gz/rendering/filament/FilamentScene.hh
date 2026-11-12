/*
 * Copyright (C) 2026 Open Source Robotics Foundation
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
#ifndef GZ_RENDERING_FILAMENT_FILAMENTSCENE_HH_
#define GZ_RENDERING_FILAMENT_FILAMENTSCENE_HH_

#include <memory>
#include <set>
#include <string>
#include <vector>

#include <math/vec3.h>

#include "gz/rendering/config.hh"
#include "gz/rendering/base/BaseScene.hh"
#include "gz/rendering/filament/FilamentRenderTypes.hh"
#include "gz/rendering/filament/Export.hh"

namespace filament
{
  class IndirectLight;
  class Skybox;
  class Scene;
  class Texture;
}

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    //
    class FilamentDynamicMesh;
    class FilamentProjector;
    namespace filament_probe { class ReflectionProbe; }

    /// \brief Filament implementation of the scene class.
    class GZ_RENDERING_FILAMENT_VISIBLE FilamentScene :
      public BaseScene
    {
      /// \brief Constructor
      /// \param[in] _id Unique scene id
      /// \param[in] _name Scene name
      protected: FilamentScene(unsigned int _id, const std::string &_name);

      /// \brief Destructor
      public: virtual ~FilamentScene();

      /// \brief Completely destroy the scene and all its resources,
      /// including its filament objects: the reflection probe, the sky and
      /// ambient lighting, the skybox and the filament::Scene. Meshes
      /// detached from every visual but still referenced by the user are
      /// destroyed too, like ogre2 does with its scene manager.
      public: virtual void Destroy() override;

      // Documentation inherited
      public: virtual RenderEngine *Engine() const override;

      // Documentation inherited
      public: virtual VisualPtr RootVisual() const override;

      // Documentation inherited
      public: virtual math::Color AmbientLight() const override;

      /// \brief Get the visual at a mouse position, found by GPU picking
      /// through the camera. FilamentRayQuery casts a ray against the
      /// meshes instead.
      /// \param[in] _camera Camera being used
      /// \param[in] _mousePos Position of the mouse in pixels
      /// \return The visual, null if none was found.
      public: virtual VisualPtr VisualAt(const CameraPtr &_camera,
          const math::Vector2i &_mousePos) override;

      // Documentation inherited
      public: virtual void SetAmbientLight(const math::Color &_color) override;

      // Documentation inherited.
      public: virtual void SetSkyEnabled(bool _enabled) override;

      /// \brief Prepare the scene for rendering, as BaseScene does, after
      /// running filament's garbage collection
      /// (FilamentRenderEngine::CollectGarbage), then update the sky
      /// lighting (UpdateSkyLight).
      public: virtual void PreRender() override;

      // Documentation inherited.
      public: virtual bool SkyEnabled() const override;

      /// \brief Set the shadow texture size of the directional lights; other
      /// light types are not supported, like in ogre2. The size must be a
      /// power of two, at least 512 and at most the GPU's maximum texture
      /// size (capped at 16384).
      /// \param[in] _lightType Light type, only LightType::DIRECTIONAL
      /// \param[in] _textureSize Shadow texture size in pixels
      /// \return True if the size was set, false (with an error
      /// logged) if it was rejected.
      public: virtual bool SetShadowTextureSize(LightType _lightType,
          unsigned int _textureSize) override;

      // Documentation inherited.
      public: virtual unsigned int ShadowTextureSize(
          LightType _lightType) const override;

      /// \brief Get the native filament scene.
      /// \return The filament::Scene, owned by this scene: null before
      /// initialisation and after Fini.
      public: filament::Scene *Native() const;

      /// \brief Meshes currently alive in this scene; used by sensors that
      /// need to render every mesh with an override material.
      /// \return The registered meshes, not owned.
      public: const std::set<FilamentMesh *> &Meshes() const;

      /// \brief Add a mesh to Meshes().
      /// \param[in] _mesh Mesh to add, not owned
      public: void RegisterMesh(FilamentMesh *_mesh);

      /// \brief Create a mesh whose vertices are rebuilt on change (lines,
      /// points, triangles), for markers, wire boxes, grids and the
      /// visuals made of lines.
      /// \return The new mesh
      public: std::shared_ptr<FilamentDynamicMesh> CreateDynamicMesh();

      /// \brief Remove a mesh from Meshes().
      /// \param[in] _mesh Mesh to remove
      public: void UnregisterMesh(FilamentMesh *_mesh);

      /// \brief Get the particle emitters, in creation order. The sensors'
      /// particle effects use the first one with live particles, like
      /// ogre2.
      /// \return The registered emitters, not owned.
      public: const std::vector<FilamentParticleEmitter *> &
          ParticleEmitters() const;

      /// \brief Add an emitter to ParticleEmitters()
      /// \param[in] _emitter Emitter to add, not owned
      public: void RegisterParticleEmitter(FilamentParticleEmitter *_emitter);

      /// \brief Remove an emitter from ParticleEmitters()
      /// \param[in] _emitter Emitter to remove
      public: void UnregisterParticleEmitter(
          FilamentParticleEmitter *_emitter);

      /// \brief Get the projectors of this scene.
      /// \return The registered projectors, not owned.
      public: const std::set<FilamentProjector *> &Projectors() const;

      /// \brief Add a projector to Projectors().
      /// \param[in] _projector Projector to add, not owned
      public: void RegisterProjector(FilamentProjector *_projector);

      /// \brief Remove a projector from Projectors().
      /// \param[in] _projector Projector to remove
      public: void UnregisterProjector(FilamentProjector *_projector);

      // Documentation inherited
      protected: virtual ProjectorPtr CreateProjectorImpl(unsigned int _id,
                     const std::string &_name) override;

      // Documentation inherited
      protected: virtual DirectionalLightPtr CreateDirectionalLightImpl(
                     unsigned int _id, const std::string &_name) override;

      // Documentation inherited
      protected: virtual PointLightPtr CreatePointLightImpl(unsigned int _id,
                     const std::string &_name) override;

      // Documentation inherited
      protected: virtual SpotLightPtr CreateSpotLightImpl(unsigned int _id,
                     const std::string &_name) override;

      // Documentation inherited
      protected: virtual CameraPtr CreateCameraImpl(unsigned int _id,
                     const std::string &_name) override;

      // Documentation inherited
      protected: virtual DepthCameraPtr CreateDepthCameraImpl(unsigned int _id,
                     const std::string &_name) override;

      // Documentation inherited
      protected: virtual ThermalCameraPtr CreateThermalCameraImpl(
                     unsigned int _id, const std::string &_name) override;

      // Documentation inherited
      protected: virtual BoundingBoxCameraPtr CreateBoundingBoxCameraImpl(
          unsigned int _id, const std::string &_name) override;

      // Documentation inherited
      protected: virtual SegmentationCameraPtr CreateSegmentationCameraImpl(
                     unsigned int _id, const std::string &_name) override;

      // Documentation inherited
      protected: virtual GpuRaysPtr CreateGpuRaysImpl(unsigned int _id,
                     const std::string &_name) override;

      // Documentation inherited
      protected: virtual WideAngleCameraPtr CreateWideAngleCameraImpl(
                     unsigned int _id, const std::string &_name) override;

      // Documentation inherited
      protected: virtual VisualPtr CreateVisualImpl(unsigned int _id,
                     const std::string &_name) override;

      // Documentation inherited
      protected: virtual ArrowVisualPtr CreateArrowVisualImpl(unsigned int _id,
                     const std::string &_name) override;

      // Documentation inherited
      protected: virtual AxisVisualPtr CreateAxisVisualImpl(unsigned int _id,
                     const std::string &_name) override;

      // Documentation inherited
      protected: virtual GeometryPtr CreateBoxImpl(unsigned int _id,
                     const std::string &_name) override;

      // Documentation inherited
      protected: virtual GeometryPtr CreateConeImpl(unsigned int _id,
                     const std::string &_name) override;

      // Documentation inherited
      protected: virtual GeometryPtr CreateCylinderImpl(unsigned int _id,
                     const std::string &_name) override;

      // Documentation inherited
      protected: virtual GeometryPtr CreatePlaneImpl(unsigned int _id,
                     const std::string &_name) override;

      // Documentation inherited
      protected: virtual GeometryPtr CreateSphereImpl(unsigned int _id,
                     const std::string &_name) override;

      /// \brief Create a mesh from the name of a mesh of the
      /// common::MeshManager, as MeshDescriptor does. The box, cone,
      /// cylinder, plane and sphere are the meshes "unit_box",
      /// "unit_cone", "unit_cylinder", "unit_plane" and
      /// "unit_sphere".
      /// \param[in] _id Unique id of the mesh
      /// \param[in] _name Name of the mesh
      /// \param[in] _meshName Name of the mesh to create
      /// \return The mesh, null on failure
      protected: virtual MeshPtr CreateMeshImpl(unsigned int _id,
                     const std::string &_name,
                     const std::string &_meshName);

      // Documentation inherited
      protected: virtual MeshPtr CreateMeshImpl(unsigned int _id,
                     const std::string &_name,
                     const MeshDescriptor &_desc) override;

      // Documentation inherited
      protected: virtual CapsulePtr CreateCapsuleImpl(unsigned int _id,
                     const std::string &_name) override;

      // Documentation inherited
      protected: virtual GridPtr CreateGridImpl(unsigned int _id,
                     const std::string &_name) override;

      // Documentation inherited
      protected: virtual MarkerPtr CreateMarkerImpl(unsigned int _id,
                     const std::string &_name) override;

      // Documentation inherited
      protected: virtual LidarVisualPtr CreateLidarVisualImpl(unsigned int _id,
                     const std::string &_name) override;

      // Documentation inherited
      protected: virtual FrustumVisualPtr CreateFrustumVisualImpl(
                     unsigned int _id, const std::string &_name) override;

      // Documentation inherited
      protected: virtual HeightmapPtr CreateHeightmapImpl(unsigned int _id,
                     const std::string &_name,
                     const HeightmapDescriptor &_desc) override;

      // Documentation inherited
      protected: virtual WireBoxPtr CreateWireBoxImpl(unsigned int _id,
                     const std::string &_name) override;

      // Documentation inherited
      protected: virtual MaterialPtr CreateMaterialImpl(unsigned int _id,
                     const std::string &_name) override;

      // Documentation inherited
      protected: virtual RenderTexturePtr CreateRenderTextureImpl(
                     unsigned int _id, const std::string &_name) override;

      /// \brief Render windows are not supported: always returns null.
      /// \param[in] _id Unused
      /// \param[in] _name Unused
      /// \return Always null
      protected: virtual RenderWindowPtr CreateRenderWindowImpl(
                     unsigned int _id, const std::string &_name) override;

      // Documentation inherited
      protected: virtual ParticleEmitterPtr CreateParticleEmitterImpl(
                     unsigned int _id, const std::string &_name) override;

      // Documentation inherited
      protected: virtual RayQueryPtr CreateRayQueryImpl(
                     unsigned int _id, const std::string &_name) override;

      // Documentation inherited
      protected: virtual COMVisualPtr CreateCOMVisualImpl(unsigned int _id,
                     const std::string &_name) override;

      // Documentation inherited
      protected: virtual InertiaVisualPtr CreateInertiaVisualImpl(
                     unsigned int _id, const std::string &_name) override;

      // Documentation inherited
      protected: virtual JointVisualPtr CreateJointVisualImpl(unsigned int _id,
                     const std::string &_name) override;

      // Documentation inherited
      protected: virtual LightVisualPtr CreateLightVisualImpl(unsigned int _id,
                     const std::string &_name) override;

      // Documentation inherited
      protected: virtual GizmoVisualPtr CreateGizmoVisualImpl(unsigned int _id,
                     const std::string &_name) override;

      /// \brief Get the store of the scene's lights.
      /// \return The light store
      protected: virtual LightStorePtr Lights() const override;

      /// \brief Get the store of the scene's sensors.
      /// \return The sensor store
      protected: virtual SensorStorePtr Sensors() const override;

      /// \brief Get the store of the scene's visuals.
      /// \return The visual store
      protected: virtual VisualStorePtr Visuals() const override;

      /// \brief Get the map of the scene's materials.
      /// \return The material map
      protected: virtual MaterialMapPtr Materials() const override;

      /// \brief Load the scene-specific resources: there are none.
      /// \return True
      protected: virtual bool LoadImpl() override;

      /// \brief Create the filament::Scene, the stores, the root visual and
      /// the mesh factory.
      /// \return True on success, false if the filament engine is not
      /// initialised.
      protected: virtual bool InitImpl() override;

      /// \brief Light the scene with reflections of the scene around _pos,
      /// for the render of the quality-mode GUI camera: a cube map is
      /// rendered there (see filament_probe::ReflectionProbe) and used as
      /// the IBL's reflections, with the scene IBL's irradiance. Other
      /// cameras keep the scene IBL. Does nothing while the render engine
      /// has a frame open (FilamentRenderEngine::InFrame). Pair with
      /// EndReflections().
      /// \param[in] _pos Position the cube map is rendered from
      /// \param[in] _visibilityMask The camera's visibility mask
      /// \param[in] _near Near clip distance of the cube map's views
      /// \param[in] _far Far clip distance of the cube map's views
      public: void BeginReflections(const math::Vector3d &_pos,
          uint32_t _visibilityMask, double _near, double _far);

      /// \brief Hash of the scene state the reflections depend on.
      /// \param[in] _pos Position of the cube map
      /// \return FNV-1a hash of _pos, the world transforms and layers of
      /// the meshes, and the lights
      private: uint64_t ReflectionsSignature(const math::Vector3d &_pos) const;

      /// \brief Restore the scene IBL after BeginReflections().
      public: void EndReflections();

      /// \brief Give an object its id, name and scene, then load and
      /// initialise it.
      /// \param[in] _object Object to set up
      /// \param[in] _id Unique id to give the object
      /// \param[in] _name Name to give the object
      /// \return True, always: Load and Init report no failure
      protected: bool InitObject(FilamentObjectPtr _object, unsigned int _id,
                     const std::string &_name);

      /// \brief Create the root visual, if there is none.
      protected: void CreateRootVisual();

      /// \brief Create the light, sensor, visual and material stores.
      protected: void CreateStores();

      /// \brief Get a shared pointer to this scene.
      /// \return This scene as a FilamentScenePtr
      private: FilamentScenePtr SharedThis();

      /// \brief Sky lighting: the sky, when enabled, lights the scene (image
      /// based lighting): its prefiltered cube map as reflections and its
      /// irradiance (3-band SH, scaled to the ambient light's level) as the
      /// ambient term, instead of SetAmbientLight's uniform ambient with
      /// black reflections. Every camera of this filament::Scene sees it;
      /// sensors that draw with unlit override materials (depth, thermal,
      /// segmentation, lidar) are unaffected.
      private: void UpdateSkyLight();

      /// \brief Irradiance of the sky light: the sky's directional
      /// irradiance in the ambient light's colour and level, 3-band SH.
      /// \param[out] _sh The 9 SH coefficients, in filament's shader form
      private: void SkyLightSh(filament::math::float3 _sh[9]) const;

      /// \brief The native filament scene, created by InitImpl (see Native)
      protected: filament::Scene *scene{nullptr};

      /// \brief Uniform ambient light built by SetAmbientLight, null until
      /// then
      protected: filament::IndirectLight *ambientLight{nullptr};

      /// \brief Skybox drawn when the sky is enabled
      protected: filament::Skybox *skybox{nullptr};

      /// \brief Sky lighting (see UpdateSkyLight), rebuilt when the ambient
      /// light changes
      private: filament::IndirectLight *skyLight{nullptr};

      /// \brief Prefiltered sky cube map (reflections of skyLight)
      private: filament::Texture *skyReflections{nullptr};

      /// \brief Sky irradiance, 3-band SH in filament's shader form
      private: float skySh[9][3]{};

      /// \brief Cube map file of the skybox (see SetSkyEnabled)
      private: std::string skyCubemap;

      /// \brief Ambient light color, as last set by SetAmbientLight
      protected: math::Color ambient;

      /// \brief Scene reflections of the GUI camera (BeginReflections)
      private: std::unique_ptr<filament_probe::ReflectionProbe> probe;

      /// \brief Scene IBL during BeginReflections
      private: filament::IndirectLight *probeSavedLight{nullptr};

      /// \brief The sky at the ambient light's level, drawn by the
      /// reflection probe instead of the skybox
      private: filament::Skybox *probeSkybox{nullptr};

      /// \brief Cube map of probeSkybox
      private: filament::Texture *probeSkyTexture{nullptr};

      /// \brief Destroy the sky light, its reflections and probeSkybox
      /// (rebuilt lazily by UpdateSkyLight)
      private: void DestroySkyLight();

      /// \brief Between BeginReflections and EndReflections
      private: bool probeActive{false};

      /// \brief Shadow map size of the directional lights
      protected: unsigned int dirTexSize{2048u};

      /// \brief Root of the scene graph (see RootVisual)
      protected: FilamentVisualPtr rootVisual;

      /// \brief Builds the renderables of the meshes loaded by CreateMeshImpl
      protected: FilamentMeshFactoryPtr meshFactory;

      /// \brief Materials of the scene (see Materials)
      protected: FilamentMaterialMapPtr materials;

      /// \brief Sensors of the scene (see Sensors)
      protected: FilamentSensorStorePtr sensors;

      /// \brief Visuals of the scene (see Visuals)
      protected: FilamentVisualStorePtr visuals;

      /// \brief Lights of the scene (see Lights)
      protected: FilamentLightStorePtr lights;

      /// \brief Meshes alive in this scene, not owned (see Meshes)
      protected: std::set<FilamentMesh *> meshes;

      /// \brief Particle emitters in creation order, not owned (see
      /// ParticleEmitters)
      protected: std::vector<FilamentParticleEmitter *> particleEmitters;

      /// \brief Projectors of this scene, not owned (see Projectors)
      protected: std::set<FilamentProjector *> projectors;

      private: friend class FilamentRenderEngine;
      private: friend class FilamentMesh;

    };
    }
  }
}
#endif
