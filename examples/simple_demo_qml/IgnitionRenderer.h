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

#ifndef IGNITIONRENDERER_H
#define IGNITIONRENDERER_H

#include <QSize>

#include <ignition/rendering.hh>

class IgnitionRenderer
{
public:
    virtual ~IgnitionRenderer();
    IgnitionRenderer();

    void render();
    void initialise();
    void initialiseOnMainThread();

    bool initialised() const;
    unsigned int textureId() const;
    QSize textureSize() const;

private:
    void initEngine();
    void updateCamera();

    unsigned int m_textureId = 0;
    QSize m_textureSize = QSize(800, 600);
    bool m_initialised = false;
    double m_cameraOffset = 0.0;
    ignition::rendering::CameraPtr m_camera;
};

#endif // IGNITIONRENDERER_H
