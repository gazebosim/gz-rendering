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
#include "gz/rendering/filament/FilamentWireBox.hh"

using namespace gz;
using namespace rendering;

//////////////////////////////////////////////////
FilamentWireBox::FilamentWireBox() = default;

//////////////////////////////////////////////////
FilamentWireBox::~FilamentWireBox() = default;

//////////////////////////////////////////////////
void FilamentWireBox::Init()
{
  BaseWireBox::Init();
  this->Create();
}

//////////////////////////////////////////////////
void FilamentWireBox::PreRender()
{
  if (this->wireBoxDirty)
  {
    this->Create();
    this->wireBoxDirty = false;
  }
  BaseWireBox::PreRender();
}

//////////////////////////////////////////////////
void FilamentWireBox::Create()
{
  this->lines->Clear();
  const math::Vector3d lo = this->box.Min();
  const math::Vector3d hi = this->box.Max();
  // corner i has bit 0 / 1 / 2 set for the max x / y / z
  auto corner = [&](int i)
  {
    return math::Vector3d((i & 1) ? hi.X() : lo.X(), (i & 2) ? hi.Y() : lo.Y(),
        (i & 4) ? hi.Z() : lo.Z());
  };
  for (int i = 0; i < 8; ++i)
  {
    for (int bit = 1; bit < 8; bit <<= 1)
    {
      if (!(i & bit))
      {
        this->lines->AddPoint(corner(i));
        this->lines->AddPoint(corner(i | bit));
      }
    }
  }
}
