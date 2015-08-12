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
#include "GlutWindow.hh"

#include <boost/make_shared.hpp>
#include <GL/glut.h>
#include <GL/gl.h>

#include "gazebo/common/Console.hh"
#include "ignition/rendering/Camera.hh"
#include "ignition/rendering/Image.hh"
#include "ignition/rendering/Scene.hh"

#include <GL/glx.h>

#define KEY_ESC 27

//////////////////////////////////////////////////
unsigned int imgw = 0;
unsigned int imgh = 0;
gz::CameraPtr g_camera;
gz::ImagePtr g_image;
bool g_initContext = false;

//////////////////////////////////////////////////
void GlutRun(gz::CameraPtr _camera)
{
  GlutInitCamera(_camera);
  GlutInitContext();
  glutMainLoop();
}

//////////////////////////////////////////////////
void GlutDisplay()
{
  g_camera->Capture(*g_image);

  unsigned char *data = g_image->GetData<unsigned char>();

  // share the context
  if (!g_initContext)
  {
    g_initContext = true;
    int attributeList[] = {GLX_RGBA, None};

    GLXContext context = glXGetCurrentContext();
    Display *display = glXGetCurrentDisplay();
    GLXDrawable drawable = glXGetCurrentDrawable();

    XVisualInfo * visualInfo = glXChooseVisual(display, 0, attributeList);
    GLXContext currentContext
        = glXCreateContext(display, visualInfo, context, GL_TRUE);
    glXMakeCurrent(display, drawable, currentContext);
  }


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
  glutPostRedisplay();
}

//////////////////////////////////////////////////
void GlutKeyboard(unsigned char _key, int, int)
{
  if (_key == KEY_ESC || _key == 'q' || _key == 'Q')
  {
    exit(0);
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
  imgw = g_camera->GetImageWidth();
  imgh = g_camera->GetImageHeight();
  gz::Image image = g_camera->CreateImage();
  g_image = boost::make_shared<gz::Image>(image);
  _camera->Capture(*g_image);
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
