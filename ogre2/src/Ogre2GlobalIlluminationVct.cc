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

#include "gz/rendering/ogre2/Ogre2GlobalIlluminationVct.hh"

#include "gz/rendering/ogre2/Ogre2RenderEngine.hh"
#include "gz/rendering/ogre2/Ogre2Scene.hh"

#ifdef _MSC_VER
#  pragma warning(push, 0)
#endif
#include <Hlms/Pbs/OgreHlmsPbs.h>
#include <Hlms/Pbs/Vct/OgreVctLighting.h>
#include <Hlms/Pbs/Vct/OgreVctVoxelizer.h>
#include <OgreHlmsManager.h>
#include <OgreItem.h>
#include <OgreRoot.h>
#ifdef _MSC_VER
#  pragma warning(pop)
#endif

using namespace gz;
using namespace rendering;

/// \brief Private data for the Ogre2GlobalIlluminationVct class
class gz::rendering::Ogre2GlobalIlluminationVctPrivate
{
  // clang-format off
  /// \brief Ogre's Voxelizer. In charge of voxelizing the scene
  /// at voxelizing phase time
  public: Ogre::VctVoxelizer *voxelizer = nullptr;

  /// \brief Takes VctVoxelizer and creates the lighting structures
  /// which are later used for GI in real time during render.
  public: Ogre::VctLighting *vctLighting = nullptr;

  /// \brief See GlobalIlluminationVct::SetResolution
  public: uint32_t resolution[3]{16u, 16u, 16u};

  /// \brief See GlobalIlluminationVct::SetOctantCount
  public: uint32_t octants[3]{1u, 1u, 1u};

  /// \brief See GlobalIlluminationVct::SetBounceCount
  public: uint32_t bounceCount = 6u;

  /// \brief See GlobalIlluminationVct::SetThinWallCounter
  public: float thinWallCounter = 1.0f;

  /// \brief See GlobalIlluminationVct::SetHighQuality
  /// This value is cached because it globally affects HlmsPbs
  public: bool highQuality = false;

  /// \brief See GlobalIlluminationVct::SetConserveMemory
  public: bool conserveMemory = false;

  /// \brief See GlobalIlluminationVct::SetAnisotropic
  public: bool anisotropic = true;
  // clang-format on
};

//////////////////////////////////////////////////
Ogre2GlobalIlluminationVct::Ogre2GlobalIlluminationVct() :
  dataPtr(new Ogre2GlobalIlluminationVctPrivate)
{
}

//////////////////////////////////////////////////
Ogre2GlobalIlluminationVct::~Ogre2GlobalIlluminationVct()
{
  this->Destroy();
}

//////////////////////////////////////////////////
void Ogre2GlobalIlluminationVct::Init()
{
  Ogre2Object::Init();

  Ogre::Root *ogreRoot = Ogre2RenderEngine::Instance()->OgreRoot();
  this->dataPtr->voxelizer = new Ogre::VctVoxelizer(
    Ogre::Id::generateNewId<Ogre::VctVoxelizer>(), ogreRoot->getRenderSystem(),
    ogreRoot->getHlmsManager(), true);
  this->dataPtr->voxelizer->setResolution(this->dataPtr->resolution[0],
                                          this->dataPtr->resolution[1],
                                          this->dataPtr->resolution[2]);
}

//////////////////////////////////////////////////
void Ogre2GlobalIlluminationVct::Destroy()
{
  Ogre2Object::Destroy();

  if (this->Enabled())
    this->SetEnabled(false);

  delete this->dataPtr->vctLighting;
  this->dataPtr->vctLighting = nullptr;

  delete this->dataPtr->voxelizer;
  this->dataPtr->voxelizer = nullptr;
}

//////////////////////////////////////////////////
void Ogre2GlobalIlluminationVct::SetResolution(const uint32_t _resolution[3])
{
  for (int i = 0; i < 3; ++i)
  {
    this->dataPtr->resolution[i] = _resolution[i];
  }
  this->dataPtr->voxelizer->setResolution(_resolution[0], _resolution[1],
                                          _resolution[2]);
}

