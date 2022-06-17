/*
 * Copyright (C) 2021 Open Source Robotics Foundation
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


#include "gz/rendering/BoundingBox.hh"

using namespace gz;
using namespace rendering;

//////////////////////////////////////////////////
class gz::rendering::BoundingBoxPrivate
{
};

//////////////////////////////////////////////////
BoundingBox::BoundingBox() :
    dataPtr(std::make_unique<BoundingBoxPrivate>())
{
}

/////////////////////////////////////////////////
BoundingBox::~BoundingBox()
{
}

//////////////////////////////////////////////////
BoundingBox::BoundingBox(const BoundingBox &_texture)
  : dataPtr(std::make_unique<BoundingBoxPrivate>(*_texture.dataPtr))
{
}

//////////////////////////////////////////////////
BoundingBox::BoundingBox(BoundingBox &&_texture) noexcept
  : dataPtr(std::exchange(_texture.dataPtr, nullptr))
{
}

/////////////////////////////////////////////////
BoundingBox &BoundingBox::operator=(
    const BoundingBox &_texture)
{
  return *this = BoundingBox(_texture);
}

/////////////////////////////////////////////////
BoundingBox &BoundingBox::operator=(BoundingBox &&_texture)
{
  std::swap(this->dataPtr, _texture.dataPtr);
  return *this;
}

