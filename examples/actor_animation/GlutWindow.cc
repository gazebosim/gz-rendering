/*
 * Copyright (C) 2017 Open Source Robotics Foundation
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

#if __APPLE__
  #include <OpenGL/gl.h>
  #include <OpenGL/OpenGL.h>
  #include <GLUT/glut.h>
#else
  #include <GL/glew.h>
  #include <GL/gl.h>
  #include <GL/glut.h>
#endif

#if not defined(__APPLE__) && not defined(_WIN32)
  #include <GL/glx.h>
#endif

#include <mutex>
#include <map>
#include <string>

#include <ignition/math/Matrix4.hh>
#include <ignition/common/Console.hh>
#include <ignition/common/Mesh.hh>
#include <ignition/common/SubMesh.hh>
#include <ignition/common/Skeleton.hh>
#include <ignition/common/SkeletonNode.hh>
#include <ignition/common/SkeletonAnimation.hh>
#include <ignition/rendering/Camera.hh>
#include <ignition/rendering/Image.hh>
#include <ignition/rendering/RayQuery.hh>
#include <ignition/rendering/Scene.hh>
#include <ignition/rendering/OrbitViewController.hh>
#include <ignition/rendering/TransformController.hh>

#include <ignition/common/MeshManager.hh>

#include "GlutWindow.hh"
#include "example_config.hh"

#define KEY_ESC 27
#define KEY_TAB  9

//////////////////////////////////////////////////
unsigned int imgw = 0;
unsigned int imgh = 0;

ir::CameraPtr g_camera;
ir::CameraPtr g_currCamera;
ir::ImagePtr g_image;
ir::MeshPtr g_mesh;
ic::SkeletonPtr g_skel;
ic::SkeletonAnimation *g_skelAnim;

bool g_initContext = false;

#if __APPLE__
  CGLContextObj g_context;
  CGLContextObj g_glutContext;
#elif _WIN32
#else
  GLXContext g_context;
  Display *g_display;
  GLXDrawable g_drawable;
  GLXContext g_glutContext;
  Display *g_glutDisplay;
  GLXDrawable g_glutDrawable;
#endif

// view control variables
ir::RayQueryPtr g_rayQuery;
ir::OrbitViewController g_viewControl;
ir::RayQueryResult g_target;
struct mouseButton
{
  int button = 0;
  int state = GLUT_UP;
  int x = 0;
  int y = 0;
  int motionX = 0;
  int motionY = 0;
  int dragX = 0;
  int dragY = 0;
  int scroll = 0;
  bool buttonDirty = false;
  bool motionDirty = false;
};
// transform control variables
ir::TransformController g_transformControl;
ir::TransformSpace g_space = ir::TransformSpace::TS_LOCAL;
ir::TransformMode g_mode = ir::TransformMode::TM_TRANSLATION;

struct mouseButton g_mouse;
std::mutex g_mouseMutex;

ir::ScenePtr scene;

//////////////////////////////////////////////////
void mouseCB(int _button, int _state, int _x, int _y)
{
  // ignore unknown mouse button numbers
  if (_button >= 5)
    return;

  std::lock_guard<std::mutex> lock(g_mouseMutex);
  g_mouse.button = _button;
  g_mouse.state = _state;
  g_mouse.x = _x;
  g_mouse.y = _y;
  g_mouse.motionX = _x;
  g_mouse.motionY = _y;
  g_mouse.buttonDirty = true;
}

//////////////////////////////////////////////////
void motionCB(int _x, int _y)
{
  std::lock_guard<std::mutex> lock(g_mouseMutex);
  int deltaX = _x - g_mouse.motionX;
  int deltaY = _y - g_mouse.motionY;
  g_mouse.motionX = _x;
  g_mouse.motionY = _y;

  if (g_mouse.motionDirty)
  {
    g_mouse.dragX += deltaX;
    g_mouse.dragY += deltaY;
  }
  else
  {
    g_mouse.dragX = deltaX;
    g_mouse.dragY = deltaY;
  }
  g_mouse.motionDirty = true;
}

//////////////////////////////////////////////////
void handleMouse()
{
  std::lock_guard<std::mutex> lock(g_mouseMutex);
  // only ogre supports ray query for now so use
  // ogre camera located at camera index = 0.
  ir::CameraPtr rayCamera = g_camera;
  if (!g_rayQuery)
  {
    g_rayQuery = rayCamera->Scene()->CreateRayQuery();
    if (!g_rayQuery)
    {
      ignerr << "Failed to create Ray Query" << std::endl;
      return;
    }
  }
  if (g_mouse.buttonDirty)
  {
    g_mouse.buttonDirty = false;
    double nx =
        2.0 * g_mouse.x / static_cast<double>(rayCamera->ImageWidth()) - 1.0;
    double ny = 1.0 -
        2.0 * g_mouse.y / static_cast<double>(rayCamera->ImageHeight());
    g_rayQuery->SetFromCamera(rayCamera, ignition::math::Vector2d(nx, ny));
    g_target  = g_rayQuery->ClosestPoint();
    if (!g_target)
    {
      // set point to be 10m away if no intersection found
      g_target.point = g_rayQuery->Origin() + g_rayQuery->Direction() * 10;
      return;
    }

    // mouse wheel scroll zoom
    if ((g_mouse.button == 3 || g_mouse.button == 4) &&
        g_mouse.state == GLUT_UP)
    {
      double scroll = (g_mouse.button == 3) ? -1.0 : 1.0;
      double distance = rayCamera->WorldPosition().Distance(
          g_target.point);
      int factor = 1;
      double amount = -(scroll * factor) * (distance / 5.0);

      g_viewControl.SetCamera(g_camera);
      g_viewControl.SetTarget(g_target.point);
      g_viewControl.Zoom(amount);
    }
  }

  if (g_mouse.motionDirty)
  {
    g_mouse.motionDirty = false;
    auto drag = ignition::math::Vector2d(g_mouse.dragX, g_mouse.dragY);

    // left mouse button pan
    if (g_mouse.button == GLUT_LEFT_BUTTON && g_mouse.state == GLUT_DOWN)
    {
      g_viewControl.SetCamera(g_camera);
      g_viewControl.SetTarget(g_target.point);
      g_viewControl.Pan(drag);
    }
    else if (g_mouse.button == GLUT_MIDDLE_BUTTON && g_mouse.state == GLUT_DOWN)
    {
      g_viewControl.SetCamera(g_camera);
      g_viewControl.SetTarget(g_target.point);
      g_viewControl.Orbit(drag);
    }
    // right mouse button zoom
    else if (g_mouse.button == GLUT_RIGHT_BUTTON && g_mouse.state == GLUT_DOWN)
    {
      double hfov = rayCamera->HFOV().Radian();
      double vfov = 2.0f * atan(tan(hfov / 2.0f) /
          rayCamera->AspectRatio());
      double distance = rayCamera->WorldPosition().Distance(
          g_target.point);
      double amount = ((-g_mouse.dragY /
          static_cast<double>(rayCamera->ImageHeight()))
          * distance * tan(vfov/2.0) * 6.0);
      g_viewControl.SetCamera(g_camera);
      g_viewControl.SetTarget(g_target.point);
      g_viewControl.Zoom(amount);
    }
  }
}

//////////////////////////////////////////////////
void updatePose(double _time)
{
  g_skelAnim = g_skel->Animation(0u);
  std::map<std::string, ignition::math::Matrix4d> animFrames;
  animFrames = g_skelAnim->PoseAt(_time, true);

  std::map<std::string, ignition::math::Matrix4d> skinFrames;

  for (auto pair : animFrames)
  {
    std::string animName = pair.first;
    auto animTf = pair.second;

    std::string skinName = g_skel->NodeNameAnimToSkin(animName);
    ignition::math::Matrix4d skinTf = g_skel->AlignTranslation(animName)
            * animTf * g_skel->AlignRotation(animName);

    skinFrames[skinName] = skinTf;
  }

  g_mesh->SetSkeletonLocalTransforms(skinFrames);
}
double curr_time = 0.0;

//////////////////////////////////////////////////
void updateActor()
{
  updatePose(curr_time);
  if (curr_time >= g_skelAnim->Length())
  {
    curr_time = 0;
  }
  curr_time+= 0.05;
}

//////////////////////////////////////////////////
void displayCB()
{
#if __APPLE__
  CGLSetCurrentContext(g_context);
#elif _WIN32
#else
  if (g_display)
  {
    glXMakeCurrent(g_display, g_drawable, g_context);
  }
#endif

  g_camera->Capture(*g_image);

  handleMouse();

#if __APPLE__
  CGLSetCurrentContext(g_glutContext);
#elif _WIN32
#else
  glXMakeCurrent(g_glutDisplay, g_glutDrawable, g_glutContext);
#endif

  unsigned char *data = g_image->Data<unsigned char>();

  glClearColor(0.5, 0.5, 0.5, 1);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glPixelZoom(1, -1);
  glRasterPos2f(-1, 1);
  glDrawPixels(imgw, imgh, GL_RGB, GL_UNSIGNED_BYTE, data);

  glutSwapBuffers();

  updateActor();
}

//////////////////////////////////////////////////
void idleCB()
{
  glutPostRedisplay();
}

//////////////////////////////////////////////////
void keyboardCB(unsigned char _key, int, int)
{
  if (_key == KEY_ESC || _key == 'q' || _key == 'Q')
  {
    exit(0);
  }
  else if (_key == 'r')
  {
    g_mode = ir::TransformMode::TM_ROTATION;
  }
  else if (_key == 't')
  {
    g_mode = ir::TransformMode::TM_TRANSLATION;
  }
  else if (_key == 's')
  {
    g_mode = ir::TransformMode::TM_SCALE;
  }
  else if (_key == 'g')
  {
    // toggle
    if (g_space == ir::TransformSpace::TS_LOCAL)
    {
      g_space = ir::TransformSpace::TS_WORLD;
      std::cout << "Transformation in World Space" << std::endl;
    }
    else
    {
      g_space = ir::TransformSpace::TS_LOCAL;
      std::cout << "Transformation in Local Space" << std::endl;
    }
  }
}

//////////////////////////////////////////////////
void initCamera(ir::CameraPtr _camera)
{
  g_camera = _camera;
  imgw = g_camera->ImageWidth();
  imgh = g_camera->ImageHeight();
  ir::Image image = g_camera->CreateImage();
  g_image = std::make_shared<ir::Image>(image);
  g_camera->Capture(*g_image);
}

//////////////////////////////////////////////////
void initContext()
{
  glutInitDisplayMode(GLUT_DOUBLE);
  glutInitWindowPosition(0, 0);
  glutInitWindowSize(imgw, imgh);
  glutCreateWindow("Mesh Viewer");
  glutDisplayFunc(displayCB);
  glutIdleFunc(idleCB);
  glutKeyboardFunc(keyboardCB);

  glutMouseFunc(mouseCB);
  glutMotionFunc(motionCB);
}

void initAnimation()
{
  if (!g_skel || g_skel->AnimationCount() == 0)
  {
    std::cout << "Failed to load animation." << std::endl;
    return;
  }
  const std::string RESOURCE_PATH =
      ic::joinPaths(std::string(PROJECT_BINARY_PATH), "media");
  std::string bvhFile = ic::joinPaths(RESOURCE_PATH, "cmu-13_26.bvh");
  double scale = 0.055;

  g_skel->AddBvhAnimation(bvhFile, scale);
  g_skelAnim = g_skel->Animation(0);
  std::cout << g_skelAnim->Length() << std::endl;
}

//////////////////////////////////////////////////
void printUsage()
{
  std::cout << "===============================" << std::endl;
  std::cout << "  TAB - Switch render engines  " << std::endl;
  std::cout << "  ESC - Exit                   " << std::endl;
  std::cout << "  t   - Translate Mode         " << std::endl;
  std::cout << "  r   - Rotate Mode            " << std::endl;
  std::cout << "  s   - Scale Mode             " << std::endl;
  std::cout << "  g   - Toggle Transform Space " << std::endl;
  std::cout << "===============================" << std::endl;
}

//////////////////////////////////////////////////
void run(ir::MeshPtr _mesh, ir::CameraPtr _camera, ic::SkeletonPtr _skel)
{
#if __APPLE__
  g_context = CGLGetCurrentContext();
#elif _WIN32
#else
  g_context = glXGetCurrentContext();
  g_display = glXGetCurrentDisplay();
  g_drawable = glXGetCurrentDrawable();
#endif

  g_mesh = _mesh;
  g_camera = _camera;
  g_skel = _skel;

  initCamera(_camera);
  initContext();
  initAnimation();
  printUsage();

#if __APPLE__
  g_glutContext = CGLGetCurrentContext();
#elif _WIN32
#else
  g_glutDisplay = glXGetCurrentDisplay();
  g_glutDrawable = glXGetCurrentDrawable();
  g_glutContext = glXGetCurrentContext();
#endif

  glutMainLoop();
}
