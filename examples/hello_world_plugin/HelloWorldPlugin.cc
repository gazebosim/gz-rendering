/*
 * Copyright (C) 2020 Open Source Robotics Foundation
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
#include "ignition/rendering/base/BaseRenderTypes.hh"

namespace mock
{
  class HelloWorldRenderEngine :
    public virtual ignition::rendering::BaseRenderEngine,
    public ignition::common::SingletonT<HelloWorldRenderEngine>
  {
    // Documentation Inherited.
    public: virtual bool IsEnabled() const override
    {
      return true;
    }

    // Documentation Inherited.
    public: virtual std::string Name() const override
    {
      return "test2";
    }

    protected: virtual bool LoadImpl(const std::map<std::string, std::string> &_params) override
    {
      return true;
    }

    protected: virtual bool InitImpl() override
    {
      return true;
    }

    protected: virtual ignition::rendering::SceneStorePtr Scenes() const override
    {
      return nullptr;
    }

    protected: virtual ignition::rendering::ScenePtr CreateSceneImpl(unsigned int _id,
                const std::string &_name) override
    {
      return nullptr;
    }

    private: friend class ignition::common::SingletonT<HelloWorldRenderEngine>;
  };

  class HelloWorldPlugin :
    public ignition::rendering::RenderEnginePlugin
  {
    public: std::string Name() const
    {
      return "test";
    }
    public: ignition::rendering::RenderEngine *Engine() const
    {
      return HelloWorldRenderEngine::Instance();
    }
  };
}

// Register this plugin
IGNITION_ADD_PLUGIN(mock::HelloWorldPlugin,
                    ignition::rendering::RenderEnginePlugin)
