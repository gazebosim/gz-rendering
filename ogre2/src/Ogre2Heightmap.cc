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

#include <chrono>

#include <ignition/common/Console.hh>
#include <ignition/common/Util.hh>

#include "ignition/rendering/ogre2/Ogre2Heightmap.hh"
#include "ignition/rendering/ogre2/Ogre2Conversions.hh"
#include "ignition/rendering/ogre2/Ogre2Light.hh"
#include "ignition/rendering/ogre2/Ogre2RenderEngine.hh"
#include "ignition/rendering/ogre2/Ogre2Scene.hh"

#include "Terra/Terra.h"

#ifdef _MSC_VER
  #pragma warning(push, 0)
#endif
#include <OgreImage2.h>
#include <OgreRoot.h>
#include <OgreSceneManager.h>
#ifdef _MSC_VER
  #pragma warning(pop)
#endif

//////////////////////////////////////////////////
class ignition::rendering::Ogre2HeightmapPrivate
{
  /// \brief The raw height values.
  public: std::vector<float> heights;

  /// \brief Size of the heightmap data.
  public: unsigned int dataSize{0u};

  public: std::unique_ptr<Ogre::Terra> terra{nullptr};
};

using namespace ignition;
using namespace rendering;

//////////////////////////////////////////////////
Ogre2Heightmap::Ogre2Heightmap(const HeightmapDescriptor &_desc)
    : BaseHeightmap(_desc), dataPtr(std::make_unique<Ogre2HeightmapPrivate>())
{
}

//////////////////////////////////////////////////
Ogre2Heightmap::~Ogre2Heightmap()
{
}

