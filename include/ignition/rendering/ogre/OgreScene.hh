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

#include <string>
#include "ignition/rendering/base/BaseScene.hh"
#include "ignition/rendering/ogre/OgreRenderTypes.hh"
#include "ignition/rendering/ogre/OgreIncludes.hh"

namespace Ogre
{
  class Root;
  class SceneManager;
  class raySceneQuery;
}

namespace ignition
{
  namespace rendering
  {
    class IGNITION_VISIBLE OgreScene :
      public BaseScene
    {
      protected: OgreScene(unsigned int _id, const std::string &_name);

      public: virtual ~OgreScene();

      public: virtual void Fini();

      public: virtual RenderEngine *Engine() const;

      public: virtual VisualPtr RootVisual() const;

      public: virtual math::Color AmbientLight() const;

      public: virtual void SetAmbientLight(const math::Color &_color);

      public: virtual math::Color BackgroundColor() const;

      public: virtual void SetBackgroundColor(const math::Color &_color);

      public: virtual void PreRender();

      public: virtual void Clear();

      public: virtual void Destroy();

      public: virtual Ogre::SceneManager *OgreSceneManager() const;

      /// \brief Get a visual at a mouse position.
      /// \param[in] _camera Pointer to the camera used to project the mouse
      /// position.
      /// \param[in] _mousePos The 2d position of the mouse in pixels.
      /// \return Pointer to the visual, NULL if none found.
      public: VisualPtr VisualAt(OgreCameraPtr _camera,
                                 const ignition::math::Vector2i &_mousePos);

      /// \brief Helper function for GetVisualAt functions.
      /// \param[in] _camera Pointer to the camera.
      /// \param[in] _mousePos 2D position of the mouse in pixels.
      /// \param[in] _ignoreSelectionObj True to ignore selection objects,
      /// which are GUI objects use to manipulate objects.
      /// \return Pointer to the Ogre::Entity, NULL if none.
      protected: Ogre::Entity *OgreEntityAt(OgreCameraPtr _camera,
          const ignition::math::Vector2i &_mousePos,
          const bool _ignoreSelectionObj);

      /// \brief Get the mesh information for the given mesh.
      /// \param[in] _mesh Mesh to get info about.
      /// \param[out] _vertexCount Number of vertices in the mesh.
      /// \param[out] _vertices Array of the vertices.
      /// \param[out] _indexCount Number if indices.
      /// \param[out] _indices Array of the indices.
      /// \param[in] _position Position of the mesh.
      /// \param[in] _orient Orientation of the mesh.
      /// \param[in] _scale Scale of the mesh
      // Code found in Wiki: www.ogre3d.org/wiki/index.php/RetrieveVertexData
      protected: void MeshInformation(const Ogre::Mesh *_mesh,
                                    size_t &_vertexCount,
                                    Ogre::Vector3* &_vertices,
                                    size_t &_indexCount,
                                    uint64_t* &_indices,
                                    const ignition::math::Vector3d &_position,
                                    const ignition::math::Quaterniond &_orient,
                                    const ignition::math::Vector3d &_scale);

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
                     const std::string &_name, const std::string &_meshName);

      protected: virtual MeshPtr CreateMeshImpl(unsigned int _id,
                     const std::string &_name, const MeshDescriptor &_desc);

      protected: virtual MaterialPtr CreateMaterialImpl(unsigned int _id,
                     const std::string &_name);

      protected: virtual RenderTexturePtr CreateRenderTextureImpl(
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

      protected: math::Color backgroundColor;

      /// \brief A ray query used to locate distances to visuals.
      public: Ogre::RaySceneQuery *raySceneQuery;

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
#endif