//////////////////////////////////////////////////
const uint32_t *Ogre2GlobalIlluminationVct::Resolution() const
{
  return this->dataPtr->resolution;
}

//////////////////////////////////////////////////
void Ogre2GlobalIlluminationVct::SetOctantCount(const uint32_t _octants[3])
{
  for (int i = 0; i < 3; ++i)
  {
    this->dataPtr->octants[i] = _octants[i];
  }
}

//////////////////////////////////////////////////
const uint32_t *Ogre2GlobalIlluminationVct::OctantCount() const
{
  return this->dataPtr->octants;
}

//////////////////////////////////////////////////
void Ogre2GlobalIlluminationVct::SetBounceCount(uint32_t _bounceCount)
{
  this->dataPtr->bounceCount = _bounceCount;
  if (_bounceCount == 0u && this->dataPtr->vctLighting)
    this->dataPtr->vctLighting->setAllowMultipleBounces(false);
}

//////////////////////////////////////////////////
uint32_t Ogre2GlobalIlluminationVct::BounceCount() const
{
  return this->dataPtr->bounceCount;
}

//////////////////////////////////////////////////
void Ogre2GlobalIlluminationVct::SetHighQuality(bool _highQuality)
{
  if (this->Enabled())
  {
    Ogre::HlmsPbs *hlmsPbs = this->HlmsPbs();
    hlmsPbs->setVctFullConeCount(_highQuality);
  }
  this->dataPtr->highQuality = _highQuality;
}

//////////////////////////////////////////////////
bool Ogre2GlobalIlluminationVct::HighQuality() const
{
  return this->dataPtr->highQuality;
}

//////////////////////////////////////////////////
void Ogre2GlobalIlluminationVct::SetAnisotropic(bool _anisotropic)
{
  this->dataPtr->anisotropic = _anisotropic;
  if (this->dataPtr->vctLighting)
    this->dataPtr->vctLighting->setAnisotropic(_anisotropic);
}

//////////////////////////////////////////////////
bool Ogre2GlobalIlluminationVct::Anisotropic() const
{
  return this->dataPtr->anisotropic;
}

//////////////////////////////////////////////////
void Ogre2GlobalIlluminationVct::SetThinWallCounter(float _thinWallCounter)
{
  this->dataPtr->thinWallCounter = _thinWallCounter;
}

//////////////////////////////////////////////////
float Ogre2GlobalIlluminationVct::ThinWallCounter() const
{
  return this->dataPtr->thinWallCounter;
}

//////////////////////////////////////////////////
void Ogre2GlobalIlluminationVct::SetConserveMemory(bool _conserveMemory)
{
  this->dataPtr->conserveMemory = _conserveMemory;
  if (this->dataPtr->conserveMemory && this->dataPtr->vctLighting)
    this->dataPtr->vctLighting->setAllowMultipleBounces(false);
}

//////////////////////////////////////////////////
bool Ogre2GlobalIlluminationVct::ConserveMemory() const
{
  return this->dataPtr->conserveMemory;
}