//////////////////////////////////////////////////
void Ogre2Heightmap::Init()
{
  Ogre2Object::Init();

  if (this->descriptor.Data() == nullptr)
  {
    ignerr << "Failed to initialize: null heightmap data." << std::endl;
    return;
  }

  if (this->descriptor.Name().empty())
    this->descriptor.SetName(this->Name());

  // Add paths
  for (auto i = 0u; i < this->descriptor.TextureCount(); ++i)
  {
    auto texture = this->descriptor.TextureByIndex(i);
    Ogre2RenderEngine::Instance()->AddResourcePath(texture->Diffuse());
    Ogre2RenderEngine::Instance()->AddResourcePath(texture->Normal());
  }


  // Terra is optimized to work with UNORM heightmaps, therefore it assumes
  // lowest height is 0.
  // So we move the heightmap so that its min elevation = 0 before feeding to
  // ogre. It is later translated back by the setOrigin call.
  float minElevation = 0.0;

  // TODO(chapulina) add a virtual HeightmapData::MinElevation function to
  // avoid the ifdef check. i.e. heightmapSizeZ = MaxElevation - MinElevation
  double heightmapSizeZ = this->descriptor.Data()->MaxElevation();

  // \todo These parameters shouldn't be hardcoded, and instead parametrized so
  // that they can be made consistent across different libraries (like
  // ign-physics)
  bool flipY = false;
  // sampling size along image width and height
  unsigned int newWidth = (this->descriptor.Data()->Width() *
                           this->descriptor.Sampling());
  math::Vector3d scale;
  scale.X(this->descriptor.Size().X() / newWidth);
  scale.Y(this->descriptor.Size().Y() / newWidth);

  if (math::equal(heightmapSizeZ, 0.0))
    scale.Z(1.0);
  else
    scale.Z(fabs(this->descriptor.Size().Z()) / heightmapSizeZ);

  // Construct the heightmap lookup table
  std::vector<float> lookup;
  this->descriptor.Data()->FillHeightMap(this->descriptor.Sampling(),
      newWidth, this->descriptor.Size(), scale, flipY, lookup);
  this->dataPtr->heights.reserve(newWidth * newWidth);

  for (unsigned int y = 0; y < newWidth; ++y)
  {
    for (unsigned int x = 0; x < newWidth; ++x)
    {
      const size_t index = (newWidth - y - 1u) * newWidth + x;
      this->dataPtr->heights.push_back(lookup[index] - minElevation);
    }
  }

  this->dataPtr->dataSize = newWidth;

  if (this->dataPtr->heights.empty())
  {
    ignerr << "Failed to load terrain. Heightmap data is empty" << std::endl;
    return;
  }

  if (!math::isPowerOfTwo(this->dataPtr->dataSize))
  {
    ignerr << "Heightmap final sampling must satisfy 2^n."
           << std::endl << "size = (width * sampling) = sampling + 1"
           << std::endl << "[" << this->dataPtr->dataSize << "] = (["
           << this->descriptor.Data()->Width() << "] * ["
           << this->descriptor.Sampling() << "]) = ["
           << this->descriptor.Sampling() << "] + 1: "
        << std::endl;
    return;
  }

  // Create terrain group, which holds all the individual terrain instances.
  // Param 1: Pointer to the scene manager
  // Param 2: Alignment plane
  // Param 3: Number of vertices along one edge of the terrain (2^n+1).
  //          Terrains must be square, with each side a power of 2 in size
  // Param 4: World size of each terrain instance, in meters.

  auto ogreScene = std::dynamic_pointer_cast<Ogre2Scene>(this->Scene());

  Ogre::Image2 image;
  image.loadDynamicImage(this->dataPtr->heights.data(), newWidth, newWidth,
                         1u, Ogre::TextureTypes::Type2D,
                         Ogre::PFG_R32_FLOAT, false);

  math::Vector3d origin(
      this->descriptor.Position().X() + this->descriptor.Size().X() * 0.5,
      this->descriptor.Position().Y() + this->descriptor.Size().Y() * 0.5,
      this->descriptor.Position().Z() + this->descriptor.Size().Z() * 0.5);

  Ogre::Root *ogreRoot = Ogre2RenderEngine::Instance()->OgreRoot();
  Ogre::SceneManager *ogreSceneManager = ogreScene->OgreSceneManager();
  Ogre::CompositorManager2 *ogreCompMgr = ogreRoot->getCompositorManager2();

  // TODO(anyone): Gazebo doesn't support SCENE_STATIC scene nodes
  this->dataPtr->terra =
      std::make_unique<Ogre::Terra>(
        Ogre::Id::generateNewId<Ogre::MovableObject>(),
        &ogreSceneManager->_getEntityMemoryManager(
          Ogre::/*SCENE_STATIC*/SCENE_DYNAMIC),
        ogreSceneManager, 11u, ogreCompMgr, nullptr );
  this->dataPtr->terra->load(
        image,
        Ogre2Conversions::Convert(origin),
        Ogre2Conversions::Convert(this->descriptor.Size()),
        this->descriptor.Name());

#if 0
  // textures. The default material generator takes two materials per layer.
  //    1. diffuse_specular - diffuse texture with a specular map in the
  //    alpha channel
  //    2. normal_height - normal map with a height map in the alpha channel
  {
    // number of texture layers
    defaultimp.layerList.resize(this->descriptor.TextureCount());

    // The worldSize decides how big each splat of textures will be.
    // A smaller value will increase the resolution
    for (unsigned int i = 0; i < this->descriptor.TextureCount(); ++i)
    {
      auto texture = this->descriptor.TextureByIndex(i);

      defaultimp.layerList[i].worldSize = texture->Size();
      defaultimp.layerList[i].textureNames.push_back(texture->Diffuse());
      defaultimp.layerList[i].textureNames.push_back(texture->Normal());
    }
  }
#endif

  ignmsg << "Loading heightmap: " << this->descriptor.Name() << std::endl;
  auto time = std::chrono::steady_clock::now();

  // use ignition shaders
  this->CreateMaterial();

  ignmsg << "Heightmap loaded. Process took "
        <<  std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - time).count()
        << " ms." << std::endl;

#if 0
  // Calculate blend maps
  auto ti = this->dataPtr->terrainGroup->getTerrainIterator();
  while (ti.hasMoreElements())
  {
    this->InitBlendMaps(ti.getNext()->instance);
  }
#endif
}

//////////////////////////////////////////////////
void Ogre2Heightmap::PreRender()
{
}

