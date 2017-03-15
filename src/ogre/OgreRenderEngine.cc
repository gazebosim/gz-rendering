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

#include <boost/filesystem.hpp>

#include <ignition/common/Console.hh>
#include <ignition/common/Util.hh>

#ifndef _WIN32
  #include <dirent.h>
#else
  // Ensure that Winsock2.h is included before Windows.h, which can get
  // pulled in by anybody (e.g., Boost).
  #include <Winsock2.h>
  #include <ignition/common/win_dirent.h>
#endif


#include "ignition/rendering/SystemPaths.hh"

#include "ignition/rendering/ogre/OgreRenderEngine.hh"
#include "ignition/rendering/ogre/OgreRenderTypes.hh"
#include "ignition/rendering/ogre/OgreIncludes.hh"
#include "ignition/rendering/ogre/OgreRTShaderSystem.hh"
#include "ignition/rendering/ogre/OgreScene.hh"
#include "ignition/rendering/ogre/OgreStorage.hh"

#if not (__APPLE__ || _WIN32)
  #include <X11/Xlib.h>
  #include <X11/Xutil.h>
  #include <GL/glx.h>
#endif

using namespace ignition;
using namespace rendering;

//////////////////////////////////////////////////
OgreRenderEngine::OgreRenderEngine() :
  loaded(false),
  initialized(false),
  ogreRoot(nullptr),
  ogreLogManager(nullptr),
  dummyDisplay(0),
  dummyContext(0)
{
#if not (__APPLE__ || _WIN32)
  this->dummyDisplay = nullptr;
  this->dummyWindowId = 0;
#endif

#ifdef OGRE_OVERLAY_NEEDED
  this->ogreOverlaySystem = nullptr;
#endif
}

//////////////////////////////////////////////////
OgreRenderEngine::~OgreRenderEngine()
{
}

//////////////////////////////////////////////////
bool OgreRenderEngine::Fini()
{
  if (this->scenes)
  {
    this->scenes->RemoveAll();
  }

#if not (__APPLE__ || _WIN32)
  if (this->dummyDisplay)
  {
    Display *x11Display = static_cast<Display*>(this->dummyDisplay);
    GLXContext x11Context = static_cast<GLXContext>(this->dummyContext);
    glXDestroyContext(x11Display, x11Context);
    XDestroyWindow(x11Display, this->dummyWindowId);
    XCloseDisplay(x11Display);
    this->dummyDisplay = nullptr;
  }
#endif

#ifdef OGRE_OVERLAY_NEEDED
  delete this->ogreOverlaySystem;
  this->ogreOverlaySystem = nullptr;
#endif

  if (ogreRoot)
  {
    this->ogreRoot->shutdown();
    // TODO: fix segfault on delete
    // delete this->ogreRoot;
    this->ogreRoot = nullptr;
  }

  delete this->ogreLogManager;
  this->ogreLogManager = nullptr;

  this->loaded = false;
  this->initialized = false;

  return true;
}

//////////////////////////////////////////////////
bool OgreRenderEngine::IsEnabled() const
{
  return this->initialized && this->renderPathType != NONE;
}

//////////////////////////////////////////////////
std::string OgreRenderEngine::Name() const
{
  return "Ogre";
}

//////////////////////////////////////////////////
OgreRenderEngine::OgreRenderPathType
    OgreRenderEngine::RenderPathType() const
{
  return this->renderPathType;
}

