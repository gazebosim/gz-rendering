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

#include <luxcore/luxcore.h>

#include "ignition/rendering/RenderEnginePlugin.hh"
#include "ignition/rendering/base/BaseRenderEngine.hh"
#include "ignition/rendering/base/BaseScene.hh"

#include "LuxCoreEngineCamera.hh"
#include "LuxCoreEngineGeometry.hh"
#include "LuxCoreEngineMaterial.hh"
#include "LuxCoreEngineMesh.hh"
#include "LuxCoreEngineMeshFactory.hh"
#include "LuxCoreEngineNode.hh"
#include "LuxCoreEngineObject.hh"
#include "LuxCoreEngineRenderTarget.hh"
#include "LuxCoreEngineRenderTypes.hh"
#include "LuxCoreEngineScene.hh"
#include "LuxCoreEngineSensor.hh"
#include "LuxCoreEngineVisual.hh"

using namespace ignition;
using namespace rendering;

namespace ignition {

namespace rendering {

inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {

class LuxCoreEngineRenderEngine
    : public virtual BaseRenderEngine,
      public common::SingletonT<LuxCoreEngineRenderEngine> {

  public: virtual bool IsEnabled() const override
  {
    return true;
  }

  public: virtual std::string Name() const override
  {
    return "LuxCoreEngineRenderEngine";
  }

  protected: virtual bool LoadImpl(
      const std::map<std::string, std::string> &/*_params*/) override
  {
    return true;
  }

  protected: virtual bool InitImpl() override
  {
    this->scenes = LuxCoreEngineSceneStorePtr(new LuxCoreEngineSceneStore);
    return true;
  }

  protected: virtual SceneStorePtr Scenes() const override
  {
    return this->scenes;
  }
  
  protected: virtual ScenePtr CreateSceneImpl(unsigned int _id,
                                              const std::string &_name) override
  {
    auto scene = LuxCoreEngineScenePtr(new LuxCoreEngineScene(_id, _name));
    this->scenes->Add(scene);
    return scene;
  }

  private: friend class common::SingletonT<LuxCoreEngineRenderEngine>;

  private: LuxCoreEngineSceneStorePtr scenes;
};

class LuxCoreEnginePlugin : public RenderEnginePlugin {

  public: std::string Name() const override
  {
    return LuxCoreEngineRenderEngine::Instance()->Name();
  }

  public: RenderEngine *Engine() const override
  {
    return LuxCoreEngineRenderEngine::Instance();
  }
};

} // namespace IGNITION_RENDERING_VERSION_NAMESPACE

} // namespace rendering

} // namespace ignition

IGNITION_ADD_PLUGIN(ignition::rendering::LuxCoreEnginePlugin,
                    ignition::rendering::RenderEnginePlugin)
