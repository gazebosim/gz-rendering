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

#include <QGuiApplication>
#include <QQuickView>
#include <QDirIterator>

#include <exception>
#include <iostream>

int main(int _argc, char** _argv)
{
  QGuiApplication app(_argc, _argv);

  QQuickWindow::setGraphicsApi(QSGRendererInterface::Vulkan);

  QQuickView view;
  view.setResizeMode(QQuickView::SizeRootObjectToView);
  view.setSource(QUrl("qrc:///simple_demo_qml/main.qml"));
  view.resize(400, 400);

  int r = app.exec();
  return r;
}