//////////////////////////////////////////////////
void OgreRenderEngine::AddResourcePath(const std::string &_uri)
{
  if (_uri == "__default__" || _uri.empty())
    return;

  std::string path = common::findFilePath(_uri);

  if (path.empty())
  {
    ignerr << "URI doesn't exist[" << _uri << "]\n";
    return;
  }

  try
  {
    if (!Ogre::ResourceGroupManager::getSingleton().resourceLocationExists(
          path, "General"))
    {
      Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
          path, "FileSystem", "General", true);

      Ogre::ResourceGroupManager::getSingleton().initialiseResourceGroup(
          "General");
      // Parse all material files in the path if any exist
      boost::filesystem::path dir(path);

      if (boost::filesystem::exists(dir) &&
          boost::filesystem::is_directory(dir))
      {
        std::vector<boost::filesystem::path> paths;

        std::copy(boost::filesystem::directory_iterator(dir),
            boost::filesystem::directory_iterator(),
            std::back_inserter(paths));

        std::sort(paths.begin(), paths.end());

        // Iterate over all the models in the current ign-rendering path
        for (std::vector<boost::filesystem::path>::iterator dIter =
            paths.begin(); dIter != paths.end(); ++dIter)
        {
          if (dIter->filename().extension() == ".material")
          {
            boost::filesystem::path fullPath = path / dIter->filename();

            Ogre::DataStreamPtr stream =
              Ogre::ResourceGroupManager::getSingleton().openResource(
                  fullPath.string(), "General");

            // There is a material file under there somewhere, read the thing in
            try
            {
              Ogre::MaterialManager::getSingleton().parseScript(
                  stream, "General");
              Ogre::MaterialPtr matPtr =
                Ogre::MaterialManager::getSingleton().getByName(
                    fullPath.string());

              if (!matPtr.isNull())
              {
                // is this necessary to do here? Someday try it without
                matPtr->compile();
                matPtr->load();
              }
            }
            catch(Ogre::Exception& e)
            {
              ignerr << "Unable to parse material file[" << fullPath << "]\n";
            }
            stream->close();
          }
        }
      }
    }
  }
  catch(Ogre::Exception &/*_e*/)
  {
    ignerr << "Unable to load Ogre Resources.\nMake sure the"
        "resources path in the world file is set correctly." << std::endl;
  }
}

//////////////////////////////////////////////////
Ogre::Root *OgreRenderEngine::OgreRoot() const
{
  return this->ogreRoot;
}

//////////////////////////////////////////////////
ScenePtr OgreRenderEngine::CreateSceneImpl(unsigned int _id,
    const std::string &_name)
{
  return OgreScenePtr(new OgreScene(_id, _name));
}

//////////////////////////////////////////////////
SceneStorePtr OgreRenderEngine::Scenes() const
{
  return this->scenes;
}

//////////////////////////////////////////////////
bool OgreRenderEngine::LoadImpl()
{
  try
  {
    this->LoadAttempt();
    return true;
  }
  catch (Ogre::Exception &ex)
  {
    ignerr << ex.what() << std::endl;
    return false;
  }
  catch (...)
  {
    ignerr << "Failed to load render-engine" << std::endl;
    return false;
  }
}

//////////////////////////////////////////////////
bool OgreRenderEngine::InitImpl()
{
  try
  {
    this->InitAttempt();
    return true;
  }
  catch (...)
  {
    ignerr << "Failed to initialize render-engine" << std::endl;
    return false;
  }
}

//////////////////////////////////////////////////
void OgreRenderEngine::LoadAttempt()
{
  this->CreateLogger();
  this->CreateContext();
  this->CreateRoot();
  this->CreateOverlay();
  this->LoadPlugins();
  this->CreateRenderSystem();
  this->ogreRoot->initialise(false);
  this->CreateResources();
  this->CreateWindow();
  this->CheckCapabilities();
}

//////////////////////////////////////////////////
void OgreRenderEngine::CreateLogger()
{
  // create log file path
  std::string logPath = SystemPaths::Instance()->LogPath();
  logPath += "/ogre.log";

  // create actual log
  this->ogreLogManager = new Ogre::LogManager();
  this->ogreLogManager->createLog(logPath, true, false, false);
}

