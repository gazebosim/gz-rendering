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
#ifndef IGNITION_RENDERING_OGRE2_OGRE2RENDERENGINE_HH_
#define IGNITION_RENDERING_OGRE2_OGRE2RENDERENGINE_HH_

#include <memory>
#include <string>
#include <vector>

#include <ignition/common/SingletonT.hh>

#include "ignition/rendering/RenderEnginePlugin.hh"
#include "ignition/rendering/base/BaseRenderEngine.hh"
#include "ignition/rendering/base/BaseRenderTypes.hh"
//#include "ignition/rendering/ogre/OgreIncludes.hh"
#include "ignition/rendering/ogre2/Ogre2RenderTypes.hh"
#include "ignition/rendering/ogre2/Export.hh"

namespace Ogre
{
  class LogManager;
  class Root;
}

namespace ignition
{
  namespace rendering
  {
    // forward declaration
    class Ogre2RenderEnginePrivate;

    /// \brief Plugin for loading ogre render engine
    class IGNITION_RENDERING_OGRE2_VISIBLE Ogre2RenderEnginePlugin :
      public RenderEnginePlugin
    {
      /// \brief Constructor
      public: Ogre2RenderEnginePlugin();

      /// \brief Destructor
      public: ~Ogre2RenderEnginePlugin() = default;

      /// \brief Get the name of the render engine loaded by this plugin.
      /// \return Name of render engine
      public: std::string Name() const;

      /// \brief Get a pointer to the render engine loaded by this plugin.
      /// \return Render engine instance
      public: RenderEngine *Engine() const;
    };

    class IGNITION_RENDERING_OGRE2_VISIBLE Ogre2RenderEngine :
      public virtual BaseRenderEngine,
      public common::SingletonT<Ogre2RenderEngine>
    {
      /// \enum RenderPathType
      /// \brief The type of rendering path used by the rendering engine.
/*      public: enum OgreRenderPathType
              {
                /// \brief No rendering is done.
                NONE = 0,
                /// \brief Most basic rendering, with least fidelity.
                VERTEX = 1,
                /// \brief Utilizes the RTT shader system.
                FORWARD = 2,
                /// \brief Utilizes deferred rendering. Best fidelity.
                DEFERRED = 3,
                /// \brief Count of the rendering path enums.
                RENDER_PATH_COUNT
              };
              */

      /// \brief Constructor
      private: Ogre2RenderEngine();

      public: virtual ~Ogre2RenderEngine();

      public: virtual bool Fini();

      public: virtual bool IsEnabled() const;

      public: virtual std::string Name() const;

//      public: OgreRenderPathType RenderPathType() const;

//      public: void AddResourcePath(const std::string &_uri);

      public: virtual Ogre::Root *OgreRoot() const;

      public: std::string CreateWindow(const std::string &_handle,
                  const unsigned int _width, const unsigned int _height,
                  const double _ratio, const unsigned int antiAliasing);

      protected: virtual ScenePtr CreateSceneImpl(unsigned int _id,
                  const std::string &_name);

      protected: virtual SceneStorePtr Scenes() const;

      protected: virtual bool LoadImpl();

      protected: virtual bool InitImpl();

      private: void LoadAttempt();

      private: void CreateLogger();

      private: void CreateContext();

      private: void CreateRoot();

      private: void CreateOverlay();

      private: void LoadPlugins();

      private: void CreateRenderSystem();

      private: void CreateWindow();

      private: void CreateResources();


/*      private: void CreateLogger();


      private: void CreateRoot();


      private: void LoadPlugins();

      private: void CreateRenderSystem();

      private: void CreateResources();

      private: void CreateWindow();

      private: void CheckCapabilities();
      */

      /// \brief Attempt to initialize engine and catch expections if they
      /// occur
      private: void InitAttempt();


      private: Ogre::v1::OverlaySystem *ogreOverlaySystem = nullptr;
      /// \internal
      /// \brief Get a pointer to the Ogre overlay system.
      /// \return Pointer to the OGRE overlay system.
      public: Ogre::v1::OverlaySystem *OverlaySystem() const;

      /// \brief Flag to indicate if engine is loaded
      private: bool loaded = false;

      /// \brief Flag to indicate if engine is initialized 
      private: bool initialized = false;

      private: Ogre2SceneStorePtr scenes;

//      private: OgreRenderPathType renderPathType;

      /// \brief Ogre root
      private: Ogre::Root *ogreRoot = nullptr;

      /// \brief Ogre log manager
      private: Ogre::LogManager *ogreLogManager = nullptr;

      /// \brief Paths to ogre plugins
      private: std::vector<std::string> ogrePaths;

#if not (__APPLE__ || _WIN32)
      private: void *dummyDisplay;

      private: void *dummyContext;
#endif

      private: uint64_t dummyWindowId;

      private: std::unique_ptr<Ogre2RenderEnginePrivate> dataPtr;

      private: friend class SingletonT<Ogre2RenderEngine>;
    };
  }
}
#endif

