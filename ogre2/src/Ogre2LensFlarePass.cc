/*
 * Copyright (C) 2022 Open Source Robotics Foundation
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
#include "gz/rendering/ogre2/Ogre2LensFlarePass.hh"

#include <gz/common/Util.hh>

#include "gz/rendering/RenderPassSystem.hh"
#include "gz/rendering/ogre2/Ogre2Conversions.hh"
#include "gz/rendering/ogre2/Ogre2Light.hh"

#ifdef _MSC_VER
#  pragma warning(push, 0)
#endif
#include <Compositor/OgreCompositorWorkspace.h>
#include <Compositor/OgreCompositorWorkspaceListener.h>
#include <Compositor/Pass/OgreCompositorPass.h>
#include <Compositor/Pass/PassQuad/OgreCompositorPassQuad.h>
#include <OgreCamera.h>
#include <OgreGpuProgram.h>
#include <OgrePass.h>
#include <OgreTechnique.h>
#include <OgreVector3.h>
#ifdef _MSC_VER
#  pragma warning(pop)
#endif

// clang-format off
namespace gz
{
namespace rendering
{
inline namespace GZ_RENDERING_VERSION_NAMESPACE {
/// \brief Helper class for setting up Camera and Materials when rendering
/// via Ogre2LensFlarePass.
class GZ_RENDERING_OGRE2_HIDDEN Ogre2LensFlarePassWorkspaceListenerPrivate
    final : public Ogre::CompositorWorkspaceListener
{
  public: gz::rendering::Ogre2LensFlarePass &owner;

  /// \brief constructor
  /// \param[in] _scene the scene manager responsible for rendering
  public: explicit Ogre2LensFlarePassWorkspaceListenerPrivate(
        gz::rendering::Ogre2LensFlarePass &_owner) :
    owner(_owner)
  {
  }

  /// \brief Called when each pass is about to be executed.
  /// \param[in] _pass Ogre pass which is about to execute
  public: void passPreExecute(Ogre::CompositorPass *_pass) override;
};
}
}
}
// clang-format on

/// \brief Private data for the Ogre2LensFlarePass class
class gz::rendering::Ogre2LensFlarePass::Implementation
{
  // clang-format off

  /// \brief Position of light in world frame
  public: math::Vector3d lightWorldPos;

  /// \brief Color of lens flare.
  public: math::Vector3d color = math::Vector3d(1.0, 1.0, 1.0);

  /// \brief See Ogre2LensFlarePassWorkspaceListenerPrivate
  public: Ogre2LensFlarePassWorkspaceListenerPrivate workspaceListener;

  public: explicit Implementation(gz::rendering::Ogre2LensFlarePass &_owner) :
    workspaceListener(_owner)
  {
  }
  // clang-format on
};

using namespace gz;
using namespace rendering;

// Arbitrary values, but they must be in sync with the Compositor script file
static constexpr uint32_t kLensFlareNodePassQuadId = 98744413u;

//////////////////////////////////////////////////
Ogre2LensFlarePass::Ogre2LensFlarePass() :
  dataPtr(utils::MakeUniqueImpl<Implementation>(*this))
{
  this->ogreCompositorNodeDefName = "LensFlareNode";
}

//////////////////////////////////////////////////
Ogre2LensFlarePass::~Ogre2LensFlarePass()
{
}

//////////////////////////////////////////////////
void Ogre2LensFlarePass::PreRender()
{
  if (!this->enabled || this->light == nullptr)
    return;

  // use light's world position for lens flare position
  DirectionalLight *dirLight =
    dynamic_cast<DirectionalLight *>(this->light.get());
  if (dirLight)
  {
    // Directional lights misuse position as a direction.
    // The large multiplier is for occlusion testing and assumes the light
    // is very far away. Larger values cause the light to disappear on
    // some frames for some unknown reason.
    this->dataPtr->lightWorldPos =
      -(this->light->WorldPose().Rot() * dirLight->Direction()) * 100000.0;
  }
  else
    this->dataPtr->lightWorldPos = this->light->WorldPose().Pos();
}

//////////////////////////////////////////////////
void Ogre2LensFlarePass::WorkspaceAdded(Ogre::CompositorWorkspace *_workspace)
{
  _workspace->addListener(&this->dataPtr->workspaceListener);
}

// Documentation inherited
void Ogre2LensFlarePass::WorkspaceRemoved(Ogre::CompositorWorkspace *_workspace)
{
  _workspace->removeListener(&this->dataPtr->workspaceListener);
}

//////////////////////////////////////////////////

void Ogre2LensFlarePassWorkspaceListenerPrivate::passPreExecute(
  Ogre::CompositorPass *_pass)
{
  if (!this->owner.enabled)
    return;

  const uint32_t identifier = _pass->getDefinition()->mIdentifier;
  if (identifier != kLensFlareNodePassQuadId)
    return;

  using namespace Ogre;

  GZ_ASSERT(dynamic_cast<CompositorPassQuad *>(_pass),
            "Impossible! Corrupted memory? lens_flare.compositor out of sync?");

  CompositorPassQuad *passQuad = static_cast<CompositorPassQuad *>(_pass);

  Pass *pass = passQuad->getPass();

  GpuProgramParametersSharedPtr psParams = pass->getFragmentProgramParameters();

  const Ogre::Camera *camera = passQuad->getCamera();

  // project 3d world space to clip space
  const Matrix4 viewProj =
    camera->getProjectionMatrix() * camera->getViewMatrix();
  const Vector4 pos =
    viewProj *
    Vector4(Ogre2Conversions::Convert(this->owner.dataPtr->lightWorldPos));

  // normalize x and y, keep z for visibility test
  Vector3 lightPos;
  lightPos.x = pos.x / pos.w;
  lightPos.y = pos.y / pos.w;
  // Make lightPos.z > 0 mean we're in front of near plane
  // since pos.z is in range [-|pos.w|; |pos.w|]
  lightPos.z = pos.z + std::abs(pos.w);

  double lensFlareScale = 1.0;

  psParams->setNamedConstant("vpAspectRatio", camera->getAspectRatio());
  psParams->setNamedConstant("lightPos", lightPos);
  psParams->setNamedConstant("scale", static_cast<Real>(lensFlareScale));
  psParams->setNamedConstant(
    "color", Ogre2Conversions::Convert(this->owner.dataPtr->color));
}

GZ_RENDERING_REGISTER_RENDER_PASS(Ogre2LensFlarePass, LensFlarePass)