//////////////////////////////////////////////////
void OgreRenderEngine::CreateContext()
{
#if not (__APPLE__ || _WIN32)
  // create X11 display
  this->dummyDisplay = XOpenDisplay(0);
  Display *x11Display = static_cast<Display*>(this->dummyDisplay);

  if (!this->dummyDisplay)
  {
    ignerr << "Unable to open dipslay: " << XDisplayName(0) << std::endl;
    return;
  }

  // create X11 visual
  int screenId = DefaultScreen(x11Display);

  int attributeList[] = { GLX_RGBA, GLX_DOUBLEBUFFER, GLX_DEPTH_SIZE, 16,
      GLX_STENCIL_SIZE, 8, None };

  XVisualInfo *dummyVisual =
      glXChooseVisual(x11Display, screenId, attributeList);

  if (!dummyVisual)
  {
    ignerr << "Unable to create glx visual" << std::endl;
    return;
  }

  // create X11 context
  this->dummyWindowId = XCreateSimpleWindow(x11Display,
      RootWindow(this->dummyDisplay, screenId), 0, 0, 1, 1, 0, 0, 0);

  this->dummyContext = glXCreateContext(x11Display, dummyVisual, nullptr, 1);

  GLXContext x11Context = static_cast<GLXContext>(this->dummyContext);

  if (!this->dummyContext)
  {
    ignerr << "Unable to create glx context" << std::endl;
    return;
  }

  // select X11 context
  glXMakeCurrent(x11Display, this->dummyWindowId, x11Context);
#endif
}

//////////////////////////////////////////////////
void OgreRenderEngine::CreateRoot()
{
  try
  {
    this->ogreRoot = new Ogre::Root();
  }
  catch (Ogre::Exception &ex)
  {
    ignerr << "Unable to create Ogre root" << std::endl;
  }
}

//////////////////////////////////////////////////
void OgreRenderEngine::CreateOverlay()
{
#ifdef OGRE_OVERLAY_NEEDED
  this->ogreOverlaySystem = new Ogre::OverlaySystem();
#endif
}

//////////////////////////////////////////////////
void OgreRenderEngine::LoadPlugins()
{
  std::list<std::string>::iterator iter;
  std::list<std::string> ogrePaths =
    SystemPaths::Instance()->OgrePaths();

  for (iter = ogrePaths.begin();
       iter != ogrePaths.end(); ++iter)
  {
    std::string path(*iter);
    DIR *dir = opendir(path.c_str());

    if (dir == nullptr)
    {
      continue;
    }

    closedir(dir);

    std::vector<std::string> plugins;
    std::vector<std::string>::iterator piter;

#ifdef __APPLE__
    std::string prefix = "lib";
    std::string extension = ".dylib";
#else
    std::string prefix = "";
    std::string extension = ".so";
#endif

    plugins.push_back(path+"/"+prefix+"RenderSystem_GL");
    plugins.push_back(path+"/"+prefix+"Plugin_ParticleFX");
    plugins.push_back(path+"/"+prefix+"Plugin_BSPSceneManager");
    plugins.push_back(path+"/"+prefix+"Plugin_OctreeSceneManager");

#ifdef HAVE_OCULUS
    plugins.push_back(path+"/Plugin_CgProgramManager");
#endif

    for (piter = plugins.begin(); piter != plugins.end(); ++piter)
    {
      try
      {
        // Load the plugin into OGRE
        this->ogreRoot->loadPlugin(*piter+extension);
      }
      catch(Ogre::Exception &e)
      {
        try
        {
          // Load the debug plugin into OGRE
          this->ogreRoot->loadPlugin(*piter+"_d"+extension);
        }
        catch(Ogre::Exception &ed)
        {
          if ((*piter).find("RenderSystem") != std::string::npos)
          {
            ignerr << "Unable to load Ogre Plugin[" << *piter
                  << "]. Rendering will not be possible."
                  << "Make sure you have installed OGRE and Gazebo properly.\n";
          }
        }
      }
    }
  }
}

