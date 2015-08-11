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
#ifndef _IGNITION_RENDERING_RENDERINGIFACE_HH_
#define _IGNITION_RENDERING_RENDERINGIFACE_HH_

#include <string>
#include "ignition/rendering/Util.hh"

namespace ignition
{
  namespace rendering
  {
    class RenderEngine;

    IGNITION_VISIBLE
    bool load();

    IGNITION_VISIBLE
    bool init();

    IGNITION_VISIBLE
    bool fini();

    IGNITION_VISIBLE
    unsigned int get_engine_count();

    IGNITION_VISIBLE
    bool has_engine(const std::string &_name);

    IGNITION_VISIBLE
    RenderEngine *get_engine(const std::string &_name);

    IGNITION_VISIBLE
    RenderEngine *get_engine(unsigned int _index);

    IGNITION_VISIBLE
    void register_engine(const std::string &_name, RenderEngine *_engine);

    IGNITION_VISIBLE
    void unregister_engine(const std::string &_name);

    IGNITION_VISIBLE
    void unregister_engine(RenderEngine *_engine);

    IGNITION_VISIBLE
    void unregister_engine(unsigned int _index);
  }
}
#endif
