/*
 * Copyright (C) 2021 Rhys Mainwaring
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

#ifndef IGNITION_RENDERING_EXAMPLES_SIMPLE_DEMO_IGNITION_RENDERER_HH_
#define IGNITION_RENDERING_EXAMPLES_SIMPLE_DEMO_IGNITION_RENDERER_HH_

#include <QSize>

#include <ignition/rendering.hh>

class IgnitionRenderer
{
public:
    virtual ~IgnitionRenderer();
    IgnitionRenderer();

    void Render();
    void Initialise();
    void InitialiseOnMainThread();

    bool Initialised() const;
    unsigned int TextureId() const;
    QSize TextureSize() const;

private:
    void InitEngine();
    void UpdateCamera();

    unsigned int textureId = 0;
    QSize textureSize = QSize(800, 600);
    bool initialised = false;
    double cameraOffset = 0.0;
    ignition::rendering::CameraPtr camera;
};

#endif // IGNITION_RENDERING_EXAMPLES_SIMPLE_DEMO_IGNITION_RENDERER_HH_