//////////////////////////////////////////////////
void OgreRenderEngine::CreateRenderSystem()
{
  Ogre::RenderSystem *renderSys;
  const Ogre::RenderSystemList *rsList;

  // Set parameters of render system (window size, etc.)
#if  OGRE_VERSION_MAJOR == 1 && OGRE_VERSION_MINOR == 6
  rsList = this->ogreRoot->getAvailableRenderers();
#else
  rsList = &(this->ogreRoot->getAvailableRenderers());
#endif

  int c = 0;

  renderSys = nullptr;

  do
  {
    if (c == static_cast<int>(rsList->size()))
      break;

    renderSys = rsList->at(c);
    c++;
  }
  while (renderSys &&
         renderSys->getName().compare("OpenGL Rendering Subsystem") != 0);

  if (renderSys == nullptr)
  {
    ignerr << "unable to find OpenGL rendering system. OGRE is probably "
            "installed incorrectly. Double check the OGRE cmake output, "
            "and make sure OpenGL is enabled." << std::endl;
  }

  // We operate in windowed mode
  renderSys->setConfigOption("Full Screen", "No");

  /// We used to allow the user to set the RTT mode to PBuffer, FBO, or Copy.
  ///   Copy is slow, and there doesn't seem to be a good reason to use it
  ///   PBuffer limits the size of the renderable area of the RTT to the
  ///           size of the first window created.
  ///   FBO seem to be the only good option
  renderSys->setConfigOption("RTT Preferred Mode", "FBO");

  renderSys->setConfigOption("FSAA", "4");

  this->ogreRoot->setRenderSystem(renderSys);
}

//////////////////////////////////////////////////
void OgreRenderEngine::CreateResources()
{
  std::vector< std::pair<std::string, std::string> > archNames;
  std::vector< std::pair<std::string, std::string> >::iterator aiter;
  std::list<std::string>::const_iterator iter;
  // std::list<std::string> paths = SystemPaths::Instance()->GetGazeboPaths();
  // TODO do this in gazebo plugin
  std::list<std::string> paths;


  std::list<std::string> mediaDirs;
  mediaDirs.push_back("media");
  mediaDirs.push_back("Media");

  for (iter = paths.begin(); iter != paths.end(); ++iter)
  {
    DIR *dir;
    if ((dir = opendir((*iter).c_str())) == nullptr)
    {
      continue;
    }
    closedir(dir);

    archNames.push_back(
        std::make_pair((*iter)+"/", "General"));

    for (std::list<std::string>::iterator mediaIter = mediaDirs.begin();
         mediaIter != mediaDirs.end(); ++mediaIter)
    {
      std::string prefix = (*iter) + "/" + (*mediaIter);

      archNames.push_back(
          std::make_pair(prefix, "General"));
      archNames.push_back(
          std::make_pair(prefix + "/skyx", "SkyX"));
      archNames.push_back(
          std::make_pair(prefix + "/rtshaderlib", "General"));
      archNames.push_back(
          std::make_pair(prefix + "/materials/programs", "General"));
      archNames.push_back(
          std::make_pair(prefix + "/materials/scripts", "General"));
      archNames.push_back(
          std::make_pair(prefix + "/materials/textures", "General"));
      archNames.push_back(
          std::make_pair(prefix + "/media/models", "General"));
      archNames.push_back(
          std::make_pair(prefix + "/fonts", "Fonts"));
      archNames.push_back(
          std::make_pair(prefix + "/gui/looknfeel", "LookNFeel"));
      archNames.push_back(
          std::make_pair(prefix + "/gui/schemes", "Schemes"));
      archNames.push_back(
          std::make_pair(prefix + "/gui/imagesets", "Imagesets"));
      archNames.push_back(
          std::make_pair(prefix + "/gui/fonts", "Fonts"));
      archNames.push_back(
          std::make_pair(prefix + "/gui/layouts", "Layouts"));
      archNames.push_back(
          std::make_pair(prefix + "/gui/animations", "Animations"));
    }

    for (aiter = archNames.begin(); aiter != archNames.end(); ++aiter)
    {
      try
      {
        Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
            aiter->first, "FileSystem", aiter->second);
      }
      catch(Ogre::Exception &/*_e*/)
      {
        ignerr << "Unable to load Ogre Resources. Make sure the resources "
            "path in the world file is set correctly." << std::endl;
      }
    }
  }
}

