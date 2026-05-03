/*
 * Copyright (C) 2018 Open Source Robotics Foundation
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

#include <gz/common/Profiler.hh>

#include "gz/rendering/ogre_next/OgreNextCamera.hh"
#include "gz/rendering/ogre_next/OgreNextConversions.hh"
#include "gz/rendering/ogre_next/OgreNextRenderTarget.hh"
#include "gz/rendering/ogre_next/OgreNextScene.hh"
#include "gz/rendering/ogre_next/OgreNextSelectionBuffer.hh"
#include "gz/rendering/Utils.hh"

#ifdef _MSC_VER
  #pragma warning(push, 0)
#endif
#include <OgreCamera.h>
#include <OgreItem.h>
#include <OgreSceneManager.h>
#ifdef _MSC_VER
  #pragma warning(pop)
#endif

/// \brief Private data for the OgreNextCamera class
class gz::rendering::OgreNextCameraPrivate
{
};

using namespace gz;
using namespace rendering;

//////////////////////////////////////////////////
OgreNextCamera::OgreNextCamera()
  : dataPtr(std::make_unique<OgreNextCameraPrivate>())
{
}

//////////////////////////////////////////////////
OgreNextCamera::~OgreNextCamera()
{
  this->Destroy();
}

//////////////////////////////////////////////////
void OgreNextCamera::Destroy()
{
  if (!this->ogreCamera || !this->Scene()->IsInitialized())
    return;

  this->RemoveAllRenderPasses();
  this->DestroyRenderTexture();

  Ogre::SceneManager *ogreSceneManager = this->scene->OgreSceneManager();
  if (ogreSceneManager == nullptr)
  {
    gzerr << "Scene manager cannot be obtained" << std::endl;
  }
  else
  {
    if (ogreSceneManager->findCameraNoThrow(this->name) != nullptr)
    {
      ogreSceneManager->destroyCamera(this->ogreCamera);
      this->ogreCamera = nullptr;
    }
  }

  if (this->selectionBuffer)
  {
    delete this->selectionBuffer;
    this->selectionBuffer = nullptr;
  }

  BaseCamera::Destroy();
}

//////////////////////////////////////////////////
math::Angle OgreNextCamera::HFOV() const
{
  return BaseCamera::HFOV();
}

//////////////////////////////////////////////////
void OgreNextCamera::SetHFOV(const math::Angle &_angle)
{
  BaseCamera::SetHFOV(_angle);
  this->SyncOgreCameraAspectRatio();
}

//////////////////////////////////////////////////
double OgreNextCamera::AspectRatio() const
{
  return BaseCamera::AspectRatio();
}

//////////////////////////////////////////////////
void OgreNextCamera::SetAspectRatio(const double _ratio)
{
  BaseCamera::SetAspectRatio(_ratio);
  this->SyncOgreCameraAspectRatio();
}

//////////////////////////////////////////////////
unsigned int OgreNextCamera::AntiAliasing() const
{
  return this->renderTexture->AntiAliasing();
}

//////////////////////////////////////////////////
void OgreNextCamera::SetAntiAliasing(const unsigned int _aa)
{
  BaseCamera::SetAntiAliasing(_aa);
  this->renderTexture->SetAntiAliasing(_aa);
}

//////////////////////////////////////////////////
math::Color OgreNextCamera::BackgroundColor() const
{
  return this->renderTexture->BackgroundColor();
}

//////////////////////////////////////////////////
void OgreNextCamera::SetBackgroundColor(const math::Color &_color)
{
  this->renderTexture->SetBackgroundColor(_color);
}

//////////////////////////////////////////////////
MaterialPtr OgreNextCamera::BackgroundMaterial() const
{
  return this->renderTexture->BackgroundMaterial();
}

//////////////////////////////////////////////////
void OgreNextCamera::SetBackgroundMaterial(MaterialPtr _material)
{
  this->renderTexture->SetBackgroundMaterial(_material);
}

//////////////////////////////////////////////////
void OgreNextCamera::SetMaterial(const MaterialPtr &_material)
{
  this->renderTexture->SetMaterial(_material);
}

//////////////////////////////////////////////////
void OgreNextCamera::Render()
{
  GZ_PROFILE("Ogre2Camera::Render");
  this->renderTexture->Render();
}

//////////////////////////////////////////////////
RenderTargetPtr OgreNextCamera::RenderTarget() const
{
  return this->renderTexture;
}

//////////////////////////////////////////////////
void OgreNextCamera::Init()
{
  BaseCamera::Init();
  this->CreateCamera();
  this->CreateRenderTexture();
  this->Reset();
}

//////////////////////////////////////////////////
void OgreNextCamera::SyncOgreCameraAspectRatio()
{
  const double aspectRatio = this->AspectRatio();
  const double angle = this->HFOV().Radian();
  const double vfov = 2.0 * atan(tan(angle / 2.0) / aspectRatio);
  this->ogreCamera->setFOVy(Ogre::Radian((Ogre::Real)vfov));
  this->ogreCamera->setAspectRatio((Ogre::Real)aspectRatio);
}

//////////////////////////////////////////////////
void OgreNextCamera::CreateCamera()
{
  // create ogre camera object
  Ogre::SceneManager *ogreSceneManager;
  ogreSceneManager = this->scene->OgreSceneManager();
  this->ogreCamera = ogreSceneManager->createCamera(this->name);
  // by default, ogre_next cameras are attached to root scene node
  this->ogreCamera->detachFromParent();
  this->ogreNode->attachObject(this->ogreCamera);

  // rotate to Gazebo coordinate system
  this->ogreCamera->yaw(Ogre::Degree(-90.0));
  this->ogreCamera->roll(Ogre::Degree(-90.0));
  this->ogreCamera->setFixedYawAxis(false);

  // TODO(anyone): provide api access
  this->ogreCamera->setProjectionType(Ogre::PT_PERSPECTIVE);
  this->ogreCamera->setCustomProjectionMatrix(false);
}

//////////////////////////////////////////////////
void OgreNextCamera::CreateRenderTexture()
{
  this->DestroyRenderTexture();
  RenderTexturePtr base = this->scene->CreateRenderTexture();
  this->renderTexture = std::dynamic_pointer_cast<OgreNextRenderTexture>(base);
  this->renderTexture->SetCamera(this->ogreCamera);
  this->renderTexture->SetFormat(PF_R8G8B8);
  this->renderTexture->SetWidth(this->ImageWidth());
  this->renderTexture->SetHeight(this->ImageHeight());
  this->renderTexture->SetBackgroundColor(this->scene->BackgroundColor());
  this->renderTexture->SetVisibilityMask(this->visibilityMask);
}

//////////////////////////////////////////////////
void OgreNextCamera::DestroyRenderTexture()
{
  if (this->renderTexture)
  {
    dynamic_cast<OgreNextRenderTarget *>(this->renderTexture.get())->Destroy();
    this->renderTexture.reset();
  }
}

//////////////////////////////////////////////////
unsigned int OgreNextCamera::RenderTextureGLId() const
{
  if (!this->renderTexture)
    return 0u;

  OgreNextRenderTexturePtr rt =
      std::dynamic_pointer_cast<OgreNextRenderTexture>(this->renderTexture);

  if (!rt)
    return 0u;

  return rt->GLId();
}

//////////////////////////////////////////////////
void OgreNextCamera::RenderTextureMetalId(void *_textureIdPtr) const
{
  if (!this->renderTexture)
    return;

  OgreNextRenderTexturePtr rt =
      std::dynamic_pointer_cast<OgreNextRenderTexture>(this->renderTexture);

  if (!rt)
    return;

  rt->MetalId(_textureIdPtr);
}

//////////////////////////////////////////////////
void OgreNextCamera::PrepareForExternalSampling()
{
  if (!this->renderTexture)
    return;
  this->renderTexture->PrepareForExternalSampling();
}

//////////////////////////////////////////////////
void OgreNextCamera::SetShadowsDirty()
{
  this->SetShadowsNodeDefDirty();
}

//////////////////////////////////////////////////
void OgreNextCamera::SetShadowsNodeDefDirty()
{
  if (!this->renderTexture)
    return;

  OgreNextRenderTexturePtr rt =
      std::dynamic_pointer_cast<OgreNextRenderTexture>(this->renderTexture);

  if (rt)
    rt->SetShadowsNodeDefDirty();
}

//////////////////////////////////////////////////
void OgreNextCamera::SetSelectionBuffer()
{
  this->selectionBuffer = new OgreNextSelectionBuffer(this->name, this->scene,
    this->ImageWidth(), this->ImageHeight());
}

//////////////////////////////////////////////////
OgreNextSelectionBuffer *OgreNextCamera::SelectionBuffer() const
{
  return this->selectionBuffer;
}

//////////////////////////////////////////////////
VisualPtr OgreNextCamera::VisualAt(const math::Vector2i &_mousePos)
{
  GZ_PROFILE("Ogre2Camera::VisualAt");
  VisualPtr result;

  if (!this->selectionBuffer)
  {
    this->SetSelectionBuffer();

    if (!this->selectionBuffer)
    {
      return result;
    }
  }
  else
  {
    this->selectionBuffer->SetDimensions(
      this->ImageWidth(), this->ImageHeight());
  }

  float ratio = screenScalingFactor();
  math::Vector2i mousePos(
      static_cast<int>(std::rint(ratio * _mousePos.X())),
      static_cast<int>(std::rint(ratio * _mousePos.Y())));
  Ogre::MovableObject *ogreObj = this->selectionBuffer->OnSelectionClick(
      mousePos.X(), mousePos.Y());

  if (ogreObj)
  {
    if (!ogreObj->getUserObjectBindings().getUserAny().isEmpty() &&
        ogreObj->getUserObjectBindings().getUserAny().getType() ==
        typeid(unsigned int))
    {
      try
      {
        result = this->scene->VisualById(Ogre::any_cast<unsigned int>(
              ogreObj->getUserObjectBindings().getUserAny()));
      }
      catch(Ogre::Exception &e)
      {
        gzerr << "Ogre Error:" << e.getFullDescription() << "\n";
      }
    }
  }

  return result;
}

//////////////////////////////////////////////////
RenderWindowPtr OgreNextCamera::CreateRenderWindow()
{
  RenderWindowPtr base = this->scene->CreateRenderWindow();
  OgreNextRenderWindowPtr renderWindow =
      std::dynamic_pointer_cast<OgreNextRenderWindow>(base);
  renderWindow->SetWidth(this->ImageWidth());
  renderWindow->SetHeight(this->ImageHeight());
  renderWindow->SetDevicePixelRatio(1);
  renderWindow->SetCamera(this->ogreCamera);
  renderWindow->SetBackgroundColor(this->scene->BackgroundColor());

  this->renderTexture = renderWindow;
  return base;
}

//////////////////////////////////////////////////
math::Matrix4d OgreNextCamera::ProjectionMatrix() const
{
  return OgreNextConversions::Convert(this->ogreCamera->getProjectionMatrix());
}

//////////////////////////////////////////////////
math::Matrix4d OgreNextCamera::ViewMatrix() const
{
  return OgreNextConversions::Convert(this->ogreCamera->getViewMatrix(true));
}

//////////////////////////////////////////////////
void OgreNextCamera::SetProjectionMatrix(const math::Matrix4d &_matrix)
{
  BaseCamera::SetProjectionMatrix(_matrix);
  this->ogreCamera->setCustomProjectionMatrix(true,
      OgreNextConversions::Convert(this->projectionMatrix));
}

//////////////////////////////////////////////////
void OgreNextCamera::SetProjectionType(CameraProjectionType _type)
{
  BaseCamera::SetProjectionType(_type);
  switch (this->projectionType)
  {
    default:
    case CPT_PERSPECTIVE:
      this->ogreCamera->setProjectionType(Ogre::PT_PERSPECTIVE);
      break;
    case CPT_ORTHOGRAPHIC:
      this->ogreCamera->setProjectionType(Ogre::PT_ORTHOGRAPHIC);
      break;
  }
  // reset projection matrix when projection type changes
  this->ogreCamera->setCustomProjectionMatrix(false);
}

//////////////////////////////////////////////////
void OgreNextCamera::SetNearClipPlane(const double _near)
{
  BaseCamera::SetNearClipPlane(_near);
  this->ogreCamera->setNearClipDistance(this->nearClip);
}

//////////////////////////////////////////////////
void OgreNextCamera::SetFarClipPlane(const double _far)
{
  BaseCamera::SetFarClipPlane(_far);
  this->ogreCamera->setFarClipDistance(this->farClip);
}

//////////////////////////////////////////////////
void OgreNextCamera::SetVisibilityMask(uint32_t _mask)
{
  if (_mask & ~Ogre::VisibilityFlags::RESERVED_VISIBILITY_FLAGS)
  {
    gzwarn << "OgreNextCamera::SetVisibilityMask: Mask bits " << std::hex
           << ~Ogre::VisibilityFlags::RESERVED_VISIBILITY_FLAGS << std::dec
           << " are set but will be ignored as they conflict with the "
           << "reserved bits used internally by the ogre_next backend."
           << std::endl;
  }
  BaseSensor::SetVisibilityMask(_mask);
  if (this->renderTexture)
    this->renderTexture->SetVisibilityMask(_mask);
}

//////////////////////////////////////////////////
Ogre::Camera *OgreNextCamera::OgreCamera() const
{
  return this->ogreCamera;
}