//////////////////////////////////////////////////
void Ogre2GlobalIlluminationVct::Build()
{
  Ogre::SceneManager *sceneManager = this->scene->OgreSceneManager();
  sceneManager->updateSceneGraph();

  Ogre::VctVoxelizer *voxelizer = this->dataPtr->voxelizer;

  voxelizer->removeAllItems();

  // Add all static Item from Ogre
  Ogre::ObjectMemoryManager &objMemoryManager =
    sceneManager->_getEntityMemoryManager(Ogre::SCENE_STATIC);

  const size_t numRenderQueues = objMemoryManager.getNumRenderQueues();

  for (size_t i = 0u; i < numRenderQueues; ++i)
  {
    Ogre::ObjectData objData;
    const size_t totalObjs = objMemoryManager.getFirstObjectData(objData, i);

    for (size_t j = 0; j < totalObjs; j += ARRAY_PACKED_REALS)
    {
      // objData.mOwner is guaranteed by Ogre to not be a nullptr
      if (objData.mOwner[j]->getVisible())
      {
        auto item = dynamic_cast<Ogre::Item *>(objData.mOwner[j]);
        if (item)
        {
          voxelizer->addItem(item, false);
        }
      }

      objData.advancePack();
    }
  }

  voxelizer->autoCalculateRegion();
  voxelizer->dividideOctants(this->dataPtr->octants[0],
                             this->dataPtr->octants[1],
                             this->dataPtr->octants[2]);

  voxelizer->build(sceneManager);

  if (this->dataPtr->vctLighting == nullptr)
  {
    // Create Ogre::VctLighting
    this->dataPtr->vctLighting =
      new Ogre::VctLighting(Ogre::Id::generateNewId<Ogre::VctLighting>(),
                            this->dataPtr->voxelizer, true);

    this->dataPtr->vctLighting->setAnisotropic(this->dataPtr->anisotropic);
  }

  this->LightingChanged();
}

//////////////////////////////////////////////////
void Ogre2GlobalIlluminationVct::SetEnabled(bool _enabled)
{
  Ogre::HlmsPbs *hlmsPbs = this->HlmsPbs();
  if (_enabled)
  {
    GZ_ASSERT(hlmsPbs->getVctLighting() == nullptr ||
                 hlmsPbs->getVctLighting() == this->dataPtr->vctLighting,
               "There's already an active GI solution!");
    if (!this->dataPtr->vctLighting)
    {
      Build();
    }
    hlmsPbs->setVctLighting(this->dataPtr->vctLighting);
    hlmsPbs->setVctFullConeCount(this->dataPtr->highQuality);
  }
  else
  {
    GZ_ASSERT(hlmsPbs->getVctLighting() == nullptr ||
                hlmsPbs->getVctLighting() == this->dataPtr->vctLighting,
              "There's an active GI solution but isn't us!");
    hlmsPbs->setVctLighting(nullptr);
  }
}

//////////////////////////////////////////////////
bool Ogre2GlobalIlluminationVct::Enabled() const
{
  if (this->dataPtr->vctLighting == nullptr)
    return false;

  Ogre::HlmsPbs *hlmsPbs = this->HlmsPbs();
  return hlmsPbs->getVctLighting() == this->dataPtr->vctLighting;
}

//////////////////////////////////////////////////
void Ogre2GlobalIlluminationVct::LightingChanged()
{
  GZ_ASSERT(this->dataPtr->vctLighting != nullptr,
            "Incorrect usage detected. Did you call Build?");

  this->dataPtr->vctLighting->setAllowMultipleBounces(
    this->dataPtr->bounceCount > 0u);

  this->dataPtr->vctLighting->update(this->scene->OgreSceneManager(),
                                     this->dataPtr->bounceCount,
                                     this->dataPtr->thinWallCounter);
  if (this->dataPtr->conserveMemory)
  {
    this->dataPtr->vctLighting->setAllowMultipleBounces(false);
  }
}

//////////////////////////////////////////////////
Ogre::HlmsPbs *Ogre2GlobalIlluminationVct::HlmsPbs() const
{
  Ogre::Root *ogreRoot = Ogre2RenderEngine::Instance()->OgreRoot();
  Ogre::HlmsManager *hlmsManager = ogreRoot->getHlmsManager();

  Ogre::Hlms *hlms = hlmsManager->getHlms(Ogre::HLMS_PBS);
  GZ_ASSERT(dynamic_cast<Ogre::HlmsPbs *>(hlms),
            "Corruption or incorrect setup detected");
  return static_cast<Ogre::HlmsPbs *>(hlms);
}