//////////////////////////////////////////////////
void OgreRenderEngine::CreateWindow()
{
  Ogre::StringVector paramsVector;
  Ogre::NameValuePairList params;
  Ogre::RenderWindow *window = nullptr;

  // Mac and Windows *must* use externalWindow handle.
#if defined(__APPLE__) || defined(_MSC_VER)
  params["externalWindowHandle"] = std::to_string(this->dummyWindowId);
#else
  params["parentWindowHandle"] = std::to_string(this->dummyWindowId);
#endif
  params["FSAA"] = "4";
  params["stereoMode"] = "Frame Sequential";

  // TODO: determine api without qt

  // Set the macAPI for Ogre based on the Qt implementation
// #ifdef QT_MAC_USE_COCOA
  params["macAPI"] = "cocoa";
  params["macAPICocoaUseNSView"] = "true";
// #else
//   params["macAPI"] = "carbon";
// #endif

  // Hide window if dimensions are less than or equal to one.
  params["border"] = "none";

  std::ostringstream stream;
  stream << "OgreWindow(0)";

  int attempts = 0;
  while (window == nullptr && (attempts++) < 10)
  {
    try
    {
      window = this->ogreRoot->createRenderWindow(
          stream.str(), 1, 1, false, &params);
    }
    catch(...)
    {
      ignerr << " Unable to create the rendering window\n";
      window = nullptr;
    }
  }

  if (attempts >= 10)
  {
    ignerr << "Unable to create the rendering window\n" << std::endl;
    return;
  }

  if (window)
  {
    window->setActive(true);
    window->setVisible(true);
    window->setAutoUpdated(false);

    // Windows needs to reposition the render window to 0,0.
    window->reposition(0, 0);
  }
}

//////////////////////////////////////////////////
void OgreRenderEngine::CheckCapabilities()
{
  const Ogre::RenderSystemCapabilities *capabilities;
  Ogre::RenderSystemCapabilities::ShaderProfiles profiles;
  Ogre::RenderSystemCapabilities::ShaderProfiles::const_iterator iter;

  capabilities = this->ogreRoot->getRenderSystem()->getCapabilities();
  profiles = capabilities->getSupportedShaderProfiles();

  bool hasFragmentPrograms =
    capabilities->hasCapability(Ogre::RSC_FRAGMENT_PROGRAM);

  bool hasVertexPrograms =
    capabilities->hasCapability(Ogre::RSC_VERTEX_PROGRAM);

  bool hasFBO =
    capabilities->hasCapability(Ogre::RSC_FBO);

  bool hasGLSL =
    std::find(profiles.begin(), profiles.end(), "glsl") != profiles.end();

  if (!hasFragmentPrograms || !hasVertexPrograms)
    ignwarn << "Vertex and fragment shaders are missing. "
           << "Fixed function rendering will be used.\n";

  if (!hasGLSL)
    ignwarn << "GLSL is missing."
           << "Fixed function rendering will be used.\n";

  if (!hasFBO)
    ignwarn << "Frame Buffer Objects (FBO) is missing. "
           << "Rendering will be disabled.\n";

  this->renderPathType = OgreRenderEngine::NONE;

  if (hasFBO && hasGLSL && hasVertexPrograms && hasFragmentPrograms)
  {
    this->renderPathType = OgreRenderEngine::FORWARD;
  }
  else if (hasFBO)
  {
    this->renderPathType = OgreRenderEngine::VERTEX;
  }
}

//////////////////////////////////////////////////
void OgreRenderEngine::InitAttempt()
{
  if (this->renderPathType == NONE)
  {
    ignwarn << "Cannot initialize render engine since "
           << "render path type is NONE. Ignore this warning if"
           << "rendering has been turned off on purpose.\n";
    return;
  }

  this->initialized = false;

  Ogre::ColourValue ambient;

  /// Create a dummy rendering context.
  /// This will allow ign-rendering to run headless. And it also allows OGRE to
  /// initialize properly

  // Set default mipmap level (NB some APIs ignore this)
  Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);

  // init the resources
  Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

  Ogre::MaterialManager::getSingleton().setDefaultTextureFiltering(
      Ogre::TFO_ANISOTROPIC);

  OgreRTShaderSystem::Instance()->Init();

  this->scenes = OgreSceneStorePtr(new OgreSceneStore);
}

//////////////////////////////////////////////////
