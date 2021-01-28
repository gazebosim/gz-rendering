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
#include "CameraWindow.hh"

#if __APPLE__
  #include <OpenGL/gl.h>
  #include <GLUT/glut.h>
#else
  #include <GL/glew.h>
  #include <GL/gl.h>
  #include <GL/glut.h>
#endif

#if !defined(__APPLE__) && !defined(_WIN32)
  #include <GL/glx.h>
#endif

#include <gazebo/common/Console.hh>

#include <ignition/rendering/Camera.hh>
#include <ignition/rendering/Image.hh>
#include <ignition/rendering/Scene.hh>

#include "SceneManager.hh"

#define KEY_ESC 27
#define KEY_TAB  9

//////////////////////////////////////////////////
unsigned int imgw = 0;
unsigned int imgh = 0;

std::vector<gz::CameraPtr> g_cameras;
gz::CameraPtr g_camera;
gz::CameraPtr g_currCamera;
unsigned int g_cameraIndex = 0;
gz::ImagePtr g_image;

bool g_initContext = false;

#if not (__APPLE__ || _WIN32)
  GLXContext g_context;
  Display *g_display;
  GLXDrawable g_drawable;
  GLXContext g_glutContext;
  Display *g_glutDisplay;
  GLXDrawable g_glutDrawable;
#endif

double g_offset = 0.0;

//////////////////////////////////////////////////
void GlutRun(std::vector<gz::CameraPtr> _cameras)
{
#if not (__APPLE__ || _WIN32)
  g_context = glXGetCurrentContext();
  g_display = glXGetCurrentDisplay();
  g_drawable = glXGetCurrentDrawable();
#endif

  g_cameras = _cameras;
  GlutInitCamera(_cameras[0]);
  GlutInitContext();
  GlutPrintUsage();

#if not (__APPLE__ || _WIN32)
  g_glutDisplay = glXGetCurrentDisplay();
  g_glutDrawable = glXGetCurrentDrawable();
  g_glutContext = glXGetCurrentContext();
#endif

  glutMainLoop();
}

//////////////////////////////////////////////////
void GlutDisplay()
{
#if not (__APPLE__ || _WIN32)
  if (g_display)
  {
    glXMakeCurrent(g_display, g_drawable, g_context);
  }
#endif

  g_cameras[g_cameraIndex]->Capture(*g_image);

#if not (__APPLE__ || _WIN32)
  glXMakeCurrent(g_glutDisplay, g_glutDrawable, g_glutContext);
#endif

  unsigned char *data = g_image->Data<unsigned char>();

  glClearColor(0.5, 0.5, 0.5, 1);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glPixelZoom(1, -1);
  glRasterPos2f(-1, 1);
  glDrawPixels(imgw, imgh, GL_RGB, GL_UNSIGNED_BYTE, data);

  glutSwapBuffers();
}

//////////////////////////////////////////////////
void GlutIdle()
{
#if not (__APPLE__ || _WIN32)
  if (g_display)
  {
    glXMakeCurrent(g_display, g_drawable, g_context);
  }
#endif

  ignition::rendering::SceneManager* manager =
      ignition::rendering::SceneManager::Instance();

  manager->UpdateScenes();

#if not (__APPLE__ || _WIN32)
  glXMakeCurrent(g_glutDisplay, g_glutDrawable, g_glutContext);
#endif

  glutPostRedisplay();
}

//////////////////////////////////////////////////
void GlutKeyboard(unsigned char _key, int, int)
{
  if (_key == KEY_ESC || _key == 'q' || _key == 'Q')
  {
    exit(0);
  }
  else if (_key == KEY_TAB)
  {
    g_cameraIndex = (g_cameraIndex + 1) % g_cameras.size();
  }
}

//////////////////////////////////////////////////
void GlutReshape(int, int)
{
}

//////////////////////////////////////////////////
void GlutInitCamera(gz::CameraPtr _camera)
{
  g_camera = _camera;
  imgw = g_camera->ImageWidth();
  imgh = g_camera->ImageHeight();
  gz::Image image = g_camera->CreateImage();
  g_image = std::make_shared<gz::Image>(image);
  g_camera->Capture(*g_image);
}

//////////////////////////////////////////////////
void GlutInitContext()
{
  int argc = 0;
  char **argv = 0;
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE);
  glutInitWindowPosition(0, 0);
  glutInitWindowSize(imgw, imgh);
  glutCreateWindow("Gazebo");
  glutDisplayFunc(GlutDisplay);
  glutIdleFunc(GlutIdle);
  glutKeyboardFunc(GlutKeyboard);
  glutReshapeFunc(GlutReshape);
}

void GlutPrintUsage()
{
  std::cout << "===============================" << std::endl;
  std::cout << "  TAB - Switch render engines  " << std::endl;
  std::cout << "  ESC - Exit                   " << std::endl;
  std::cout << "===============================" << std::endl;
}
