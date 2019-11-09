/*
 * Copyright (C) 2019 Open Source Robotics Foundation
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
#include <math.h>

#include <cmath>
#include <sstream>
#include <ignition/math/Color.hh>

#include "ignition/common/Console.hh"
#include "ignition/rendering/ogre2/Ogre2DynamicLines.hh"

using namespace ignition;
using namespace rendering;

enum {POSITION_BINDING, TEXCOORD_BINDING};


/// \brief Private implementation
class ignition::rendering::Ogre2DynamicLinesPrivate
{
  /// \brief list of colors at each point
  public: std::vector<ignition::math::Color> colors;

  /// \brief List of points for the line
  public: std::vector<ignition::math::Vector3d> points;

  /// \brief Used to indicate if the lines require an update
  public: bool dirty = false;
};

/////////////////////////////////////////////////
Ogre2DynamicLines::Ogre2DynamicLines(Ogre::IdType _id,
    Ogre::ObjectMemoryManager *_objectMemoryManager,
    Ogre::SceneManager *_manager, MarkerType _opType)
  : Ogre2DynamicRenderable(_id, _objectMemoryManager, _manager),
    dataPtr(new Ogre2DynamicLinesPrivate)
{
  this->Init(_opType, false);
  this->setCastShadows(false);
  this->dataPtr->dirty = true;
}

/////////////////////////////////////////////////
Ogre2DynamicLines::~Ogre2DynamicLines()
{
}

/////////////////////////////////////////////////
std::string Ogre2DynamicLines::MovableType()
{
  return "ignition::rendering::ogre2dynamiclines";
}

/////////////////////////////////////////////////
const Ogre::String &Ogre2DynamicLines::getMovableType() const
{
  static Ogre::String moveType = Ogre2DynamicLines::MovableType();
  return moveType;
}

/////////////////////////////////////////////////
void Ogre2DynamicLines::AddPoint(const ignition::math::Vector3d &_pt,
                            const ignition::math::Color &_color)
{
  this->dataPtr->points.push_back(_pt);
  this->dataPtr->colors.push_back(_color);
  this->dataPtr->dirty = true;
}

/////////////////////////////////////////////////
void Ogre2DynamicLines::AddPoint(double _x, double _y,
                double _z, const ignition::math::Color &_color)
{
  this->AddPoint(ignition::math::Vector3d(_x, _y, _z), _color);
}

/////////////////////////////////////////////////
void Ogre2DynamicLines::SetPoint(unsigned int _index,
                            const ignition::math::Vector3d &_value)
{
  if (_index >= this->dataPtr->points.size())
  {
    ignerr << "Point index[" << _index << "] is out of bounds[0-"
           << this->dataPtr->points.size()-1 << "]\n";
    return;
  }

  this->dataPtr->points[_index] = _value;

  this->dataPtr->dirty = true;
}

/////////////////////////////////////////////////
void Ogre2DynamicLines::SetColor(unsigned int _index,
                            const ignition::math::Color &_color)
{
  this->dataPtr->colors[_index] = _color;
  this->dataPtr->dirty = true;
}

/////////////////////////////////////////////////
ignition::math::Vector3d Ogre2DynamicLines::Point(
    unsigned int _index) const
{
  if (_index >= this->dataPtr->points.size())
  {
    ignerr << "Point index[" << _index << "] is out of bounds[0-"
           << this->dataPtr->points.size()-1 << "]\n";

    return ignition::math::Vector3d(ignition::math::INF_D,
                                    ignition::math::INF_D,
                                    ignition::math::INF_D);
  }

  return this->dataPtr->points[_index];
}

/////////////////////////////////////////////////
unsigned int Ogre2DynamicLines::PointCount() const
{
  return this->dataPtr->points.size();
}

/////////////////////////////////////////////////
void Ogre2DynamicLines::Clear()
{
  this->dataPtr->points.clear();
  this->dataPtr->dirty = true;
}

/////////////////////////////////////////////////
void Ogre2DynamicLines::Update()
{
  if (this->dataPtr->dirty && this->dataPtr->points.size() > 1)
    this->FillHardwareBuffers();
}

/////////////////////////////////////////////////
void Ogre2DynamicLines::CreateVertexDeclaration()
{
  Ogre::v1::VertexDeclaration *decl =
    this->mRenderOp.vertexData->vertexDeclaration;

  decl->addElement(POSITION_BINDING, 0, Ogre::VET_FLOAT3, Ogre::VES_POSITION);
  decl->addElement(1, 0, Ogre::VET_COLOUR, Ogre::VES_DIFFUSE);
}

/////////////////////////////////////////////////
void Ogre2DynamicLines::FillHardwareBuffers()
{
  int size = this->dataPtr->points.size();
  this->PrepareHardwareBuffers(size, 0);

  if (!size)
  {
    this->mBox.setExtents(Ogre::Vector3::ZERO, Ogre::Vector3::ZERO);
    this->dataPtr->dirty = false;
  }

  Ogre::v1::HardwareVertexBufferSharedPtr vbuf =
    this->mRenderOp.vertexData->vertexBufferBinding->getBuffer(0);

  Ogre::Real *prPos =
    static_cast<Ogre::Real*>(vbuf->lock(Ogre::v1::HardwareBuffer::HBL_WRITE_ONLY));
  {
    for (int i = 0; i < size; ++i)
    {
      *prPos++ = this->dataPtr->points[i].X();
      *prPos++ = this->dataPtr->points[i].Y();
      *prPos++ = this->dataPtr->points[i].Z();

      this->mBox.merge(Ogre2Conversions::Convert(this->dataPtr->points[i]));
    }
  }
  vbuf->unlock();

  // Update the colors
  Ogre::v1::HardwareVertexBufferSharedPtr cbuf =
    this->mRenderOp.vertexData->vertexBufferBinding->getBuffer(1);

  Ogre::RGBA *colorArrayBuffer =
        static_cast<Ogre::RGBA *>(
                        cbuf->lock(Ogre::v1::HardwareBuffer::HBL_DISCARD));
  Ogre::RenderSystem *renderSystemForVertex =
        Ogre::Root::getSingleton().getRenderSystem();
  for (int i = 0; i < size; ++i)
  {
    Ogre::ColourValue color =
            Ogre2Conversions::Convert(this->dataPtr->colors[i]);
    renderSystemForVertex->convertColourValue(color, &colorArrayBuffer[i]);
  }
  cbuf->unlock();

  this->dataPtr->dirty = false;
}
