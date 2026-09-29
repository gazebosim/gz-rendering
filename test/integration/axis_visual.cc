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

#include "CommonRenderingTest.hh"

#include "gz/rendering/ArrowVisual.hh"
#include "gz/rendering/AxisVisual.hh"
#include "gz/rendering/Camera.hh"
#include "gz/rendering/Image.hh"
#include "gz/rendering/PixelFormat.hh"
#include "gz/rendering/Scene.hh"

#include <gz/utils/ExtraTestMacros.hh>

using namespace gz;
using namespace rendering;

/// \brief Scale of the visuals under test, so that the thin rotation
/// visuals cover whole pixels.
static constexpr double kScale = 20.0;

class AxisVisualTest : public CommonRenderingTest
{
  /// \brief Create a scene with a camera looking down the z axis.
  public: void SetUpScene()
  {
    this->scene = engine->CreateScene("scene");
    ASSERT_NE(nullptr, this->scene);
    this->scene->SetBackgroundColor(0, 0, 0);
    this->scene->SetAmbientLight(1, 1, 1);

    this->camera = this->scene->CreateCamera();
    ASSERT_NE(nullptr, this->camera);
    this->camera->SetImageWidth(200);
    this->camera->SetImageHeight(200);
    this->camera->SetHFOV(0.4);
    this->camera->SetLocalPosition(0, 0, 2 * kScale);
    this->camera->SetLocalRotation(0, GZ_PI / 2, 0);
    this->scene->RootVisual()->AddChild(this->camera);

    this->image = this->camera->CreateImage();
  }

  /// \brief Render and count the pixels that are not background.
  public: unsigned int CountNonBackgroundPixels()
  {
    this->camera->Capture(this->image);
    const unsigned char *data = this->image.Data<unsigned char>();
    const unsigned int bpp =
        PixelUtil::BytesPerPixel(this->camera->ImageFormat());
    const unsigned int size =
        this->camera->ImageWidth() * this->camera->ImageHeight() * bpp;

    unsigned int count = 0u;
    for (unsigned int i = 0u; i < size; i += bpp)
    {
      if (data[i] > 0u || data[i + 1] > 0u || data[i + 2] > 0u)
        ++count;
    }
    return count;
  }

  /// \brief Scene to render
  public: ScenePtr scene;

  /// \brief Downward looking camera
  public: CameraPtr camera;

  /// \brief Image to capture into
  public: Image image;
};

/////////////////////////////////////////////////
// Regression test for gazebosim/gz-rendering#771: showing the parent of an
// axis visual must not show the rotation visuals of the axis arrows.
TEST_F(AxisVisualTest,
    GZ_UTILS_TEST_DISABLED_ON_WIN32(ParentSetVisibleKeepsRotationHidden))
{
  // Ogre 1.x still cascades the visibility of a visual to its children.
  CHECK_SUPPORTED_ENGINE("ogre2");

  ASSERT_NO_FATAL_FAILURE(this->SetUpScene());

  VisualPtr parent = this->scene->CreateVisual();
  this->scene->RootVisual()->AddChild(parent);

  AxisVisualPtr axis = this->scene->CreateAxisVisual();
  ASSERT_NE(nullptr, axis);
  axis->SetLocalScale(kScale, kScale, kScale);
  parent->AddChild(axis);

  parent->SetVisible(false);
  EXPECT_EQ(0u, this->CountNonBackgroundPixels());

  parent->SetVisible(true);
  const unsigned int parentShown = this->CountNonBackgroundPixels();

  // Showing the axis itself keeps the rotation visuals hidden
  axis->SetVisible(true);
  const unsigned int axisShown = this->CountNonBackgroundPixels();

  EXPECT_GT(axisShown, 0u);
  EXPECT_EQ(axisShown, parentShown);

  engine->DestroyScene(this->scene);
}

/////////////////////////////////////////////////
// A rotation visual that was explicitly shown must stay visible when the
// parent of the arrow is shown again, e.g. for revolute joint visuals.
TEST_F(AxisVisualTest,
    GZ_UTILS_TEST_DISABLED_ON_WIN32(ParentSetVisibleKeepsShownRotation))
{
  CHECK_SUPPORTED_ENGINE("ogre2");

  ASSERT_NO_FATAL_FAILURE(this->SetUpScene());

  VisualPtr parent = this->scene->CreateVisual();
  this->scene->RootVisual()->AddChild(parent);

  ArrowVisualPtr arrow = this->scene->CreateArrowVisual();
  ASSERT_NE(nullptr, arrow);
  arrow->SetLocalScale(kScale, kScale, kScale);
  parent->AddChild(arrow);

  arrow->ShowArrowRotation(false);
  const unsigned int rotationHidden = this->CountNonBackgroundPixels();

  arrow->ShowArrowRotation(true);
  parent->SetVisible(false);
  parent->SetVisible(true);
  const unsigned int rotationShown = this->CountNonBackgroundPixels();

  EXPECT_GT(rotationHidden, 0u);
  EXPECT_GT(rotationShown, rotationHidden);

  engine->DestroyScene(this->scene);
}
