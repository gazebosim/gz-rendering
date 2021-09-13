/*
 * Copyright (C) 2021 Open Source Robotics Foundation
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

#include <ignition/common/SingletonT.hh>

#include <ignition/plugin/Register.hh>

#include "ignition/rendering/RenderEnginePlugin.hh"
#include "ignition/rendering/base/BaseRenderEngine.hh"
#include "ignition/rendering/base/BaseScene.hh"
#include "ignition/rendering/base/BaseCamera.hh"
#include "ignition/rendering/base/BaseSensor.hh"
#include "ignition/rendering/base/BaseNode.hh"
#include "ignition/rendering/base/BaseObject.hh"
#include "ignition/rendering/base/BaseMaterial.hh"
#include "LuxCoreEngineRenderTypes.hh"

#include "luxcore/luxcore.h"

using namespace ignition;
using namespace rendering;

namespace ignition
{

namespace rendering
{

inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {

  class LuxCoreEngineObject:
    public BaseObject
  {
    protected: LuxCoreEngineObject() {}

    public: virtual ~LuxCoreEngineObject() {}
    
    // This functions seems to be the issue, but for some reason i cant return this->scene
    public: virtual ScenePtr Scene() const
    {
      // line 59 triggers this, for some reason upclass casting does not work
      // error: could not convert ‘((const ignition::rendering::v5::LuxCoreEngineObject*)this)->
      // ignition::rendering::v5::LuxCoreEngineObject::scene’ 
      // from ‘shared_ptr<ignition::rendering::v5::LuxCoreEngineScene>’ to ‘shared_ptr<ignition::rendering::v5::Scene>’
      // return this->scene;
      return nullptr;
    }

    protected: LuxCoreEngineScenePtr scene;

    private: friend class LuxCoreEngineScene;
  };

  class LuxCoreEngineMaterial :
    public BaseMaterial<LuxCoreEngineObject>
  {
    protected: LuxCoreEngineMaterial() {}

    public: virtual ~LuxCoreEngineMaterial() {}

    private: friend class LuxCoreEngineScene;
  };

  class LuxCoreEngineRenderTarget :
      public virtual BaseRenderTarget<LuxCoreEngineObject>
  {
    protected: LuxCoreEngineRenderTarget() : hostDataBuffer(0) {}

    public: virtual ~LuxCoreEngineRenderTarget() {}

    public: virtual void Copy(Image &_image) const
    {
      if (_image.Width() != this->width || _image.Height() != this->height)
      {
        ignerr << "Invalid image dimensions" << std::endl;
        return;
      }

      if (this->hostDataBuffer == NULL)
      {
        ignerr << "Host buffer is NULL" << std::endl;
        return;
      }

      void *imageData = _image.Data<void>();
      memcpy(imageData, this->hostDataBuffer, this->width * this->height * 3);
    }

    public: void *HostDataBuffer()
    {
      return this->hostDataBuffer;
    }

    public: void ResizeHostDataBuffer(unsigned int size)
    {
      this->hostDataBuffer = malloc(size);
    }

    protected: unsigned int MemorySize() const;

    protected: virtual void RebuildImpl() {}

    protected: void *hostDataBuffer;

    private: friend class LuxCoreEngineCamera;
  };

  class LuxCoreEngineNode :
    public BaseNode<LuxCoreEngineObject>
  {
    protected: LuxCoreEngineNode() {}

    public: virtual ~LuxCoreEngineNode() {}

    public: virtual bool HasParent() const override
    {
     return true;
    }

    public: virtual NodePtr Parent() const override
    {
      return nullptr;
    }

    public: virtual void PreRender() override {}

      // Documentation inherited.
    public: virtual math::Vector3d LocalScale() const override 
    {
      return math::Vector3d::One;
    }

      // Documentation inherited.
    public: virtual bool InheritScale() const override 
    {
      return true;
    }

      // Documentation inherited.
    public: virtual void SetInheritScale(bool _inherit) override {}

      // Documentation inherited.
    protected: virtual void SetLocalScaleImpl(
                     const math::Vector3d &_scale) override {}

    protected: virtual math::Pose3d RawLocalPose() const override 
    {
      return math::Pose3d::Zero;
    }

    protected: virtual void SetRawLocalPose(const math::Pose3d &_pose) override {}

    protected: virtual void WritePoseToDevice() {}

    protected: virtual void WritePoseToDeviceImpl() {}

    protected: virtual void SetParent(LuxCoreEngineNodePtr _parent) {}

    protected: virtual void Init() {}

    protected: virtual NodeStorePtr Children() const override 
    {
      return nullptr;
    }

    protected: virtual bool AttachChild(NodePtr _child) override
    {
      return true;
    }

    protected: virtual bool DetachChild(NodePtr _child) override
    {
      return true;
    }

    protected: LuxCoreEngineNodePtr parent;

    protected: math::Pose3d pose;

    protected: bool poseDirty;

    // protected: LuxCoreStorePtr children;

    protected: math::Vector3d scale = math::Vector3d::One;

    protected: bool inheritScale = true;
  };

  class LuxCoreEngineSensor :
    public BaseSensor<LuxCoreEngineNode>
  {
    protected: LuxCoreEngineSensor() {}

    public: virtual ~LuxCoreEngineSensor() {}
  };

  class LuxCoreEngineCamera :
    public BaseCamera<LuxCoreEngineSensor>
  {
    protected: LuxCoreEngineCamera()
    {
      this->renderTarget = LuxCoreEngineRenderTargetPtr(new LuxCoreEngineRenderTarget);
      this->renderTarget->SetFormat(PF_R8G8B8);
    };

    public: virtual ~LuxCoreEngineCamera() {};

    public: virtual void Render() 
    {
      this->renderSessionPtr->Start();
      usleep(120000);
      this->renderSessionPtr->Stop();
      luxcore::Film& film = this->renderSessionPtr->GetFilm();     
      
      float *luxcoreBuffer = (float*)malloc(film.GetWidth() * film.GetHeight() * 3 * sizeof(float));
      film.GetOutput(luxcore::Film::OUTPUT_RGB_IMAGEPIPELINE, luxcoreBuffer); 
 
      if (this->renderTarget->HostDataBuffer() == NULL) 
      {
        this->renderTarget->ResizeHostDataBuffer(this->ImageWidth() * this->ImageHeight() * 3);
      }

      unsigned char* buffer = (unsigned char*)this->renderTarget->HostDataBuffer();
      for (unsigned int x = 0; x < this->ImageHeight() * this->ImageWidth() * 3; x++) {
        buffer[x] = luxcoreBuffer[x] * 255;
      }

      free(luxcoreBuffer);
    };

    public: virtual void Update() 
    {
      this->Render();
    };

    protected: virtual RenderTargetPtr RenderTarget() const 
    {
      return this->renderTarget;
    };

    protected: void SetRenderSession(luxcore::RenderSession *renderSessionPtr)
    {
      this->renderSessionPtr = renderSessionPtr;
    };

    protected: LuxCoreEngineRenderTargetPtr renderTarget;

    private: luxcore::RenderSession *renderSessionPtr;

    private: friend class LuxCoreEngineScene;
  };

  class LuxCoreSceneImpl : public luxcore::Scene {
    public: void GetBBox(float min[3], float max[3]) const {}

    public: const luxcore::Camera &GetCamera() const {}

    public: bool IsImageMapDefined(const std::string &imgMapName) const {}

    public: void SetDeleteMeshData(const bool v) {}

    public: void SetMeshAppliedTransformation(const std::string &meshName,
			  const float *appliedTransMat) {}

    public: void DefineMesh(const std::string &meshName,
		    const long plyNbVerts, const long plyNbTris,
		    float *p, unsigned int *vi, float *n,
		    float *uvs,	float *cols, float *alphas) {}

    public: void DefineMeshExt(const std::string &meshName,
		    const long plyNbVerts, const long plyNbTris,
		    float *p, unsigned int *vi, float *n,
		    std::array<float *, LC_MESH_MAX_DATA_COUNT> *uvs,
		    std::array<float *, LC_MESH_MAX_DATA_COUNT> *cols,
		    std::array<float *, LC_MESH_MAX_DATA_COUNT> *alphas) {}

    public: void SetMeshVertexAOV(const std::string &meshName,
			  const unsigned int index, float *data) {}

    public: void SetMeshTriangleAOV(const std::string &meshName,
			  const unsigned int index, float *data) {}

    public: void SaveMesh(const std::string &meshName, 
        const std::string &fileName) {}

    public: void DefineStrands(const std::string &shapeName, 
        const luxrays::cyHairFile &strandsFile,
		    const StrandsTessellationType tesselType,
		    const unsigned int adaptiveMaxDepth, const float adaptiveError,
		    const unsigned int solidSideCount, const bool solidCapBottom, 
        const bool solidCapTop, const bool useCameraPosition) {}

    public: bool IsMeshDefined(const std::string &meshName) const {}

    public: bool IsTextureDefined(const std::string &texName) const {}

    public: bool IsMaterialDefined(const std::string &matName) const {}

    public: const unsigned int GetLightCount() const {}

    public: const unsigned int GetObjectCount() const {}

    public: void Parse(const luxrays::Properties &props) {}

    public: void DuplicateObject(const std::string &srcObjName, 
        const std::string &dstObjName, const float *transMat, 
        const unsigned int objectID = 0xffffffff) {}

    public: void DuplicateObject(const std::string &srcObjName, 
        const std::string &dstObjNamePrefix, const unsigned int count, 
        const float *transMat, const unsigned int *objectIDs = NULL) {}

    public: void DuplicateObject(const std::string &srcObjName, 
        const std::string &dstObjName, const unsigned int steps, 
        const float *times, const float *transMat,
			  const unsigned int objectID = 0xffffffff) {}

    public: void DuplicateObject(const std::string &srcObjName, 
        const std::string &dstObjNamePrefix, const unsigned int count, 
        const unsigned int steps, const float *times,
			  const float *transMat, const unsigned int *objectIDs = NULL) {}

    public: void UpdateObjectTransformation(const std::string &objName, 
        const float *transMat) {}

    public: void UpdateObjectMaterial(const std::string &objName, 
        const std::string &matName) {}

    public: void DeleteObject(const std::string &objName) {}

    public: void DeleteLight(const std::string &lightName) {}

    public: void RemoveUnusedImageMaps() {}

    public: void RemoveUnusedTextures() {}

    public: void RemoveUnusedMaterials() {}

    public: void RemoveUnusedMeshes() {}

    public: const luxrays::Properties &ToProperties() const {}

    public: void Save(const std::string &fileName) const {}

    protected: void DefineImageMapUChar(const std::string &imgMapName,
			  unsigned char *pixels, const float gamma, const unsigned int channels,
			  const unsigned int width, const unsigned int height,
			  ChannelSelectionType selectionType, WrapType wrapType) {}

    protected: void DefineImageMapHalf(const std::string &imgMapName,
			  unsigned short *pixels, const float gamma, const unsigned int channels,
			  const unsigned int width, const unsigned int height,
			  ChannelSelectionType selectionType, WrapType wrapType) {}

    protected: void DefineImageMapFloat(const std::string &imgMapName,
			  float *pixels, const float gamma, const unsigned int channels,
			  const unsigned int width, const unsigned int height,
			  ChannelSelectionType selectionType, WrapType wrapType) {}
  };

  class LuxCoreEngineScene :
    public BaseScene
  {
    protected: LuxCoreEngineScene(unsigned int _id, const std::string &_name) : BaseScene(_id, _name) 
    {
      this->id_ = _id;
      this->name_ = _name;

      luxcore::Init();
      
      luxrays::Properties sceneProperties;
      LuxCoreSceneImpl sceneLux;

      luxrays::Properties props("scenes/empty/simple.cfg");
      props.Set(luxrays::Property("renderengine.type")("PATHCPU")); 
      props.Set(luxrays::Property("scene.camera.lookat.orig")("10.951 -20.663 8.017"));
      props.Set(luxrays::Property("scene.camera.lookat.target")("0.0 0.0 1.0"));

      luxcore::RenderConfig *config = luxcore::RenderConfig::Create(props, &sceneLux);
			props = config->ToProperties();

      this->renderSessionPtr = luxcore::RenderSession::Create(config);
    }

    public: virtual ~LuxCoreEngineScene() {}

    public: virtual void Fini() {}

    public: virtual RenderEngine *Engine() const 
    {
      ignerr << "engine() was called 246" << std::endl;
      // return LuxCoreEngineRenderEngine::Instance();
      return nullptr;
    }

    public: virtual VisualPtr RootVisual() const 
    {
      return nullptr;
    }

    public: virtual math::Color AmbientLight() const 
    {
      return math::Color::Black;
    }

    public: virtual void SetAmbientLight(const math::Color &_color) 
    {

    }

    protected: virtual DirectionalLightPtr CreateDirectionalLightImpl(
                     unsigned int _id, const std::string &_name) 
    {
      return nullptr;
    }

    protected: virtual PointLightPtr CreatePointLightImpl(unsigned int _id,
                     const std::string &_name)
    {
      return nullptr;
    }

    protected: virtual SpotLightPtr CreateSpotLightImpl(unsigned int _id,
                     const std::string &_name)
    {
      return nullptr;
    }

    protected: virtual CameraPtr CreateCameraImpl(unsigned int _id,
                     const std::string &_name)
    {
      LuxCoreEngineCameraPtr camera(new LuxCoreEngineCamera);
      camera->SetRenderSession(this->renderSessionPtr);
      bool result = this->InitObject(camera, _id, _name);
      return (result) ? camera : nullptr;
    }

    protected: virtual DepthCameraPtr CreateDepthCameraImpl(unsigned int _id,
                     const std::string &_name) override
    {
      return nullptr;
    }

    protected: virtual VisualPtr CreateVisualImpl(unsigned int _id,
                     const std::string &_name)
    {
      return nullptr;
    }

    protected: virtual ArrowVisualPtr CreateArrowVisualImpl(unsigned int _id,
                     const std::string &_name)
    {
      return nullptr;
    }

    protected: virtual AxisVisualPtr CreateAxisVisualImpl(unsigned int _id,
                     const std::string &_name)
    {
      return nullptr;
    }

    protected: virtual GeometryPtr CreateBoxImpl(unsigned int _id,
                     const std::string &_name)
    {
      return nullptr;
    }

    protected: virtual GeometryPtr CreateConeImpl(unsigned int _id,
                     const std::string &_name)
    {
      return nullptr;
    }

    protected: virtual GeometryPtr CreateCylinderImpl(unsigned int _id,
                     const std::string &_name)
    {
      return nullptr;
    }

    protected: virtual GeometryPtr CreatePlaneImpl(unsigned int _id,
                     const std::string &_name)
    {
      return nullptr;
    }

    protected: virtual GeometryPtr CreateSphereImpl(unsigned int _id,
                     const std::string &_name)
    {
      return nullptr;
    }

    protected: virtual MeshPtr CreateMeshImpl(unsigned int _id,
                     const std::string &_name, const std::string &_meshName)
    {
      return nullptr;
    }

    protected: virtual MeshPtr CreateMeshImpl(unsigned int _id,
                     const std::string &_name, const MeshDescriptor &_desc)
    {
      return nullptr;
    }

    protected: virtual CapsulePtr CreateCapsuleImpl(unsigned int _id,
                     const std::string &_name)
    {
      return nullptr;
    }

    protected: virtual GridPtr CreateGridImpl(unsigned int _id,
                     const std::string &_name)
    {
      return nullptr;
    }

    protected: virtual MarkerPtr CreateMarkerImpl(unsigned int _id,
                     const std::string &_name)
    {
      return nullptr;
    }

    protected: virtual LidarVisualPtr CreateLidarVisualImpl(unsigned int _id,
                     const std::string &_name)
    {
      return nullptr;
    }

    protected: virtual HeightmapPtr CreateHeightmapImpl(unsigned int _id,
                     const std::string &_name,
                     const HeightmapDescriptor &_desc)
    {
      return nullptr;
    }

    protected: virtual WireBoxPtr CreateWireBoxImpl(unsigned int _id,
                     const std::string &_name)
    {
      return nullptr;
    }

    protected: virtual MaterialPtr CreateMaterialImpl(unsigned int _id,
                     const std::string &_name)
    {
      LuxCoreEngineMaterialPtr material(new LuxCoreEngineMaterial);
      bool result = this->InitObject(material, _id, _name);
      return (result) ? material : nullptr;
    }

    protected: virtual RenderTexturePtr CreateRenderTextureImpl(
                     unsigned int _id, const std::string &_name)
    {
      return nullptr;
    }

    protected: virtual RenderWindowPtr CreateRenderWindowImpl(
                     unsigned int _id, const std::string &_name)
    {
      return nullptr;
    }

    protected: virtual RayQueryPtr CreateRayQueryImpl(
                     unsigned int _id, const std::string &_name)
    {
      return nullptr;
    }

    protected: virtual LightStorePtr Lights() const
    {
      return nullptr;
    }

    protected: virtual SensorStorePtr Sensors() const
    {
      return this->sensors;
    }

    protected: virtual VisualStorePtr Visuals() const
    {
      return nullptr;
    }

    protected: virtual MaterialMapPtr Materials() const
    {
      return this->materials;
    }

    protected: virtual bool LoadImpl()
    {
      ignerr << "LoadImpl" << std::endl;
      return true;
    }

    protected: virtual bool InitImpl()
    {
      ignerr << "InitImpl" << std::endl;
      this->CreateStores();
      return true;
    }

    public: virtual bool IsInitialized() 
    {
      return true;
    }

    public: virtual unsigned int Id() 
    {
      return this->id_;
    }

    public: virtual std::string Name() 
    {
      return this->name_;
    }

    ignition::rendering::LightVisualPtr CreateLightVisualImpl(unsigned int _id,
                     const std::string &_name)
    {
      return nullptr;
    }

    protected: void CreateStores()
    {
      // this->lights = OptixLightStorePtr(new OptixLightStore);
      this->sensors = LuxCoreEngineSensorStorePtr(new LuxCoreEngineSensorStore);
      // this->visuals = OptixVisualStorePtr(new OptixVisualStore);
      this->materials = LuxCoreEngineMaterialMapPtr(new LuxCoreEngineMaterialMap);
    }

    protected: bool InitObject(LuxCoreEngineObjectPtr _object, unsigned int _id,
    const std::string &_name)
    {
      // assign needed varibles
      _object->id = _id;
      _object->name = _name;
      _object->scene = this->SharedThis();

      // initialize object
      _object->Load();
      _object->Init();

      return true;
    }

    private: LuxCoreEngineScenePtr SharedThis()
    {
      ScenePtr sharedBase = this->shared_from_this();
      return std::dynamic_pointer_cast<LuxCoreEngineScene>(sharedBase);
    }

    protected: luxcore::RenderSession *renderSessionPtr;

    protected: unsigned int id_;
    protected: std::string name_;

    protected: LuxCoreEngineMaterialMapPtr materials;
    protected: LuxCoreEngineSensorStorePtr sensors;

    private: friend class LuxCoreEngineRenderEngine;

  };


  /// \brief The render engine class which implements a render engine.
  class LuxCoreEngineRenderEngine :
    public virtual BaseRenderEngine,
    public common::SingletonT<LuxCoreEngineRenderEngine>
  {
    // Documentation Inherited.
    public: virtual bool IsEnabled() const override
    {
      return true;
    }

    // Documentation Inherited.
    public: virtual std::string Name() const override
    {
      return "LuxCoreEngineRenderEngine";
    }

    // Documentation Inherited.
    protected: virtual bool LoadImpl(const std::map<std::string,
                   std::string> &/*_params*/) override
    {
      return true;
    }

    /// \brief Initialize the render engine.
    /// \return True if the operation is successful
    protected: virtual bool InitImpl() override
    {
      this->scenes = LuxCoreEngineSceneStorePtr(new LuxCoreEngineSceneStore);
      return true;
    }

    /// \brief Get a pointer to the list of scenes managed by the render
    /// engine.
    /// \return list of scenes
    protected: virtual SceneStorePtr Scenes()
                     const override
    {
      return this->scenes;
    }

    /// \brief Create a scene.
    /// \param[in] _id Unique scene Id
    /// \parampin] _name Name of scene
    protected: virtual ScenePtr
                   CreateSceneImpl(unsigned int _id,
                   const std::string &_name) override
    {
      auto scene = LuxCoreEngineScenePtr(new LuxCoreEngineScene(_id, _name));
      this->scenes->Add(scene);
      return scene;
    }

    /// \brief Singelton setup.
    private: friend class common::SingletonT<LuxCoreEngineRenderEngine>;
    private: LuxCoreEngineSceneStorePtr scenes;
  };

  /// \brief Plugin for loading the HelloWorld render engine.
  class LuxCoreEnginePlugin :
    public RenderEnginePlugin
  {
    /// \brief Get the name of the render engine loaded by this plugin.
    /// \return Name of render engine
    public: std::string Name() const override
    {
      return LuxCoreEngineRenderEngine::Instance()->Name();
    }

    /// \brief Get a pointer to the render engine loaded by this plugin.
    /// \return Render engine instance
    public: RenderEngine *Engine() const override
    {
      return LuxCoreEngineRenderEngine::Instance();
    }
  };

}

}

}

// Register this plugin
IGNITION_ADD_PLUGIN(ignition::rendering::LuxCoreEnginePlugin,
                    ignition::rendering::RenderEnginePlugin)
