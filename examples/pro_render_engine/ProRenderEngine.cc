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
#include "ProRenderEngineRenderTypes.hh"

#include "RadeonProRender.h"
#include "Math/mathutils.h"

using namespace ignition;
using namespace rendering;

namespace ignition
{

namespace rendering
{

inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {

  class ProRenderEngineObject:
    public BaseObject
  {
    protected: ProRenderEngineObject() {}

    public: virtual ~ProRenderEngineObject() {}
    
    // This functions seems to be the issue, but for some reason i cant return this->scene
    public: virtual ScenePtr Scene() const
    {
      // line 59 triggers this, for some reason upclass casting does not work
      // error: could not convert ‘((const ignition::rendering::v5::ProRenderEngineObject*)this)->
      // ignition::rendering::v5::ProRenderEngineObject::scene’ 
      // from ‘shared_ptr<ignition::rendering::v5::ProRenderEngineScene>’ to ‘shared_ptr<ignition::rendering::v5::Scene>’
      // return this->scene;
      return nullptr;
    }

    protected: ProRenderEngineScenePtr scene;

    private: friend class ProRenderEngineScene;
  };

  class ProRenderEngineMaterial :
    public BaseMaterial<ProRenderEngineObject>
  {
    protected: ProRenderEngineMaterial() {}

    public: virtual ~ProRenderEngineMaterial() {}

    private: friend class ProRenderEngineScene;
  };

  class ProRenderEngineRenderTarget :
      public virtual BaseRenderTarget<ProRenderEngineObject>
  {
    protected: ProRenderEngineRenderTarget() : hostData(0) {}

    public: virtual ~ProRenderEngineRenderTarget() {}

    public: virtual void Copy(Image &_image) const
    {
      if (_image.Width() != this->width || _image.Height() != this->height)
      {
        ignerr << "Invalid image dimensions" << std::endl;
        return;
      }

      unsigned char *imageData = _image.Data<unsigned char>();

      for (unsigned int y = 0; y < this->height; y++) {
        for (unsigned int x = 0; x < this->width; x++) {
          imageData[(3 * (y * width + x)) + 0] = 255;
          imageData[(3 * (y * width + x)) + 1] = 155;
          imageData[(3 * (y * width + x)) + 2] = 55;
        }
      }
    }

    protected: unsigned int MemorySize() const;

    protected: virtual void RebuildImpl() 
    {
      delete this->hostData;
      unsigned int count = this->width * this->height * 3;
      this->hostData = new float[count];
    }

    protected: float *hostData;

    private: friend class ProRenderEngineCamera;
  };

  class ProRenderEngineNode :
    public BaseNode<ProRenderEngineObject>
  {
    protected: ProRenderEngineNode() {}

    public: virtual ~ProRenderEngineNode() {}

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

    protected: virtual void SetParent(ProRenderEngineNodePtr _parent) {}

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

    protected: ProRenderEngineNodePtr parent;

    protected: math::Pose3d pose;

    protected: bool poseDirty;

    // protected: LuxCoreStorePtr children;

    protected: math::Vector3d scale = math::Vector3d::One;

    protected: bool inheritScale = true;
  };

  class ProRenderEngineSensor :
    public BaseSensor<ProRenderEngineNode>
  {
    protected: ProRenderEngineSensor() {}

    public: virtual ~ProRenderEngineSensor() {}
  };

  class ProRenderEngineCamera :
    public BaseCamera<ProRenderEngineSensor>
  {
    protected: ProRenderEngineCamera()
    {
      this->renderTarget = ProRenderEngineRenderTargetPtr(new ProRenderEngineRenderTarget);
      this->renderTarget->SetFormat(PF_R8G8B8);
    };

    public: virtual ~ProRenderEngineCamera() {};

    public: virtual void Render() {};

    public: virtual void Update() {};

    protected: virtual RenderTargetPtr RenderTarget() const 
    {
      return this->renderTarget;
    };

    protected: ProRenderEngineRenderTargetPtr renderTarget;

    private: friend class ProRenderEngineScene;
  };

  class ProRenderEngineScene :
    public BaseScene
  {
    protected: ProRenderEngineScene(unsigned int _id, const std::string &_name) : BaseScene(_id, _name) 
    {
      this->id_ = _id;
      this->name_ = _name;
    }

    public: virtual ~ProRenderEngineScene() {}

    public: virtual void Fini() {}

    public: virtual RenderEngine *Engine() const 
    {
      ignerr << "engine() was called 246" << std::endl;
      // return ProRenderEngineRenderEngine::Instance();
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
      ProRenderEngineCameraPtr camera(new ProRenderEngineCamera);
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
      ProRenderEngineMaterialPtr material(new ProRenderEngineMaterial);
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
      this->sensors = ProRenderEngineSensorStorePtr(new ProRenderEngineSensorStore);
      // this->visuals = OptixVisualStorePtr(new OptixVisualStore);
      this->materials = ProRenderEngineMaterialMapPtr(new ProRenderEngineMaterialMap);
    }

    protected: bool InitObject(ProRenderEngineObjectPtr _object, unsigned int _id,
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

    private: ProRenderEngineScenePtr SharedThis()
    {
      ScenePtr sharedBase = this->shared_from_this();
      return std::dynamic_pointer_cast<ProRenderEngineScene>(sharedBase);
    }

    protected: unsigned int id_;
    protected: std::string name_;

    protected: ProRenderEngineMaterialMapPtr materials;
    protected: ProRenderEngineSensorStorePtr sensors;

    private: friend class ProRenderEngineRenderEngine;

  };


  /// \brief The render engine class which implements a render engine.
  class ProRenderEngineRenderEngine :
    public virtual BaseRenderEngine,
    public common::SingletonT<ProRenderEngineRenderEngine>
  {
    // Documentation Inherited.
    public: virtual bool IsEnabled() const override
    {
      return true;
    }

    // Documentation Inherited.
    public: virtual std::string Name() const override
    {
      return "ProRenderEngineRenderEngine";
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
      this->scenes = ProRenderEngineSceneStorePtr(new ProRenderEngineSceneStore);
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
      rpr_int tahoePluginID = rprRegisterPlugin("./libNorthstar64.so"); 
      assert(tahoePluginID != -1);
      
      rpr_int plugins[] = { tahoePluginID };
	    size_t pluginCount = sizeof(plugins) / sizeof(plugins[0]);

      rpr_context context = NULL;
      int result = rprCreateContext(RPR_API_VERSION, plugins, pluginCount, RPR_CREATION_FLAGS_ENABLE_GPU0, NULL, NULL, &context);
	    if (result != RPR_SUCCESS) {
        printf("%d\n", result);
      }

	    rprContextSetActivePlugin(context, plugins[0]);
	    rpr_material_system matsys;
	    rprContextCreateMaterialSystem(context, 0, &matsys);

      rpr_scene scenePR = nullptr;
	    rprContextCreateScene(context, &scenePR);

      rpr_camera cameraPR = nullptr;
      rprContextCreateCamera(context, &cameraPR);
      rprCameraLookAt(cameraPR, 0, 5, 20, 0, 1, 0, 0, 1, 0);
      rprCameraSetFocalLength(cameraPR, 75.f);
      rprSceneSetCamera(scenePR, cameraPR);
      rprContextSetScene(context, scenePR);

      rpr_framebuffer_desc desc = { 800 , 600 };
      rpr_framebuffer_format fmt = {4, RPR_COMPONENT_TYPE_FLOAT32};
      rpr_framebuffer frame_buffer = nullptr;
      rpr_framebuffer frame_buffer_resolved = nullptr;
      rprContextCreateFrameBuffer(context, fmt, &desc, &frame_buffer);
      rprContextCreateFrameBuffer(context, fmt, &desc, &frame_buffer_resolved);
      rprFrameBufferClear(frame_buffer);
      rprContextSetAOV(context, RPR_AOV_COLOR, frame_buffer);
      rprContextSetParameterByKey1u(context, RPR_CONTEXT_RENDER_MODE, RPR_RENDER_MODE_NORMAL);

      struct vertex
      {
      	rpr_float pos[3];
      	rpr_float norm[3];
      	rpr_float tex[2];
      };
      
      vertex cube_data[] = 
      {
      	{ -1.0f, 1.0f, -1.0f, 0.f, 1.f, 0.f, 0.f, 0.f },
      	{  1.0f, 1.0f, -1.0f, 0.f, 1.f, 0.f, 0.f, 0.f },
      	{  1.0f, 1.0f, 1.0f , 0.f, 1.f, 0.f, 0.f, 0.f },
      	{  -1.0f, 1.0f, 1.0f , 0.f, 1.f, 0.f, 0.f, 0.f},
      
      	{  -1.0f, -1.0f, -1.0f , 0.f, -1.f, 0.f, 0.f, 0.f },
      	{  1.0f, -1.0f, -1.0f , 0.f, -1.f, 0.f, 0.f, 0.f },
      	{  1.0f, -1.0f, 1.0f , 0.f, -1.f, 0.f, 0.f, 0.f },
      	{  -1.0f, -1.0f, 1.0f , 0.f, -1.f, 0.f, 0.f, 0.f },
      
      	{  -1.0f, -1.0f, 1.0f , -1.f, 0.f, 0.f, 0.f, 0.f },
      	{  -1.0f, -1.0f, -1.0f , -1.f, 0.f, 0.f, 0.f, 0.f },
      	{  -1.0f, 1.0f, -1.0f , -1.f, 0.f, 0.f, 0.f, 0.f },
      	{  -1.0f, 1.0f, 1.0f , -1.f, 0.f, 0.f, 0.f, 0.f },
      
      	{  1.0f, -1.0f, 1.0f ,  1.f, 0.f, 0.f, 0.f, 0.f },
      	{  1.0f, -1.0f, -1.0f ,  1.f, 0.f, 0.f, 0.f, 0.f },
      	{  1.0f, 1.0f, -1.0f ,  1.f, 0.f, 0.f, 0.f, 0.f },
      	{  1.0f, 1.0f, 1.0f ,  1.f, 0.f, 0.f, 0.f, 0.f },
      
      	{  -1.0f, -1.0f, -1.0f ,  0.f, 0.f, -1.f , 0.f, 0.f },
      	{  1.0f, -1.0f, -1.0f ,  0.f, 0.f, -1.f , 0.f, 0.f },
      	{  1.0f, 1.0f, -1.0f ,  0.f, 0.f, -1.f, 0.f, 0.f },
      	{  -1.0f, 1.0f, -1.0f ,  0.f, 0.f, -1.f, 0.f, 0.f },
      
      	{  -1.0f, -1.0f, 1.0f , 0.f, 0.f, 1.f, 0.f, 0.f },
      	{  1.0f, -1.0f, 1.0f , 0.f, 0.f,  1.f, 0.f, 0.f },
      	{  1.0f, 1.0f, 1.0f , 0.f, 0.f, 1.f, 0.f, 0.f },
      	{  -1.0f, 1.0f, 1.0f , 0.f, 0.f, 1.f, 0.f, 0.f },
      };
      
      rpr_int indices[] = 
      {
      	3,1,0,
      	2,1,3,
      
      	6,4,5,
      	7,4,6,
      
      	11,9,8,
      	10,9,11,
      
      	14,12,13,
      	15,12,14,
      
      	19,17,16,
      	18,17,19,
      
      	22,20,21,
      	23,20,22
      };
      
      rpr_int num_face_vertices[] = 
      {
      	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3
      };

      rpr_shape cube = nullptr;
      rprContextCreateMesh(context,
      			(rpr_float const*)&cube_data[0], 24, sizeof(vertex),
      			(rpr_float const*)((char*)&cube_data[0] + sizeof(rpr_float) * 3), 24, sizeof(vertex),
      			(rpr_float const*)((char*)&cube_data[0] + sizeof(rpr_float) * 6), 24, sizeof(vertex),
      			(rpr_int const*)indices, sizeof(rpr_int),
      			(rpr_int const*)indices, sizeof(rpr_int),
      			(rpr_int const*)indices, sizeof(rpr_int),
      			num_face_vertices, 12, &cube);
      rprSceneAttachShape(scenePR, cube);
      RadeonProRender::matrix m = RadeonProRender::translation(RadeonProRender::float3(-2, 1, 0));
      rprShapeSetTransform(cube, RPR_TRUE, &m.m00);

      rprContextSetParameterByKey1u(context,RPR_CONTEXT_ITERATIONS,1);
      rprContextRender(context);
      rprContextResolveFrameBuffer(context,frame_buffer,frame_buffer_resolved,true);

      rprFrameBufferSaveToFile(frame_buffer_resolved, "image.png");

      auto scene = ProRenderEngineScenePtr(new ProRenderEngineScene(_id, _name));
      this->scenes->Add(scene);
      return scene;
    }

    /// \brief Singelton setup.
    private: friend class common::SingletonT<ProRenderEngineRenderEngine>;
    private: ProRenderEngineSceneStorePtr scenes;
  };

  /// \brief Plugin for loading the HelloWorld render engine.
  class ProRenderEnginePlugin :
    public RenderEnginePlugin
  {
    /// \brief Get the name of the render engine loaded by this plugin.
    /// \return Name of render engine
    public: std::string Name() const override
    {
      return ProRenderEngineRenderEngine::Instance()->Name();
    }

    /// \brief Get a pointer to the render engine loaded by this plugin.
    /// \return Render engine instance
    public: RenderEngine *Engine() const override
    {
      return ProRenderEngineRenderEngine::Instance();
    }
  };

}

}

}

// Register this plugin
IGNITION_ADD_PLUGIN(ignition::rendering::ProRenderEnginePlugin,
                    ignition::rendering::RenderEnginePlugin)
