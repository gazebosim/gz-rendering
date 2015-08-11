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
#ifndef _IGNITION_RENDERING_RENDERENGINEMANAGER_HH_
#define _IGNITION_RENDERING_RENDERENGINEMANAGER_HH_

#include <string>
#include "gazebo/common/SingletonT.hh"
#include "gazebo/common/Console.hh"
#include "ignition/rendering/Util.hh"

namespace ignition
{
  namespace rendering
  {
    class RenderEngine;

    class IGNITION_VISIBLE RenderEngineManager :
      public virtual SingletonT<RenderEngineManager>
    {
      public: RenderEngineManager();

      public: ~RenderEngineManager();

      public: unsigned int GetEngineCount() const;

      public: bool HasEngine(const std::string &_name) const;

      public: RenderEngine *GetEngine(const std::string &_name) const;

      public: RenderEngine *GetEngineAt(unsigned int _index) const;

      public: void RegisterEngine(const std::string &_name,
                  RenderEngine *_engine);

      public: void UnregisterEngine(const std::string &_name);

      public: void UnregisterEngine(RenderEngine *_engine);

      public: void UnregisterEngineAt(unsigned int _index);

      private: class RenderEngineManagerPrivate *pimpl;

      private: friend class SingletonT<RenderEngineManager>;
    };
  }
}
#endif
