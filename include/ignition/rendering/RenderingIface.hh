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
#ifndef IGNITION_RENDERING_RENDERINGIFACE_HH_
#define IGNITION_RENDERING_RENDERINGIFACE_HH_

#include <string>
#include "ignition/rendering/Export.hh"

namespace ignition
{
  namespace rendering
  {
    class RenderEngine;

    /// \brief Load shared render-engine resources
    /// \return True if successful
    IGNITION_RENDERING_VISIBLE
    bool load();

    /// \brief Initialized shared render-engine features
    /// \return True if successful
    IGNITION_RENDERING_VISIBLE
    bool init();

    /// \brief Destroy all render-engines and related resources
    /// \return True if successful
    IGNITION_RENDERING_VISIBLE
    bool fini();

    /// \brief Get the number of available render-engines. This counts
    /// engines that that have been registered and loaded by the engine
    /// manager. Default engine plugins need to be loaded first using
    /// Engine(std::string) for them to be available.
    /// \return The number of available render-engines
    IGNITION_RENDERING_VISIBLE
    unsigned int engineCount();

    /// \brief Determine if a render-engine is registered under the given name
    /// The function also search through the list of default engines
    /// \param[in] _name Name of the desired render-engine
    /// \return True if a render-engine is registered under the given name
    IGNITION_RENDERING_VISIBLE
    bool hasEngine(const std::string &_name);

    /// \brief Get the render-engine registered under the given name. If no
    /// render-engine is registered under the given name, NULL will be
    /// returned.
    /// \param[in] _name Name of the desired render-engine
    /// \return The specified render-engine
    IGNITION_RENDERING_VISIBLE
    RenderEngine *engine(const std::string &_name);

    /// \brief Get the render-engine registered at the given index. If no
    /// render-engine is registered at the given index, NULL will be returned.
    /// \param[in] _index Index of the desired render-engine
    /// \return The specified render-engine
    IGNITION_RENDERING_VISIBLE
    RenderEngine *engine(const unsigned int _index);

    /// \brief Register a new render-engine under the given name. If the given
    /// name is already in use, the render-engine will not be registered.
    /// \param[in] _name Name the render-engine will be registered under
    /// \param[in] _engine Render-engine to be registered
    IGNITION_RENDERING_VISIBLE
    void registerEngine(const std::string &_name, RenderEngine *_engine);

    /// \brief Unregister a render-engine registered under the given name.
    /// If the no render-engine is registered under the given name no work
    /// will be done.
    /// \param[in] _name Name of the render-engine to unregister
    IGNITION_RENDERING_VISIBLE
    void unregisterEngine(const std::string &_name);

    /// \brief Unregister the given render-engine. If the given render-engine
    /// is not currently registered, no work will be done. Unregistering a
    /// default engine plugin will cause it to shutdown but will not remove it
    // from the list of available engines
    /// \param[in] _engine Render-engine to unregister
    IGNITION_RENDERING_VISIBLE
    void unregisterEngine(RenderEngine *_engine);

    /// \brief Unregister a render-engine at the given index. If the no
    /// render-engine is registered at the given index, no work will be done.
    /// \param[in] _index Index of the render-engine to unregister
    IGNITION_RENDERING_VISIBLE
    void unregisterEngine(const unsigned int _index);
  }
}
#endif
