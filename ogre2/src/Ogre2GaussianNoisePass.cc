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


#include <ignition/common/Console.hh>

#include "ignition/rendering/RenderPassSystem.hh"
#include "ignition/rendering/ogre2/Ogre2Includes.hh"
#include "ignition/rendering/ogre2/Ogre2GaussianNoisePass.hh"
#include "ignition/rendering/ogre2/Ogre2RenderEngine.hh"

using namespace ignition;
using namespace rendering;

//////////////////////////////////////////////////
Ogre2GaussianNoisePass::Ogre2GaussianNoisePass()
{
}

//////////////////////////////////////////////////
Ogre2GaussianNoisePass::~Ogre2GaussianNoisePass()
{
}

//////////////////////////////////////////////////
void Ogre2GaussianNoisePass::PreRender()
{
  if (!this->gaussianNoiseMat)
    return;

  if (!this->enabled)
    return;

  // modify material here (wont alter the base material!), called for
  // every drawn geometry instance (i.e. compositor render_quad)

  // Sample three values within the range [0,1.0] and set them for use in
  // the fragment shader, which will interpret them as offsets from (0,0)
  // to use when computing pseudo-random values.
  Ogre::Vector3 offsets(ignition::math::Rand::DblUniform(0.0, 1.0),
                        ignition::math::Rand::DblUniform(0.0, 1.0),
                        ignition::math::Rand::DblUniform(0.0, 1.0));
  // These calls are setting parameters that are declared in two places:
  // 1. media/materials/scripts/gaussian_noise.material, in
  //    fragment_program GaussianNoiseFS
  // 2. media/materials/scripts/gaussian_noise_fs.glsl
  Ogre::Pass *pass = this->gaussianNoiseMat->getTechnique(0)->getPass(0);
  Ogre::GpuProgramParametersSharedPtr psParams =
      pass->getFragmentProgramParameters();
  psParams->setNamedConstant("offsets", offsets);
  psParams->setNamedConstant("mean", static_cast<Ogre::Real>(this->mean));
  psParams->setNamedConstant("stddev",
      static_cast<Ogre::Real>(this->stdDev));
}

//////////////////////////////////////////////////
void Ogre2GaussianNoisePass::CreateRenderPass()
{
  // verify the Gaussian noise material and compositor node exist
  // since we're not creating anything here (just checking if resources exist),
  // we don't need to destroy anything.

  // The GaussianNoise material is defined in script (gaussian_noise.material).
  // We do not need to clone it since the uniform variables are modified in
  // every frame. The changes made by other render pass are overriden locally
  // by this render passes, and vice versa.
  std::string matName = "GaussianNoise";
  Ogre::MaterialPtr ogreMat =
      Ogre::MaterialManager::getSingleton().getByName(matName);
  if (!ogreMat)
  {
    ignerr << "Gaussian noise material not found: '" << matName << "'"
           << std::endl;
    return;
  }
  this->gaussianNoiseMat = ogreMat.get();
  if (!this->gaussianNoiseMat->isLoaded())
    this->gaussianNoiseMat->load();

  // check the compositor node exists
  auto engine = Ogre2RenderEngine::Instance();
  auto ogreRoot = engine->OgreRoot();
  Ogre::CompositorManager2 *ogreCompMgr = ogreRoot->getCompositorManager2();

  // GaussianNoiseNode is defined in GaussianNoise.compositor
  this->ogreCompositorNodeDefName = "GaussianNoiseNode";
  if (!ogreCompMgr->hasNodeDefinition(this->ogreCompositorNodeDefName))
  {
    ignerr << "Gaussian noise compositor node not found: '"
           << this->ogreCompositorNodeDefName << "'"  << std::endl;
  }
}

IGN_RENDERING_REGISTER_RENDER_PASS(Ogre2GaussianNoisePass, GaussianNoisePass)
