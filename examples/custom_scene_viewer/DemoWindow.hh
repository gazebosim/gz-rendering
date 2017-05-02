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
#ifndef IGNITION_RENDERING_EXAMPLES_DEMOWINDOW_HH_
#define IGNITION_RENDERING_EXAMPLES_DEMOWINDOW_HH_

#include <string>
#include <vector>
#include "TestTypes.hh"
#include "ignition/rendering/RenderTypes.hh"

namespace ignition
{
  namespace rendering
  {
    void GlutRun(ManualSceneDemoPtr _demo);

    void GlutDisplay();

    void GlutIdle();

    void GlutKeyboard(unsigned char _key, int _x, int _y);

    void GlutReshape(int _width, int _height);

    void GlutInitCamera(CameraPtr _camera);

    void GlutInitContext();

    void GlutPrintUsage();

    void GlutPrintEngine();

    void GlutPrintFPS();

    void GlutUpdateFPS();

    void GlutPrintText(const std::string &_text, int x, int y);

    void GlutPrintTextBack(const std::string &_text, int x, int y);

    void GlutPrintTextFore(const std::string &_text, int x, int y);

    void GlutPrintTextImpl(const std::string &_text, int x, int y);
  }
}

#endif