///////////////////////////////////////////////////
void Ogre2Heightmap::UpdateForRender(Ogre::Camera *_activeCamera)
{
  // Get the first directional light
  Ogre2DirectionalLightPtr directionalLight;
  for (unsigned int i = 0; i < this->Scene()->LightCount(); ++i)
  {
     auto light = std::dynamic_pointer_cast<Ogre2DirectionalLight>(
         this->Scene()->LightByIndex(i));
    if (nullptr != light)
    {
      directionalLight = light;
      break;
    }
  }

  this->dataPtr->terra->setCamera(_activeCamera);
  if (directionalLight)
  {
    this->dataPtr->terra->update(
          Ogre2Conversions::Convert(directionalLight->Direction()));
  }
  else
  {
    this->dataPtr->terra->update(Ogre::Vector3::NEGATIVE_UNIT_Y);
  }
}

/////////////////////////////////////////////////
void Ogre2Heightmap::CreateMaterial()
{
#if 0
  if (!this->dataPtr->materialName.empty())
  {
    // init custom material generator
    Ogre::TerrainMaterialGeneratorPtr terrainMaterialGenerator;
    TerrainMaterial *terrainMaterial = OGRE_NEW TerrainMaterial(
        this->dataPtr->materialName);
    if (this->dataPtr->splitTerrain)
      terrainMaterial->setGridSize(this->dataPtr->numTerrainSubdivisions);
    terrainMaterialGenerator.bind(terrainMaterial);
    this->dataPtr->terrainGlobals->setDefaultMaterialGenerator(
        terrainMaterialGenerator);
  }
#endif
}

/////////////////////////////////////////////////
#if 0
bool Ogre2Heightmap::InitBlendMaps(Ogre::Terrain *_terrain)
{
  if (nullptr == _terrain)
  {
    ignerr << "Invalid terrain\n";
    return false;
  }

  // no blending to be done if there's only one texture or no textures at all.
  if (this->descriptor.BlendCount() <= 1u ||
      this->descriptor.TextureCount() <= 1u)
    return false;

  // Bounds check for following loop
  if (_terrain->getLayerCount() < this->descriptor.BlendCount() + 1)
  {
    ignerr << "Invalid terrain, too few layers ["
           << unsigned(_terrain->getLayerCount())
           << "] for the number of blends ["
           << this->descriptor.BlendCount() << "] to initialize blend map"
           << std::endl;
    return false;
  }

  // Create the blend maps
  std::vector<Ogre::TerrainLayerBlendMap *> blendMaps;
  std::vector<float*> pBlend;
  unsigned int i{0u};

  for (i = 0; i < this->descriptor.BlendCount(); ++i)
  {
    blendMaps.push_back(_terrain->getLayerBlendMap(i+1));
    pBlend.push_back(blendMaps[i]->getBlendPointer());
  }

  // Set the blend values based on the height of the terrain
  Ogre::Real val, height;
  for (Ogre::uint16 y = 0; y < _terrain->getLayerBlendMapSize(); ++y)
  {
    for (Ogre::uint16 x = 0; x < _terrain->getLayerBlendMapSize(); ++x)
    {
      Ogre::Real tx, ty;

      blendMaps[0]->convertImageToTerrainSpace(x, y, &tx, &ty);
      height = _terrain->getHeightAtTerrainPosition(tx, ty);

      for (i = 0; i < this->descriptor.BlendCount(); ++i)
      {
        auto blend = this->descriptor.BlendByIndex(i);
        val = (height - blend->MinHeight()) / blend->FadeDistance();
        val = Ogre::Math::Clamp(val, (Ogre::Real)0, (Ogre::Real)1);
        *pBlend[i]++ = val;
      }
    }
  }

  // Make sure the blend maps are properly updated
  for (auto map : blendMaps)
  {
    map->dirty();
    map->update();
  }
  return true;
}
#endif

//////////////////////////////////////////////////
Ogre::MovableObject *Ogre2Heightmap::OgreObject() const
{
  return this->dataPtr->terra.get();
}

//////////////////////////////////////////////////
void Ogre2Heightmap::SetMaterial(MaterialPtr, bool)
{
  // no-op
}

//////////////////////////////////////////////////
MaterialPtr Ogre2Heightmap::Material() const
{
  return nullptr;
}
