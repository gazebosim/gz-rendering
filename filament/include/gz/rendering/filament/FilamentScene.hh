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

#include <set>
#include <string>
#include <vector>

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

    class GZ_RENDERING_FILAMENT_VISIBLE FilamentScene :
      public BaseScene
    {
      protected: FilamentScene(unsigned int _id, const std::string &_name);

      public: virtual ~FilamentScene();

      public: virtual void Fini() override;

      public: virtual void Destroy() override;

      public: virtual RenderEngine *Engine() const override;

      public: virtual VisualPtr RootVisual() const override;

      public: virtual math::Color AmbientLight() const override;

      /// \brief GPU picking through the camera. FilamentRayQuery casts a
      /// ray against the meshes instead.
      public: virtual VisualPtr VisualAt(const CameraPtr &_camera,
          const math::Vector2i &_mousePos) override;

      public: virtual void SetAmbientLight(const math::Color &_color) override;

      // Documentation inherited.
      public: virtual void SetSkyEnabled(bool _enabled) override;

      // Documentation inherited.
      // Also switches the quality mode's sky lighting (UpdateSkyLight).
      public: virtual void PreRender() override;

      // Documentation inherited.
      public: virtual bool SkyEnabled() const override;

      // Documentation inherited. Only directional lights, like ogre2.
      public: virtual bool SetShadowTextureSize(LightType _lightType,
          unsigned int _textureSize) override;

      // Documentation inherited.
      public: virtual unsigned int ShadowTextureSize(
          LightType _lightType) const override;

      /// \brief Native filament scene
      public: filament::Scene *Native() const;

      /// \brief Meshes currently alive in this scene; used by sensors that
      /// need to render every mesh with an override material.
      public: const std::set<FilamentMesh *> &Meshes() const;

      public: void RegisterMesh(FilamentMesh *_mesh);

      /// \brief Create a mesh whose vertices are rebuilt on change (lines,
      /// points, triangles), for markers, wire boxes and grids
      public: std::shared_ptr<FilamentDynamicMesh> CreateDynamicMesh();

      public: void UnregisterMesh(FilamentMesh *_mesh);

      /// \brief Particle emitters, in creation order (the sensors' particle
      /// effects use the first one with live particles, like ogre2)
      public: const std::vector<FilamentParticleEmitter *> &
          ParticleEmitters() const;

      /// \brief Add an emitter to ParticleEmitters()
      public: void RegisterParticleEmitter(FilamentParticleEmitter *_emitter);

      /// \brief Remove an emitter from ParticleEmitters()
      public: void UnregisterParticleEmitter(
          FilamentParticleEmitter *_emitter);

      /// \brief Sort the particles of every emitter back to front along
      /// _dir, a camera's forward vector
      public: void SortParticles(const math::Vector3d &_dir);

      /// \brief Projectors of this scene
      public: const std::set<FilamentProjector *> &Projectors() const;

      public: void RegisterProjector(FilamentProjector *_projector);

      public: void UnregisterProjector(FilamentProjector *_projector);

      protected: virtual ProjectorPtr CreateProjectorImpl(unsigned int _id,
                     const std::string &_name) override;

      protected: virtual DirectionalLightPtr CreateDirectionalLightImpl(
                     unsigned int _id, const std::string &_name) override;

      protected: virtual PointLightPtr CreatePointLightImpl(unsigned int _id,
                     const std::string &_name) override;

      protected: virtual SpotLightPtr CreateSpotLightImpl(unsigned int _id,
                     const std::string &_name) override;

      protected: virtual CameraPtr CreateCameraImpl(unsigned int _id,
                     const std::string &_name) override;

      protected: virtual DepthCameraPtr CreateDepthCameraImpl(unsigned int _id,
                     const std::string &_name) override;

      protected: virtual ThermalCameraPtr CreateThermalCameraImpl(
                     unsigned int _id, const std::string &_name) override;

      protected: virtual BoundingBoxCameraPtr CreateBoundingBoxCameraImpl(
          unsigned int _id, const std::string &_name) override;

      protected: virtual SegmentationCameraPtr CreateSegmentationCameraImpl(
                     unsigned int _id, const std::string &_name) override;

      protected: virtual GpuRaysPtr CreateGpuRaysImpl(unsigned int _id,
                     const std::string &_name) override;

      protected: virtual WideAngleCameraPtr CreateWideAngleCameraImpl(
                     unsigned int _id, const std::string &_name) override;

      protected: virtual VisualPtr CreateVisualImpl(unsigned int _id,
                     const std::string &_name) override;

      protected: virtual ArrowVisualPtr CreateArrowVisualImpl(unsigned int _id,
                     const std::string &_name) override;

      protected: virtual AxisVisualPtr CreateAxisVisualImpl(unsigned int _id,
                     const std::string &_name) override;

      protected: virtual GeometryPtr CreateBoxImpl(unsigned int _id,
                     const std::string &_name) override;

      protected: virtual GeometryPtr CreateConeImpl(unsigned int _id,
                     const std::string &_name) override;

      protected: virtual GeometryPtr CreateCylinderImpl(unsigned int _id,
                     const std::string &_name) override;

      protected: virtual GeometryPtr CreatePlaneImpl(unsigned int _id,
                     const std::string &_name) override;

      protected: virtual GeometryPtr CreateSphereImpl(unsigned int _id,
                     const std::string &_name) override;

      protected: virtual MeshPtr CreateMeshImpl(unsigned int _id,
                     const std::string &_name,
                     const std::string &_meshName);

      protected: virtual MeshPtr CreateMeshImpl(unsigned int _id,
                     const std::string &_name,
                     const MeshDescriptor &_desc) override;

      protected: virtual CapsulePtr CreateCapsuleImpl(unsigned int _id,
                     const std::string &_name) override;

      protected: virtual GridPtr CreateGridImpl(unsigned int _id,
                     const std::string &_name) override;

      protected: virtual MarkerPtr CreateMarkerImpl(unsigned int _id,
                     const std::string &_name) override;

      protected: virtual LidarVisualPtr CreateLidarVisualImpl(unsigned int _id,
                     const std::string &_name) override;

      protected: virtual FrustumVisualPtr CreateFrustumVisualImpl(
                     unsigned int _id, const std::string &_name) override;

      protected: virtual HeightmapPtr CreateHeightmapImpl(unsigned int _id,
                     const std::string &_name,
                     const HeightmapDescriptor &_desc) override;

      protected: virtual WireBoxPtr CreateWireBoxImpl(unsigned int _id,
                     const std::string &_name) override;

      protected: virtual MaterialPtr CreateMaterialImpl(unsigned int _id,
                     const std::string &_name) override;

      protected: virtual RenderTexturePtr CreateRenderTextureImpl(
                     unsigned int _id, const std::string &_name) override;

      protected: virtual RenderWindowPtr CreateRenderWindowImpl(
                     unsigned int _id, const std::string &_name) override;

      protected: virtual ParticleEmitterPtr CreateParticleEmitterImpl(
                     unsigned int _id, const std::string &_name) override;

      protected: virtual RayQueryPtr CreateRayQueryImpl(
                     unsigned int _id, const std::string &_name) override;

      protected: virtual COMVisualPtr CreateCOMVisualImpl(unsigned int _id,
                     const std::string &_name) override;

      protected: virtual InertiaVisualPtr CreateInertiaVisualImpl(
                     unsigned int _id, const std::string &_name) override;

      protected: virtual JointVisualPtr CreateJointVisualImpl(unsigned int _id,
                     const std::string &_name) override;

      protected: virtual LightVisualPtr CreateLightVisualImpl(unsigned int _id,
                     const std::string &_name) override;

      protected: virtual GizmoVisualPtr CreateGizmoVisualImpl(unsigned int _id,
                     const std::string &_name) override;

      protected: virtual LightStorePtr Lights() const override;

      protected: virtual SensorStorePtr Sensors() const override;

      protected: virtual VisualStorePtr Visuals() const override;

      protected: virtual MaterialMapPtr Materials() const override;

      protected: virtual bool LoadImpl() override;

      protected: virtual bool InitImpl() override;

      protected: bool InitObject(FilamentObjectPtr _object, unsigned int _id,
                     const std::string &_name);

      protected: void CreateRootVisual();

      protected: void CreateStores();

      private: FilamentScenePtr SharedThis();

      /// \brief Quality mode (the default; GZ_FILAMENT_QUALITY=0 keeps
      /// ogre2's lighting): the sky, when enabled, lights the scene (image
      /// based lighting): its prefiltered cube map as reflections and its
      /// irradiance (3-band SH, scaled to the ambient light's level) as the
      /// ambient term, instead of SetAmbientLight's uniform ambient with
      /// black reflections. Every camera of this filament::Scene sees it;
      /// sensors that draw with unlit override materials (depth, thermal,
      /// segmentation, lidar) are unaffected.
      private: void UpdateSkyLight();

      protected: filament::Scene *scene{nullptr};

      protected: filament::IndirectLight *ambientLight{nullptr};

      /// \brief Skybox drawn when the sky is enabled
      protected: filament::Skybox *skybox{nullptr};

      /// \brief Sky lighting of the quality mode (see UpdateSkyLight),
      /// rebuilt when the ambient light changes
      private: filament::IndirectLight *skyLight{nullptr};

      /// \brief Prefiltered sky cube map (reflections of skyLight)
      private: filament::Texture *skyReflections{nullptr};

      /// \brief Sky irradiance, 3-band SH in filament's shader form
      private: float skySh[9][3]{};

      /// \brief Cube map file of the skybox (see SetSkyEnabled)
      private: std::string skyCubemap;

      protected: math::Color ambient;

      /// \brief Shadow map size of the directional lights
      protected: unsigned int dirTexSize{2048u};

      protected: FilamentVisualPtr rootVisual;

      protected: FilamentMeshFactoryPtr meshFactory;

      protected: FilamentMaterialMapPtr materials;

      protected: FilamentSensorStorePtr sensors;

      protected: FilamentVisualStorePtr visuals;

      protected: FilamentLightStorePtr lights;

      protected: std::set<FilamentMesh *> meshes;

      protected: std::vector<FilamentParticleEmitter *> particleEmitters;

      protected: std::set<FilamentProjector *> projectors;

      private: friend class FilamentRenderEngine;
      private: friend class FilamentMesh;

      private: friend class FilamentParticleEmitter;
    };
    }
  }
}
#endif
