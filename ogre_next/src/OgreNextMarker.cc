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

#ifdef __APPLE__
  #define GL_SILENCE_DEPRECATION
  #include <OpenGL/gl.h>
  #include <OpenGL/glext.h>
#else
#ifndef _WIN32
  #include <GL/gl.h>
#endif
#endif

#include <gz/common/Console.hh>
#include <gz/common/Profiler.hh>

#include "gz/rendering/ogre_next/OgreNextCapsule.hh"
#include "gz/rendering/ogre_next/OgreNextConversions.hh"
#include "gz/rendering/ogre_next/OgreNextDynamicRenderable.hh"
#include "gz/rendering/ogre_next/OgreNextMarker.hh"
#include "gz/rendering/ogre_next/OgreNextMaterial.hh"
#include "gz/rendering/ogre_next/OgreNextMesh.hh"
#include "gz/rendering/ogre_next/OgreNextRenderEngine.hh"
#include "gz/rendering/ogre_next/OgreNextScene.hh"
#include "gz/rendering/ogre_next/OgreNextVisual.hh"

#ifdef _MSC_VER
  #pragma warning(push, 0)
#endif
#include <OgreItem.h>
#include <OgreMaterialManager.h>
#include <OgreRoot.h>
#include <OgreTechnique.h>
#ifdef _MSC_VER
  #pragma warning(pop)
#endif

class gz::rendering::OgreNextMarkerPrivate
{
  /// \brief Marker material
  public: OgreNextMaterialPtr material = nullptr;

  /// \brief Flag to indicate whether or not this mesh should be
  /// responsible for destroying the material
  public: bool ownsMaterial = false;

  /// \brief Geometry Object for primitive shapes
  public: OgreNextGeometryPtr geom{nullptr};

  /// \brief DynamicLines Object to display
  public: std::shared_ptr<OgreNextDynamicRenderable> dynamicRenderable;
};

using namespace gz;
using namespace rendering;

//////////////////////////////////////////////////
OgreNextMarker::OgreNextMarker()
  : dataPtr(new OgreNextMarkerPrivate)
{
}

//////////////////////////////////////////////////
OgreNextMarker::~OgreNextMarker()
{
  this->Destroy();
}

//////////////////////////////////////////////////
void OgreNextMarker::PreRender()
{
  GZ_PROFILE("Ogre2Marker::PreRender");
  if (this->markerType == MarkerType::MT_POINTS &&
      this->dataPtr->dynamicRenderable &&
      this->dataPtr->dynamicRenderable->PointCount() > 0u)
  {
    Ogre::Item *item = dynamic_cast<Ogre::Item *>(
        this->dataPtr->dynamicRenderable->OgreObject());
    if (!item->getSubItem(0)->getMaterial() ||
        item->getSubItem(0)->getMaterial()->getName() != "PointCloudPoint")
    {
      // enable GL_PROGRAM_POINT_SIZE so we can set gl_PointSize in vertex
      // shader
      auto engine = OgreNextRenderEngine::Instance();
      std::string renderSystemName =
          engine->OgreRoot()->getRenderSystem()->getFriendlyName();
      if (renderSystemName.find("OpenGL") != std::string::npos)
      {
      #ifdef __APPLE__
        glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
      #else
      #ifndef _WIN32
        glEnable(GL_PROGRAM_POINT_SIZE);
      #endif
      #endif
      }
      Ogre::MaterialPtr pointsMat =
          Ogre::MaterialManager::getSingleton().getByName(
          "PointCloudPoint");
      item->getSubItem(0)->setMaterial(pointsMat);
    }

    // point renderables use low level materials
    // get the material and set size uniform variable
    auto pass = item->getSubItem(0)->getMaterial()->getTechnique(0)->getPass(0);
    auto vertParams = pass->getVertexProgramParameters();
    vertParams->setNamedConstant("size", static_cast<Ogre::Real>(this->size));
  }

  this->dataPtr->dynamicRenderable->Update();
}

//////////////////////////////////////////////////
void OgreNextMarker::Destroy()
{
  BaseMarker::Destroy();

  if (this->dataPtr->geom)
  {
    this->dataPtr->geom->Destroy();
    this->dataPtr->geom.reset();
  }

  if (this->dataPtr->dynamicRenderable)
  {
    this->dataPtr->dynamicRenderable->Destroy();
    this->dataPtr->dynamicRenderable.reset();
  }

  if (this->dataPtr->material && this->dataPtr->ownsMaterial && this->Scene())
  {
    this->Scene()->DestroyMaterial(this->dataPtr->material);
    this->dataPtr->material.reset();
  }
}

//////////////////////////////////////////////////
Ogre::MovableObject *OgreNextMarker::OgreObject() const
{
  switch (this->markerType)
  {
    case MT_NONE:
      return nullptr;
    case MT_BOX:
    case MT_CAPSULE:
    case MT_CONE:
    case MT_CYLINDER:
    case MT_SPHERE:
    {
      if (nullptr != this->dataPtr->geom)
      {
        return this->dataPtr->geom->OgreObject();
      }
      return nullptr;
    }
    case MT_LINE_STRIP:
    case MT_LINE_LIST:
    case MT_POINTS:
    case MT_TRIANGLE_FAN:
    case MT_TRIANGLE_LIST:
    case MT_TRIANGLE_STRIP:
    {
      if (nullptr != this->dataPtr->dynamicRenderable)
      {
        return this->dataPtr->dynamicRenderable->OgreObject();
      }
      return nullptr;
    }
    default:
      gzerr << "Invalid Marker type " << this->markerType << "\n";
      return nullptr;
  }
}

//////////////////////////////////////////////////
void OgreNextMarker::Init()
{
  this->Create();
}

