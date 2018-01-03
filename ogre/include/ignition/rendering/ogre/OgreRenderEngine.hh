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
#ifndef IGNITION_RENDERING_OGRE_OGRERENDERENGINE_HH_
#define IGNITION_RENDERING_OGRE_OGRERENDERENGINE_HH_

#include <memory>
#include <string>
#include <vector>

#include <ignition/common/SingletonT.hh>

#include "ignition/rendering/base/BaseRenderEngine.hh"
#include "ignition/rendering/base/BaseRenderTypes.hh"
#include "ignition/rendering/ogre/OgreRenderTypes.hh"

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
    class OgreRenderEnginePrivate;

    class IGNITION_VISIBLE OgreRenderEngine :
      public virtual BaseRenderEngine,
      public common::SingletonT<OgreRenderEngine>
    {
      /// \enum RenderPathType
      /// \brief The type of rendering path used by the rendering engine.
      public: enum OgreRenderPathType
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

      /// \brief Constructor
      /// Note: This needs to be public in order for IGN_REGISTER_RENDER_ENGINE
      /// to work.
      public: OgreRenderEngine();
      // private: OgreRenderEngine();

      public: virtual ~OgreRenderEngine();

      public: virtual bool Fini();

      public: virtual bool IsEnabled() const;

      public: virtual std::string Name() const;

      public: OgreRenderPathType RenderPathType() const;

      public: void AddResourcePath(const std::string &_uri);

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

      private: void CreateResources();

      private: void CreateWindow();

      private: void CheckCapabilities();

      private: void InitAttempt();

      private: bool loaded;

      private: bool initialized;

      private: OgreSceneStorePtr scenes;

      private: OgreRenderPathType renderPathType;

      private: Ogre::Root *ogreRoot;

      private: Ogre::LogManager *ogreLogManager;

      /// \brief Paths to ogre plugins
      private: std::vector<std::string> ogrePaths;

#if not (__APPLE__ || _WIN32)
      private: void *dummyDisplay;

      private: void *dummyContext;
#endif

      private: uint64_t dummyWindowId;

      private: std::unique_ptr<OgreRenderEnginePrivate> dataPtr;
#ifdef OGRE_OVERLAY_NEEDED
      private: Ogre::OverlaySystem *ogreOverlaySystem;
#endif

      private: friend class SingletonT<OgreRenderEngine>;
    };
  }
}
#endif
