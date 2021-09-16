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

#include "LuxCoreEngineRenderTypes.hh"
#include "LuxCoreEngineObject.hh"
#include "LuxCoreEngineMaterial.hh"
#include "LuxCoreEngineRenderTarget.hh"
#include "LuxCoreEngineNode.hh"
#include "LuxCoreEngineSensor.hh"
#include "LuxCoreEngineCamera.hh"
#include "LuxCoreEngineGeometry.hh"
#include "LuxCoreEngineVisual.hh"
#include "LuxCoreEngineMesh.hh"
#include "LuxCoreEngineMeshFactory.hh"
#include "LuxCoreEngineScene.hh"

#include "luxcore/luxcore.h"

using namespace ignition;
using namespace rendering;

namespace ignition
{

namespace rendering
{

inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
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