//////////////////////////////////////////////////
void OgreNextMarker::Create()
{
  this->markerType = MT_NONE;
  this->dataPtr->dynamicRenderable.reset(new OgreNextDynamicRenderable(
      this->scene));
  if (!this->dataPtr->geom)
  {
    this->dataPtr->geom =
      std::dynamic_pointer_cast<OgreNextGeometry>(this->scene->CreateBox());
  }
}

//////////////////////////////////////////////////
void OgreNextMarker::SetMaterial(MaterialPtr _material, bool _unique)
{
  if (nullptr == _material)
  {
    gzerr << "Cannot assign null material" << std::endl;
    return;
  }

  _material = (_unique) ? _material->Clone() : _material;

  OgreNextMaterialPtr derived =
    std::dynamic_pointer_cast<OgreNextMaterial>(_material);

  if (!derived)
  {
    gzerr << "Cannot assign material created by another render-engine"
      << std::endl;

    return;
  }

  derived->SetReceiveShadows(false);
  derived->SetCastShadows(false);
  derived->SetLightingEnabled(false);

  switch (this->markerType)
  {
    case MT_NONE:
      break;
    case MT_BOX:
    case MT_CAPSULE:
    case MT_CONE:
    case MT_CYLINDER:
    case MT_SPHERE:
    {
      if (nullptr != this->dataPtr->geom)
      {
        this->dataPtr->geom->SetMaterial(derived, false);
      }
      else
      {
        gzerr << "Failed to set material, null geometry." << std::endl;
      }
      break;
    }
    case MT_LINE_STRIP:
    case MT_LINE_LIST:
    case MT_POINTS:
    case MT_TRIANGLE_FAN:
    case MT_TRIANGLE_LIST:
    case MT_TRIANGLE_STRIP:
    {
      if (nullptr != this->dataPtr->dynamicRenderable)
      {
        this->dataPtr->dynamicRenderable->SetMaterial(derived, false);
      }
      else
      {
        gzerr << "Failed to set material, null renderable." << std::endl;
      }
      break;
    }
    default:
      gzerr << "Invalid Marker type " << this->markerType << "\n";
      break;
  }

  if (this->dataPtr->material && this->dataPtr->ownsMaterial)
    this->Scene()->DestroyMaterial(this->dataPtr->material);

  this->dataPtr->material = derived;
  this->dataPtr->ownsMaterial = _unique;
}

//////////////////////////////////////////////////
MaterialPtr OgreNextMarker::Material() const
{
  return this->dataPtr->material;
}

//////////////////////////////////////////////////
void OgreNextMarker::SetPoint(unsigned int _index,
    const math::Vector3d &_value)
{
  BaseMarker::SetPoint(_index, _value);
  this->dataPtr->dynamicRenderable->SetPoint(_index, _value);
}

//////////////////////////////////////////////////
void OgreNextMarker::AddPoint(const math::Vector3d &_pt,
    const math::Color &_color)
{
  BaseMarker::AddPoint(_pt, _color);
  this->dataPtr->dynamicRenderable->AddPoint(_pt, _color);
}

//////////////////////////////////////////////////
void OgreNextMarker::ClearPoints()
{
  BaseMarker::ClearPoints();
  this->dataPtr->dynamicRenderable->Clear();
}

//////////////////////////////////////////////////
void OgreNextMarker::SetType(MarkerType _markerType)
{
  if (_markerType == this->markerType)
    return;

  this->markerType = _markerType;

  auto visual = std::dynamic_pointer_cast<OgreNextVisual>(this->Parent());

  // clear geom if needed
  if (this->dataPtr->geom)
  {
    if (visual)
    {
      visual->RemoveGeometry(
          std::dynamic_pointer_cast<Geometry>(shared_from_this()));
    }
    this->dataPtr->geom->Destroy();
  }

  bool isGeom{false};
  GeometryPtr newGeom;
  switch (_markerType)
  {
    case MT_NONE:
      break;
    case MT_BOX:
      isGeom = true;
      newGeom = this->scene->CreateBox();
      break;
    case MT_CAPSULE:
      isGeom = true;
      newGeom = this->scene->CreateCapsule();
      break;
    case MT_CONE:
      isGeom = true;
      newGeom = this->scene->CreateCone();
      break;
    case MT_CYLINDER:
      isGeom = true;
      newGeom = this->scene->CreateCylinder();
      break;
    case MT_SPHERE:
      isGeom = true;
      newGeom = this->scene->CreateSphere();
      break;
    case MT_LINE_STRIP:
    case MT_LINE_LIST:
    case MT_POINTS:
    case MT_TRIANGLE_FAN:
    case MT_TRIANGLE_LIST:
    case MT_TRIANGLE_STRIP:
      this->dataPtr->dynamicRenderable->SetOperationType(_markerType);
      break;
    default:
      gzerr << "Invalid Marker type [" << _markerType << "]" << std::endl;
      break;
  }

  if (nullptr != newGeom)
  {
    this->dataPtr->geom = std::dynamic_pointer_cast<OgreNextGeometry>(newGeom);
    if (nullptr == this->dataPtr->geom)
    {
      gzerr << "Failed to cast to [OgreNextGeom], type [" << _markerType << "]"
             << std::endl;
    }
    else if (visual)
    {
      visual->AddGeometry(
          std::dynamic_pointer_cast<Geometry>(shared_from_this()));
    }
  }
  else if (isGeom)
  {
    gzerr << "Failed to create geometry for marker type [" << _markerType
           << "]" << std::endl;
  }
}

//////////////////////////////////////////////////
MarkerType OgreNextMarker::Type() const
{
  return this->markerType;
}
