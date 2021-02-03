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
#ifndef IGNITION_RENDERING_OGRE_OGRESCENE_HH_
#define IGNITION_RENDERING_OGRE_OGRESCENE_HH_

#include <array>
#include <string>
#include "ignition/rendering/base/BaseScene.hh"
#include "ignition/rendering/ogre/Export.hh"
#include "ignition/rendering/ogre/OgreRenderTypes.hh"


namespace Ogre
{
  class Root;
  class SceneManager;
}

namespace ignition
{
  namespace rendering
  {
    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
    //
    class IGNITION_RENDERING_OGRE_VISIBLE OgreScene :
      public BaseScene
    {
      protected: OgreScene(unsigned int _id, const std::string &_name);

      public: virtual ~OgreScene();

      public: virtual void Fini();

      public: virtual RenderEngine *Engine() const;

      public: virtual VisualPtr RootVisual() const;

      public: virtual math::Color AmbientLight() const;

      public: virtual void SetAmbientLight(const math::Color &_color);

      public: virtual void SetBackgroundColor(const math::Color &_color);

      // Documentation inherited.
      public: virtual void SetGradientBackgroundColor(
                     const std::array<math::Color, 4> &_colors);

      // Documentation inherited.
      public: virtual void RemoveGradientBackgroundColor();

      public: virtual void PreRender();

      public: virtual void Clear();

      public: virtual void Destroy();

      public: virtual Ogre::SceneManager *OgreSceneManager() const;

      protected: virtual bool LoadImpl();

      protected: virtual bool InitImpl();

      protected: virtual DirectionalLightPtr CreateDirectionalLightImpl(
                     unsigned int _id, const std::string &_name);

      protected: virtual PointLightPtr CreatePointLightImpl(unsigned int _id,
                     const std::string &_name);

      protected: virtual SpotLightPtr CreateSpotLightImpl(unsigned int _id,
                     const std::string &_name);

      protected: virtual CameraPtr CreateCameraImpl(unsigned int _id,
                     const std::string &_name);

      // Documentation inherited
      protected: virtual DepthCameraPtr CreateDepthCameraImpl(
                     const unsigned int _id,
                     const std::string &_name);

      // Documentation inherited
      protected: virtual ThermalCameraPtr CreateThermalCameraImpl(
                     const unsigned int _id,
                     const std::string &_name);

      protected: virtual GpuRaysPtr CreateGpuRaysImpl(
                     const unsigned int _id,
                     const std::string &_name);

      protected: virtual VisualPtr CreateVisualImpl(unsigned int _id,
                     const std::string &_name);

      protected: virtual ArrowVisualPtr CreateArrowVisualImpl(unsigned int _id,
                     const std::string &_name);

      protected: virtual AxisVisualPtr CreateAxisVisualImpl(unsigned int _id,
                     const std::string &_name);

      // Documentation inherited
      protected: virtual GizmoVisualPtr CreateGizmoVisualImpl(unsigned int _id,
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
                     const std::string &_name, const std::string &_meshName);

      protected: virtual MeshPtr CreateMeshImpl(unsigned int _id,
                     const std::string &_name, const MeshDescriptor &_desc);

      // Documentation inherited
      protected: virtual HeightmapPtr CreateHeightmapImpl(unsigned int _id,
                 const std::string &_name, const HeightmapDescriptor &_desc);

      // Documentation inherited
      protected: virtual GridPtr CreateGridImpl(unsigned int _id,
                     const std::string &_name);

      // Documentation inherited
      protected: virtual MarkerPtr CreateMarkerImpl(unsigned int _id,
                     const std::string &_name);

      // Documentation inherited
      protected: virtual LidarVisualPtr CreateLidarVisualImpl(unsigned int _id,
                     const std::string &_name);

      // Documentation inherited
      protected: virtual WireBoxPtr CreateWireBoxImpl(unsigned int _id,
                     const std::string &_name);

      // Documentation inherited
      protected: virtual TextPtr CreateTextImpl(unsigned int _id,
                     const std::string &_name);

      protected: virtual MaterialPtr CreateMaterialImpl(unsigned int _id,
                     const std::string &_name);

      protected: virtual RenderTexturePtr CreateRenderTextureImpl(
                     unsigned int _id, const std::string &_name);

      // Documentation inherited.
      protected: virtual RenderWindowPtr CreateRenderWindowImpl(
                     unsigned int _id, const std::string &_name);

      protected: virtual RayQueryPtr CreateRayQueryImpl(
                     unsigned int _id, const std::string &_name);

      // Documentation inherited
      protected: virtual ParticleEmitterPtr CreateParticleEmitterImpl(
                     unsigned int _id, const std::string &_name);

      protected: virtual bool InitObject(OgreObjectPtr _object,
                     unsigned int _id, const std::string &_name);

      protected: virtual LightStorePtr Lights() const;

      protected: virtual SensorStorePtr Sensors() const;

      protected: virtual VisualStorePtr Visuals() const;

      protected: virtual MaterialMapPtr Materials() const;

      private: void CreateContext();

      private: void CreateRootVisual();

      private: void CreateMeshFactory();

      private: void CreateStores();

      private: OgreScenePtr SharedThis();

      protected: OgreVisualPtr rootVisual;

      protected: OgreMeshFactoryPtr meshFactory;

      protected: OgreLightStorePtr lights;

      protected: OgreSensorStorePtr sensors;

      protected: OgreVisualStorePtr visuals;

      protected: OgreMaterialMapPtr materials;

      protected: Ogre::Root *ogreRoot;

      protected: Ogre::SceneManager *ogreSceneManager;

      private: friend class OgreRenderEngine;
    };
    }
  }
}
#endif
