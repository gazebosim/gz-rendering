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

// Built with -fno-rtti like filament, whose classes have no typeinfo to
// derive from (see CMakeLists.txt).

#include <backend/platforms/PlatformEGLHeadless.h>

namespace
{
/// \brief Filament's EGL platform on the caller's EGLDisplay, so that its
/// context can share objects (the camera textures) with the caller's.
class SharedEglPlatform : public filament::backend::PlatformEGLHeadless
{
  public: explicit SharedEglPlatform(EGLDisplay _display)
  {
    this->setEglDisplay(_display);
  }

  // ponytail: the display belongs to the caller, so no eglTerminate; filament's
  // contexts leak when the engine is destroyed (at process exit in a GUI).
  protected: void terminate() noexcept override
  {
    eglMakeCurrent(this->getEglDisplay(), EGL_NO_SURFACE, EGL_NO_SURFACE,
        EGL_NO_CONTEXT);
    eglReleaseThread();
  }
};
}

/// \brief Used by FilamentRenderEngine
filament::backend::Platform *CreateSharedEglPlatform(void *_eglDisplay)
{
  return new SharedEglPlatform(static_cast<EGLDisplay>(_eglDisplay));
}
