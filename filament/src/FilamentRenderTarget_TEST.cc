/*
 * Copyright (C) 2026 Open Source Robotics Foundation
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

#include <gtest/gtest.h>

#include <vector>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GL/gl.h>
#include <GL/glext.h>

#include "gz/rendering/Camera.hh"
#include "gz/rendering/Image.hh"
#include "gz/rendering/RenderingIface.hh"
#include "gz/rendering/Scene.hh"
#include "gz/rendering/Visual.hh"
#include "gz/rendering/filament/FilamentRenderEngine.hh"

using namespace gz;
using namespace rendering;

namespace
{
  template <typename T>
  T GL(const char *_name)
  {
    return reinterpret_cast<T>(eglGetProcAddress(_name));
  }
}

/////////////////////////////////////////////////
// A caller sharing its GL context with the engine (gz-gui's zero-copy view)
// samples RenderTextureGLId() as is, and shows row 0 at the top like ogre2's
// texture: the texture must hold the image top row first, as Copy() returns
// it. Picking takes top-left based coordinates either way.
TEST(FilamentRenderTargetTest, SharedTextureTopRowFirst)
{
  // the caller's context: GLES 3, surfaceless, current on this thread
  EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  if (display == EGL_NO_DISPLAY || !eglInitialize(display, nullptr, nullptr))
    GTEST_SKIP() << "No EGL display";
  const EGLint configAttribs[] = {
      EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT, EGL_SURFACE_TYPE, 0,
      EGL_NONE};
  EGLConfig config;
  EGLint count = 0;
  const EGLint contextAttribs[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};
  EGLContext context = EGL_NO_CONTEXT;
  if (eglBindAPI(EGL_OPENGL_ES_API) &&
      eglChooseConfig(display, configAttribs, &config, 1, &count) &&
      count == 1)
  {
    context = eglCreateContext(display, config, EGL_NO_CONTEXT,
        contextAttribs);
  }
  if (context == EGL_NO_CONTEXT ||
      !eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, context))
  {
    GTEST_SKIP() << "No surfaceless GLES 3 context";
  }

  // through the plugin loader: the plugin's engine singleton, not a copy
  // of it in this binary. gz-gui's parameter: share the current context.
  RenderEngine *engine = rendering::engine("filament",
      {{"useCurrentGLContext", "1"}});
  ASSERT_NE(nullptr, engine);
  auto *filamentEngine = dynamic_cast<FilamentRenderEngine *>(engine);
  ASSERT_NE(nullptr, filamentEngine);
  ASSERT_TRUE(filamentEngine->SharesCallerGLContext());

  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);
  scene->SetBackgroundColor(0.0, 0.0, 0.0);
  scene->SetAmbientLight(1.0, 1.0, 1.0);
  // a box above the camera's axis: the top half of the image only
  VisualPtr box = scene->CreateVisual("box");
  box->AddGeometry(scene->CreateBox());
  MaterialPtr red = scene->CreateMaterial();
  red->SetDiffuse(1.0, 0.0, 0.0);
  red->SetEmissive(1.0, 0.0, 0.0);
  box->SetMaterial(red);
  box->SetLocalPosition(2.0, 0.0, 0.6);
  scene->RootVisual()->AddChild(box);

  const unsigned int width = 64u;
  const unsigned int height = 48u;
  CameraPtr camera = scene->CreateCamera("camera");
  ASSERT_NE(nullptr, camera);
  camera->SetImageWidth(width);
  camera->SetImageHeight(height);
  camera->SetHFOV(GZ_PI / 2.0);
  scene->RootVisual()->AddChild(camera);
  camera->Update();

  Image image = camera->CreateImage();
  camera->Copy(image);
  const unsigned char *rgb = image.Data<unsigned char>();
  // the box is in the top rows, the bottom rows are background
  EXPECT_GT(rgb[(4u * width + width / 2u) * 3u], 100u);
  EXPECT_EQ(0u, rgb[((height - 5u) * width + width / 2u) * 3u]);

  const unsigned int id = camera->RenderTextureGLId();
  ASSERT_NE(0u, id);
  // read the shared texture in the caller's context: glReadPixels returns
  // texture row 0 first
  GLuint fbo = 0;
  GL<PFNGLGENFRAMEBUFFERSPROC>("glGenFramebuffers")(1, &fbo);
  GL<PFNGLBINDFRAMEBUFFERPROC>("glBindFramebuffer")(GL_FRAMEBUFFER, fbo);
  GL<PFNGLFRAMEBUFFERTEXTURE2DPROC>("glFramebufferTexture2D")(GL_FRAMEBUFFER,
      GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, id, 0);
  std::vector<unsigned char> rgba(width * height * 4u);
  GL<decltype(&glReadPixels)>("glReadPixels")(0, 0,
      static_cast<GLsizei>(width), static_cast<GLsizei>(height), GL_RGBA,
      GL_UNSIGNED_BYTE, rgba.data());
  GL<PFNGLBINDFRAMEBUFFERPROC>("glBindFramebuffer")(GL_FRAMEBUFFER, 0);
  GL<PFNGLDELETEFRAMEBUFFERSPROC>("glDeleteFramebuffers")(1, &fbo);

  unsigned int differ = 0u;
  for (unsigned int i = 0u; i < width * height; ++i)
  {
    for (unsigned int c = 0u; c < 3u; ++c)
      differ += rgba[i * 4u + c] != rgb[i * 3u + c];
  }
  EXPECT_EQ(0u, differ);

  // mouse coordinates: origin at the top-left
  VisualPtr top = camera->VisualAt(math::Vector2i(width / 2, 4));
  ASSERT_NE(nullptr, top);
  EXPECT_EQ(box->Name(), top->Name());
  EXPECT_EQ(nullptr, camera->VisualAt(math::Vector2i(width / 2, height - 5)));

  rendering::unloadEngine(engine->Name());
  eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
  eglDestroyContext(display, context);
}
