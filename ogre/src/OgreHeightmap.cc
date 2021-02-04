/*
 * Copyright (C) 2020 Open Source Robotics Foundation
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

#include "ignition/rendering/ogre/OgreCamera.hh"
#include "ignition/rendering/ogre/OgreConversions.hh"
#include "ignition/rendering/ogre/OgreHeightmap.hh"
#include "ignition/rendering/ogre/OgreLight.hh"
#include "ignition/rendering/ogre/OgreMaterial.hh"
#include "ignition/rendering/ogre/OgreRenderEngine.hh"
#include "ignition/rendering/ogre/OgreRTShaderSystem.hh"
#include "ignition/rendering/ogre/OgreScene.hh"

#if OGRE_VERSION_MAJOR == 1 && OGRE_VERSION_MINOR >= 11
// Since OGRE 1.11, the once public
// Ogre::TerrainMaterialGeneratorA::SM2Profile::ShaderHelper
// class and its descendant are now private classes of OGRE, see
// * https://github.com/OGRECave/ogre/blob/master/Docs/1.11-Notes.md#other
// * https://github.com/OGRECave/ogre/pull/722
//
// As these classes are heavily used in the Heightmap class implementation
// (by accessing a protected Ogre class) we need to disable the definition
// of the custom terrain  generator, and just use the Ogre default one.
using Ogre::TechniqueType;
#endif

#if OGRE_VERSION_MAJOR == 1 && OGRE_VERSION_MINOR < 11
/// \internal
/// \brief Custom terrain material generator for GLSL terrains.
/// A custom material generator that lets Ignition use GLSL shaders
/// (as opposed to the default Cg shaders provided by Ogre) for rendering
/// terrain.
class IgnTerrainMatGen : public Ogre::TerrainMaterialGeneratorA
{
  /// \brief Constructor
  public: IgnTerrainMatGen();

  /// \brief Destructor
  public: virtual ~IgnTerrainMatGen();

  /// \brief Shader model 2 profile target.
  public: class SM2Profile :
          public Ogre::TerrainMaterialGeneratorA::SM2Profile
  {
    /// \brief Constructor
    public: SM2Profile(Ogre::TerrainMaterialGenerator *_parent,
                const Ogre::String &_name, const Ogre::String &_desc);

    /// \brief Destructor
    public: virtual ~SM2Profile();

    public: Ogre::MaterialPtr generate(const Ogre::Terrain *_terrain);

    public: Ogre::MaterialPtr generateForCompositeMap(
                const Ogre::Terrain *_terrain);

    public: void UpdateParams(const Ogre::MaterialPtr &_mat,
                              const Ogre::Terrain *_terrain);

    public: void UpdateParamsForCompositeMap(const Ogre::MaterialPtr &_mat,
                                             const Ogre::Terrain *_terrain);

    protected: virtual void addTechnique(const Ogre::MaterialPtr &_mat,
                       const Ogre::Terrain *_terrain, TechniqueType _tt);

    /// \brief Utility class to help with generating shaders for GLSL.
    /// The class contains a collection of functions that are used to
    /// dynamically generate a complete vertex or fragment shader program
    /// in a string format.
    protected: class ShaderHelperGLSL :
        public Ogre::TerrainMaterialGeneratorA::SM2Profile::ShaderHelperGLSL
    {
      public: ShaderHelperGLSL()
        {
          auto capabilities =
              Ogre::Root::getSingleton().getRenderSystem()->getCapabilities();
          Ogre::DriverVersion glVersion;
          glVersion.build = 0;
          glVersion.major = 3;
          glVersion.minor = 0;
          glVersion.release = 0;
          if (capabilities->isDriverOlderThanVersion(glVersion))
          {
            this->glslVersion = "120";
            this->vpInStr = "attribute";
            this->vpOutStr = "varying";
            this->fpInStr = "varying";
            this->textureStr = "texture2D";
          }
        };

      public: virtual Ogre::HighLevelGpuProgramPtr generateVertexProgram(
                  const SM2Profile *_prof, const Ogre::Terrain *_terrain,
                  TechniqueType _tt);

      public: virtual Ogre::HighLevelGpuProgramPtr generateFragmentProgram(
                  const SM2Profile *_prof, const Ogre::Terrain *_terrain,
                  TechniqueType _tt);

      public: virtual void updateParams(const SM2Profile *_prof,
                  const Ogre::MaterialPtr &_mat,
                  const Ogre::Terrain *_terrain, bool _compositeMap);

      protected: virtual void generateVpHeader(const SM2Profile *_prof,
                     const Ogre::Terrain *_terrain, TechniqueType _tt,
                     Ogre::StringStream &_outStream);

      protected: virtual void generateVpFooter(const SM2Profile *_prof,
                     const Ogre::Terrain *_terrain, TechniqueType _tt,
                     Ogre::StringStream &_outStream);

      protected: virtual void generateVertexProgramSource(
                     const SM2Profile *_prof, const Ogre::Terrain *_terrain,
                     TechniqueType _tt,
                     Ogre::StringStream &_outStream);

      protected: virtual void defaultVpParams(const SM2Profile *_prof,
                     const Ogre::Terrain *_terrain, TechniqueType _tt,
                     const Ogre::HighLevelGpuProgramPtr &_prog);

      protected: virtual unsigned int generateVpDynamicShadowsParams(
                     unsigned int _texCoordStart, const SM2Profile *_prof,
                     const Ogre::Terrain *_terrain, TechniqueType _tt,
                     Ogre::StringStream &_outStream);

      protected: virtual void generateVpDynamicShadows(
                     const SM2Profile *_prof, const Ogre::Terrain *_terrain,
                     TechniqueType _tt,
                     Ogre::StringStream &_outStream);

      protected: virtual void generateFpHeader(const SM2Profile *_prof,
                     const Ogre::Terrain *_terrain,
                     TechniqueType tt,
                     Ogre::StringStream &_outStream);

      protected: virtual void generateFpLayer(const SM2Profile *_prof,
                     const Ogre::Terrain *_terrain, TechniqueType tt,
                     Ogre::uint _layer,
                     Ogre::StringStream &_outStream);

      protected: virtual void generateFpFooter(const SM2Profile *_prof,
                     const Ogre::Terrain *_terrain,
                     TechniqueType tt,
                     Ogre::StringStream &_outStream);

      protected: virtual void generateFpDynamicShadowsParams(
                     Ogre::uint *_texCoord, Ogre::uint *_sampler,
                     const SM2Profile *_prof, const Ogre::Terrain *_terrain,
                     TechniqueType _tt,
                     Ogre::StringStream &_outStream);

      protected: virtual void generateFpDynamicShadowsHelpers(
                     const SM2Profile *_prof,
                     const Ogre::Terrain *_terrain,
                     TechniqueType tt,
                     Ogre::StringStream &_outStream);

      protected: void generateFpDynamicShadows(const SM2Profile *_prof,
                     const Ogre::Terrain *_terrain, TechniqueType _tt,
                     Ogre::StringStream &_outStream);

      protected: virtual void generateFragmentProgramSource(
                     const SM2Profile *_prof,
                     const Ogre::Terrain *_terrain,
                     TechniqueType _tt,
                     Ogre::StringStream &_outStream);

      protected: virtual void updateVpParams(const SM2Profile *_prof,
                     const Ogre::Terrain *_terrain, TechniqueType _tt,
                     const Ogre::GpuProgramParametersSharedPtr &_params);

      private: Ogre::String GetChannel(Ogre::uint _idx);

      /// \brief Capabilities
      private: std::string glslVersion = "130";
      private: std::string vpInStr = "in";
      private: std::string vpOutStr = "out";
      private: std::string fpInStr = "in";
      private: std::string fpOutStr = "out";
      private: std::string textureStr = "texture";
    };

    // Needed to allow access from ShaderHelperGLSL to protected members
    // of SM2Profile.
    friend ShaderHelperGLSL;
  };
};

// #if OGRE_VERSION_MAJOR == 1 && OGRE_VERSION_MINOR < 11
#endif

/// \internal
/// \brief Custom terrain material generator.
/// A custom material generator that lets user specify their own material
/// script for rendering the heightmap.
class TerrainMaterial : public Ogre::TerrainMaterialGenerator
{
  /// \brief Constructor
  /// \param[in] _materialName Name of material
  public: explicit TerrainMaterial(const std::string &_materialName);

  /// \brief Set terrain material
  /// \param[in] _materialName Name of material
  public: void setMaterialByName(const std::string &_materialname);

  /// \brief Set the grid size of the terrain, i.e. Number of terrain slots.
  /// This will be used to determined how the texture will be mapped to the
  /// terrain
  /// \param[in] _size Grid size of the terrain
  public: void setGridSize(const unsigned int _size);

  /// \brief Subclassed to provide profile-specific material generation
  class Profile : public Ogre::TerrainMaterialGenerator::Profile
  {
    /// \brief Constructor
    /// \param[in] _parent Ogre terrain material generator object
    /// \param[in] _name Name of the profile
    /// \param[on] _desc Profile description
    public: Profile(Ogre::TerrainMaterialGenerator *_parent,
        const Ogre::String &_name, const Ogre::String &_desc);

    /// \brief Destructor.
    public: ~Profile();

    // Documentation Inherited
    public: bool isVertexCompressionSupported() const override;

    // Documentation Inherited
    public: Ogre::MaterialPtr generate(const Ogre::Terrain *_terrain) override;

    // Documentation Inherited
    public: Ogre::MaterialPtr generateForCompositeMap(
        const Ogre::Terrain *_terrain) override;

    // Documentation Inherited
    public: void setLightmapEnabled(bool _enabled) override;

    // Documentation Inherited
    public: Ogre::uint8 getMaxLayers(const Ogre::Terrain *_terrain) const
        override;

    // Documentation Inherited
    public: void updateParams(const Ogre::MaterialPtr& mat,
        const Ogre::Terrain *_terrain) override;

    // Documentation Inherited
    public: void updateParamsForCompositeMap(const Ogre::MaterialPtr& mat,
        const Ogre::Terrain *_terrain) override;

    // Documentation Inherited
    public: void requestOptions(Ogre::Terrain *_terrain) override;
  };

  /// \brief Name of material
  protected: std::string materialName;

  /// \brief Size of grid
  protected: unsigned int gridSize = 1u;
};

/// \internal
/// \brief Pretends to provide procedural page content to avoid page loading
class DummyPageProvider : public Ogre::PageProvider
{
  /// \brief Give a provider the opportunity to prepare page content
  /// procedurally. The parameters are not used.
  public: bool prepareProceduralPage(Ogre::Page *, Ogre::PagedWorldSection *)
  {
    return true;
  }

  /// \brief Give a provider the opportunity to load page content
  /// procedurally. The parameters are not used.
  public: bool loadProceduralPage(Ogre::Page *, Ogre::PagedWorldSection *)
  {
    return true;
  }

  /// \brief Give a provider the opportunity to unload page content
  /// procedurally. The parameters are not used.
  public: bool unloadProceduralPage(Ogre::Page *, Ogre::PagedWorldSection *)
  {
    return true;
  }

  /// \brief Give a provider the opportunity to unprepare page content
  /// procedurally. The parameters are not used.
  public: bool unprepareProceduralPage(Ogre::Page *, Ogre::PagedWorldSection *)
  {
    return true;
  }
};

//////////////////////////////////////////////////
class ignition::rendering::OgreHeightmapPrivate
{
  /// \brief Global options - in some Ogre versions, this is enforced as a
  /// singleton.
  public: static Ogre::TerrainGlobalOptions *terrainGlobals;

  /// \brief The raw height values.
  public: std::vector<float> heights;

  /// \brief Size of the heightmap data.
  public: unsigned int dataSize{0u};

  /// \brief True if the terrain need to be split into subterrains
  public: bool splitTerrain{false};

  /// \brief Number of pieces in which a terrain is subdivided. Used
  /// for paging and also when heighmap is too large for LOD to work.
  public: unsigned int numTerrainSubdivisions{16u};

  /// \brief Max pixel error allowed for rendering the heightmap. This
  /// affects the transitions between LOD levels.
  public: double maxPixelError{0.0};

  /// \brief Group of terrains.
  public: Ogre::TerrainGroup *terrainGroup{nullptr};

  /// \brief Skirt length on LOD tiles
  public: double skirtLength{1.0};

  /// \brief Terrain casts shadows
  /// \todo(chapulina) Use Material? Expose through descriptor?
  public: bool castShadows{false};

  /// \brief True if the terrain's hash does not match the image's hash
  public: bool terrainHashChanged{true};

  /// \brief Hash file name that should be present for every terrain file
  /// loaded using paging.
  public: const std::string kHashFilename{"ignterrain.SHA1"};

  /// \brief Collection of terrains. Every terrain might be paged.
  public: std::vector<std::vector<float>> subTerrains;

  /// \brief When the terrain paging is enabled, the terrain information
  /// for every piece of terrain is stored in disk. This is the path of
  /// the top level directory where these files are located.
  public: std::string pagingDir;

  /// \brief Name of the top level directory where all the paging info is
  /// stored
  public: const std::string pagingDirname{"ogre-paging"};

  /// \brief Central registration point for extension classes,
  /// such as the PageStrategy, PageContentFactory.
  public: Ogre::PageManager *pageManager{nullptr};

  /// \brief A page provider is needed to use the paging system.
  public: DummyPageProvider dummyPageProvider;

  /// \brief Collection of world content
  public: Ogre::PagedWorld *world{nullptr};

  /// \brief Type of paging applied
  public: Ogre::TerrainPaging *terrainPaging{nullptr};

  /// \brief The terrain pages are loaded if the distance from the camera is
  /// within the loadRadius. See Ogre::TerrainPaging::createWorldSection().
  /// LoadRadiusFactor is a multiplier applied to the terrain size to create
  /// a load radius that depends on the terrain size.
  public: const double loadRadiusFactor{1.0};

  /// \brief The terrain pages are held in memory but not loaded if they
  /// are not ready when the camera is within holdRadius distance. See
  /// Ogre::TerrainPaging::createWorldSection(). HoldRadiusFactor is a
  /// multiplier applied to the terrain size to create a hold radius that
  /// depends on the terrain size.
  public: const double holdRadiusFactor{1.15};

  /// \brief True if the terrain was loaded from the cache.
  public: bool loadedFromCache{false};

  /// \brief True if the terrain was saved to the cache.
  public: bool savedToCache{false};

  /// \brief Used to iterate over all the terrains
  public: int terrainIdx{0};

  /// \brief Name of custom material to use for the terrain. If empty,
  /// default material with glsl shader will be used.
  /// \todo(chapulina) Will we ever use this?
  public: std::string materialName;

#if OGRE_VERSION_MAJOR == 1 && OGRE_VERSION_MINOR < 11
  /// \brief Pointer to the terrain material generator.
  public: IgnTerrainMatGen *ignMatGen{nullptr};
#endif
};

Ogre::TerrainGlobalOptions
    *ignition::rendering::OgreHeightmapPrivate::terrainGlobals = nullptr;

using namespace ignition;
using namespace rendering;

//////////////////////////////////////////////////
OgreHeightmap::OgreHeightmap(const HeightmapDescriptor &_desc)
    : BaseHeightmap(_desc), dataPtr(std::make_unique<OgreHeightmapPrivate>())
{
  std::string home;
  ignition::common::env(IGN_HOMEDIR, home);

  this->dataPtr->pagingDir =
      common::joinPaths(home, ".ignition", "rendering",
      this->dataPtr->pagingDirname);
}

//////////////////////////////////////////////////
OgreHeightmap::~OgreHeightmap()
{
}

//////////////////////////////////////////////////
void OgreHeightmap::Init()
{
  OgreObject::Init();

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
    OgreRenderEngine::Instance()->AddResourcePath(texture->Diffuse());
    OgreRenderEngine::Instance()->AddResourcePath(texture->Normal());
  }

  // The terraingGroup is composed by a number of terrains (1 by default)
  int nTerrains = 1;

  // There is an issue with OGRE terrain LOD if heights are not relative to 0.
  // So we move the heightmap so that its min elevation = 0 before feeding to
  // ogre. It is later translated back by the setOrigin call.
  double minElevation = 0.0;

  // TODO(chapulina) add a virtual HeightmapData::MinElevation function to
  // avoid the ifdef check. i.e. heightmapSizeZ = MaxElevation - MinElevation
  double heightmapSizeZ = this->descriptor.Data()->MaxElevation();

  // \todo These parameters shouldn't be hardcoded, and instead parametrized so
  // that they can be made consistent across different libraries (like
  // ign-physics)
  bool flipY = false;
  // sampling size along image width and height
  unsigned int vertSize = (this->descriptor.Data()->Width() *
      this->descriptor.Sampling()) - this->descriptor.Sampling() + 1;
  math::Vector3d scale;
  scale.X(this->descriptor.Size().X() / vertSize);
  scale.Y(this->descriptor.Size().Y() / vertSize);

  if (math::equal(heightmapSizeZ, 0.0))
    scale.Z(1.0);
  else
    scale.Z(fabs(this->descriptor.Size().Z()) / heightmapSizeZ);

  // Construct the heightmap lookup table
  std::vector<float> lookup;
  this->descriptor.Data()->FillHeightMap(this->descriptor.Sampling(),
      vertSize, this->descriptor.Size(), scale, flipY, lookup);

  for (unsigned int y = 0; y < vertSize; ++y)
  {
    for (unsigned int x = 0; x < vertSize; ++x)
    {
      int index = (vertSize - y - 1) * vertSize + x;
      this->dataPtr->heights.push_back(lookup[index] - minElevation);
    }
  }

  this->dataPtr->dataSize = vertSize;

  if (this->dataPtr->heights.empty())
  {
    ignerr << "Failed to load terrain. Heightmap data is empty" << std::endl;
    return;
  }

  if (!math::isPowerOfTwo(this->dataPtr->dataSize - 1))
  {
    ignerr << "Heightmap final sampling must satisfy 2^n+1."
           << std::endl << "size = (width * sampling) = sampling + 1"
           << std::endl << "[" << this->dataPtr->dataSize << "] = (["
           << this->descriptor.Data()->Width() << "] * ["
           << this->descriptor.Sampling() << "]) = ["
           << this->descriptor.Sampling() << "] + 1: "
        << std::endl;
    return;
  }

  // Get the full path of the image heightmap
  // \todo(anyone) Name is generated at runtime and depends on the number of
  // objects, so it's impossible to make sure it's the same across runs
  auto terrainDirPath = common::joinPaths(this->dataPtr->pagingDir,
      this->descriptor.Name());

  // Add the top level terrain paging directory to the OGRE
  // ResourceGroupManager
  if (!Ogre::ResourceGroupManager::getSingleton().resourceLocationExists(
        this->dataPtr->pagingDir, "General"))
  {
    Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
        this->dataPtr->pagingDir, "FileSystem", "General", true);
    Ogre::ResourceGroupManager::getSingleton().initialiseResourceGroup(
        "General");
  }

  // If the paging is enabled we modify the number of subterrains
  std::string prefix;
  if (this->descriptor.UseTerrainPaging())
  {
    this->dataPtr->splitTerrain = true;
    nTerrains = this->dataPtr->numTerrainSubdivisions;
    prefix = common::joinPaths(terrainDirPath, "ignition_terrain_cache");
  }
  else
  {
    // Note: ran into problems with LOD height glitches if heightmap size is
    // larger than 4096 so split it into chunks
    // Note: dataSize should be 2^n + 1
    if (this->dataPtr->maxPixelError > 0 && this->dataPtr->dataSize > 4096u)
    {
      this->dataPtr->splitTerrain = true;
      if (this->dataPtr->dataSize == 4097u)
        this->dataPtr->numTerrainSubdivisions = 4u;
      else
        this->dataPtr->numTerrainSubdivisions = 16u;
      nTerrains = this->dataPtr->numTerrainSubdivisions;

      ignmsg << "Large heightmap used with LOD. It will be subdivided into " <<
          this->dataPtr->numTerrainSubdivisions << " terrains." << std::endl;
    }
    prefix = common::joinPaths(terrainDirPath, "ignition_terrain");
  }

  double sqrtN = sqrt(nTerrains);

  // Create terrain group, which holds all the individual terrain instances.
  // Param 1: Pointer to the scene manager
  // Param 2: Alignment plane
  // Param 3: Number of vertices along one edge of the terrain (2^n+1).
  //          Terrains must be square, with each side a power of 2 in size
  // Param 4: World size of each terrain instance, in meters.

  auto ogreScene = std::dynamic_pointer_cast<OgreScene>(this->Scene());

  this->dataPtr->terrainGroup = new Ogre::TerrainGroup(
      ogreScene->OgreSceneManager(), Ogre::Terrain::ALIGN_X_Y,
      1 + ((this->dataPtr->dataSize - 1) / static_cast<unsigned int>(sqrtN)),
      this->descriptor.Size().X() / (sqrtN));

  this->dataPtr->terrainGroup->setFilenameConvention(
    Ogre::String(prefix), Ogre::String("dat"));

  math::Vector3d pos(
      this->descriptor.Position().X() - 0.5 * this->descriptor.Size().X() +
      0.5 * this->descriptor.Size().X() / sqrtN,
      this->descriptor.Position().Y() - 0.5 * this->descriptor.Size().X() +
      0.5 * this->descriptor.Size().X() / sqrtN,
      this->descriptor.Position().Z() + minElevation);

  this->dataPtr->terrainGroup->setOrigin(OgreConversions::Convert(pos));

  this->ConfigureTerrainDefaults();

  // Configure import settings
  auto &defaultimp = this->dataPtr->terrainGroup->getDefaultImportSettings();

  defaultimp.terrainSize = this->dataPtr->dataSize;
  defaultimp.worldSize = this->descriptor.Size().X();

  defaultimp.inputScale = 1.0;

  defaultimp.minBatchSize = 17;
  defaultimp.maxBatchSize = 65;

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

  this->dataPtr->terrainHashChanged = this->PrepareTerrain(terrainDirPath);

  if (this->descriptor.UseTerrainPaging())
  {
    if (this->dataPtr->terrainHashChanged)
    {
      // Split the terrain. Every subterrain will be saved on disk and paged
      this->SplitHeights(this->dataPtr->heights, nTerrains,
          this->dataPtr->subTerrains);
    }

    this->dataPtr->pageManager = OGRE_NEW Ogre::PageManager();
    this->dataPtr->pageManager->setPageProvider(
        &this->dataPtr->dummyPageProvider);

    // Add cameras
    for (unsigned int i = 0; i < ogreScene->NodeCount(); ++i)
    {
      auto cam = std::dynamic_pointer_cast<OgreCamera>(
          ogreScene->NodeByIndex(i));
      if (nullptr != cam)
      {
        this->dataPtr->pageManager->addCamera(cam->Camera());
      }
    }

    this->dataPtr->terrainPaging =
        OGRE_NEW Ogre::TerrainPaging(this->dataPtr->pageManager);
    this->dataPtr->world = this->dataPtr->pageManager->createWorld();
    this->dataPtr->terrainPaging->createWorldSection(
        this->dataPtr->world, this->dataPtr->terrainGroup,
        this->dataPtr->loadRadiusFactor * this->descriptor.Size().X(),
        this->dataPtr->holdRadiusFactor * this->descriptor.Size().X(),
        0, 0, static_cast<unsigned int>(sqrtN) - 1,
        static_cast<unsigned int>(sqrtN) - 1);
  }

  ignmsg << "Loading heightmap: " << this->descriptor.Name() << std::endl;
  auto time = std::chrono::steady_clock::now();

  for (int y = 0; y <= sqrtN - 1; ++y)
    for (int x = 0; x <= sqrtN - 1; ++x)
      this->DefineTerrain(x, y);

  // use ignition shaders
  this->CreateMaterial();

  // Sync load since we want everything in place when we start
  this->dataPtr->terrainGroup->loadAllTerrains(true);

  ignmsg << "Heightmap loaded. Process took "
        <<  std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - time).count()
        << " ms." << std::endl;

  // Calculate blend maps
  if (!this->dataPtr->loadedFromCache)
  {
    auto ti = this->dataPtr->terrainGroup->getTerrainIterator();
    while (ti.hasMoreElements())
    {
      this->InitBlendMaps(ti.getNext()->instance);
    }
  }

  this->dataPtr->terrainGroup->freeTemporaryResources();
}

//////////////////////////////////////////////////
void OgreHeightmap::PreRender()
{
  if (nullptr == this->dataPtr->terrainGroup)
  {
    return;
  }

  // Make sure the heightmap finishes loading by processing responses until all
  // derived data and terrains are loaded
  if (this->dataPtr->terrainGroup->isDerivedDataUpdateInProgress())
  {
    Ogre::Root::getSingleton().getWorkQueue()->processResponses();
    return;
  }

  auto ti = this->dataPtr->terrainGroup->getTerrainIterator();
  while (ti.hasMoreElements())
  {
    auto *t = ti.getNext()->instance;
    if (!t->isLoaded())
    {
      Ogre::Root::getSingleton().getWorkQueue()->processResponses();
      return;
    }
  }

  // save the terrain once its loaded
  if (this->dataPtr->loadedFromCache || this->dataPtr->savedToCache)
  {
    return;
  }

  // saving an ogre terrain data file can take quite some time for large
  // terrains.
  ignmsg << "Saving heightmap cache data to "
         << common::joinPaths(this->dataPtr->pagingDir, this->descriptor.Name())
         << std::endl;
  auto time = std::chrono::steady_clock::now();

  bool saved{false};
  try
  {
    this->dataPtr->terrainGroup->saveAllTerrains(true);
    saved = true;
  }
  catch(Ogre::Exception &_e)
  {
    ignerr << "Failed to save heightmap: " << _e.what() << std::endl;
  }

  if (saved)
  {
    ignmsg << "Heightmap cache data saved. Process took "
          <<  std::chrono::duration_cast<std::chrono::milliseconds>(
              std::chrono::steady_clock::now() - time).count()
          << " ms." << std::endl;
  }

  this->dataPtr->savedToCache = true;
}

///////////////////////////////////////////////////
void OgreHeightmap::ConfigureTerrainDefaults()
{
  if (this->dataPtr->terrainGlobals != nullptr)
    return;

  // Configure global
  this->dataPtr->terrainGlobals = new Ogre::TerrainGlobalOptions();

  // Vertex compression breaks anything, e.g. Gpu laser, that tries to build
  // a depth map.
  this->dataPtr->terrainGlobals->setUseVertexCompressionWhenAvailable(false);

  // MaxPixelError: Decides how precise our terrain is going to be.
  // A lower number will mean a more accurate terrain, at the cost of
  // performance (because of more vertices)
  this->dataPtr->terrainGlobals->setMaxPixelError(this->dataPtr->maxPixelError);

  // CompositeMapDistance: decides how far the Ogre terrain will render
  // the lightmapped terrain.
  this->dataPtr->terrainGlobals->setCompositeMapDistance(2000);

  // Vertex compression breaks anything, e.g. Gpu laser, that tries to build
  // a depth map.
  this->dataPtr->terrainGlobals->setUseVertexCompressionWhenAvailable(false);

  this->dataPtr->terrainGlobals->setSkirtSize(this->dataPtr->skirtLength);

  this->dataPtr->terrainGlobals->setCastsDynamicShadows(
        this->dataPtr->castShadows);

  auto ogreScene = std::dynamic_pointer_cast<OgreScene>(this->Scene());
  this->dataPtr->terrainGlobals->setCompositeMapAmbient(
      ogreScene->OgreSceneManager()->getAmbientLight());

  // Get the first directional light
  OgreDirectionalLightPtr directionalLight;
  for (unsigned int i = 0; i < this->Scene()->LightCount(); ++i)
  {
     auto light = std::dynamic_pointer_cast<OgreDirectionalLight>(
         this->Scene()->LightByIndex(i));
    if (nullptr != light)
    {
      directionalLight = light;
      break;
    }
  }

  // Important to set these so that the terrain knows what to use for
  // derived (non-realtime) data
  if (directionalLight)
  {
    this->dataPtr->terrainGlobals->setLightMapDirection(
        OgreConversions::Convert(directionalLight->Direction()));

    auto const &lightDiffuse = directionalLight->DiffuseColor();
    this->dataPtr->terrainGlobals->setCompositeMapDiffuse(
        OgreConversions::Convert(lightDiffuse));
  }
  else
  {
    this->dataPtr->terrainGlobals->setLightMapDirection(
        Ogre::Vector3(0, 0, -1));
    this->dataPtr->terrainGlobals->setCompositeMapDiffuse(
        Ogre::ColourValue(0.6f, 0.6f, 0.6f, 1.0f));
  }
}

//////////////////////////////////////////////////
bool OgreHeightmap::PrepareTerrain(
    const std::string &_terrainDirPath)
{
  // Compute the original heightmap's image.
  auto heightmapHash = common::sha1<std::vector<float>>(this->dataPtr->heights);

  // Check if the terrain hash exists
  auto terrainHashFullPath = common::joinPaths(_terrainDirPath,
      this->dataPtr->kHashFilename);

  bool updateHash = true;
  if (common::exists(terrainHashFullPath))
  {
    try
    {
      // Read the terrain hash
      std::ifstream in(terrainHashFullPath.c_str());
      std::stringstream buffer;
      buffer << in.rdbuf();
      std::string terrainHash(buffer.str());
      updateHash = terrainHash != heightmapHash;
    }
    catch(std::ifstream::failure &_e)
    {
      ignerr << "Terrain paging error: Unable to read terrain hash ["
             << _e.what() << "]" << std::endl;
    }
  }

  // Update the terrain hash and split the terrain into small pieces
  if (updateHash)
  {
    this->UpdateTerrainHash(heightmapHash, _terrainDirPath);
  }

  return updateHash;
}

//////////////////////////////////////////////////
void OgreHeightmap::UpdateTerrainHash(const std::string &_hash,
    const std::string &_terrainDir)
{
  // Create the subdirectories if they do not exist
  common::createDirectories(_terrainDir);

  auto terrainHashFullPath = common::joinPaths(_terrainDir,
      this->dataPtr->kHashFilename);

  // Update the terrain hash
  std::ofstream terrainHashFile;
  terrainHashFile.open(terrainHashFullPath.c_str());

  // Throw an error if we couldn't open the file for writing.
  if (terrainHashFile.is_open())
  {
    terrainHashFile << _hash;
    terrainHashFile.close();
  }
  else
  {
    ignerr << "Unable to open file for creating a terrain hash: [" +
        terrainHashFullPath + "]" << std::endl;
  }
}

//////////////////////////////////////////////////
void OgreHeightmap::SplitHeights(const std::vector<float> &_heightmap,
    int _n, std::vector<std::vector<float>> &_v)
{
  // We support splitting the terrain in 4 or 16 pieces
  if (_n != 4 && _n != 16)
  {
    ignerr << "Invalid number of terrain divisions [" << _n
           << "]. It should be 4 or 16." << std::endl;
    return;
  }

  int count = 0;
  int width = static_cast<int>(sqrt(_heightmap.size()));
  int newWidth = static_cast<int>(1 + (width - 1) / sqrt(_n));

  // Memory allocation
  _v.resize(_n);

  for (int tileR = 0; tileR < sqrt(_n); ++tileR)
  {
    int tileIndex = static_cast<int>(tileR * sqrt(_n));
    for (int row = 0; row < newWidth - 1; ++row)
    {
      for (int tileC = 0; tileC < sqrt(_n); ++tileC)
      {
        for (int col = 0; col < newWidth - 1; ++col)
        {
          _v[tileIndex].push_back(_heightmap[count]);
          ++count;
        }
        // Copy last value into the last column
        _v[tileIndex].push_back(_v[tileIndex].back());

        tileIndex = static_cast<int>(tileR * sqrt(_n) +
            (tileIndex + 1) % static_cast<int>(sqrt(_n)));
      }
      ++count;
    }
    // Copy the last row
    for (int i = 0; i < sqrt(_n); ++i)
    {
      tileIndex = static_cast<int>(tileR * sqrt(_n) + i);
      std::vector<float> lastRow(_v[tileIndex].end() - newWidth,
          _v[tileIndex].end());
      _v[tileIndex].insert(_v[tileIndex].end(),
          lastRow.begin(), lastRow.end());
    }
  }
}

/////////////////////////////////////////////////
void OgreHeightmap::DefineTerrain(int _x, int _y)
{
  Ogre::String filename = this->dataPtr->terrainGroup->generateFilename(_x, _y);

  bool resourceExists =
      Ogre::ResourceGroupManager::getSingleton().resourceExists(
      this->dataPtr->terrainGroup->getResourceGroup(), filename);

  if (resourceExists && !this->dataPtr->terrainHashChanged)
  {
    ignmsg << "Loading heightmap cache data: " << filename << std::endl;

    this->dataPtr->terrainGroup->defineTerrain(_x, _y);
    this->dataPtr->loadedFromCache = true;
  }
  else
  {
    if (this->dataPtr->splitTerrain)
    {
      // generate the subterrains if needed
      if (this->dataPtr->subTerrains.empty())
      {
        this->SplitHeights(this->dataPtr->heights,
            this->dataPtr->numTerrainSubdivisions,
            this->dataPtr->subTerrains);
      }

      this->dataPtr->terrainGroup->defineTerrain(_x, _y,
          &this->dataPtr->subTerrains[this->dataPtr->terrainIdx][0]);
      ++this->dataPtr->terrainIdx;
    }
    else
    {
      this->dataPtr->terrainGroup->defineTerrain(_x, _y,
          &this->dataPtr->heights[0]);
    }
  }
}

/////////////////////////////////////////////////
void OgreHeightmap::CreateMaterial()
{
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
  else
  {
#if OGRE_VERSION_MAJOR == 1 && OGRE_VERSION_MINOR < 11
    // use default material
    // RTSS PSSM shadows compatible terrain material
    if (!this->dataPtr->ignMatGen)
      this->dataPtr->ignMatGen = new IgnTerrainMatGen();

    auto ptr = Ogre::TerrainMaterialGeneratorPtr();
    ptr.bind(this->dataPtr->ignMatGen);

    this->dataPtr->terrainGlobals->setDefaultMaterialGenerator(ptr);
#else
    // init custom material generator
    Ogre::TerrainMaterialGeneratorPtr terrainMaterialGenerator;
    auto terrainMaterial = OGRE_NEW TerrainMaterial("Default/White");
    if (this->dataPtr->splitTerrain)
      terrainMaterial->setGridSize(this->dataPtr->numTerrainSubdivisions);
    terrainMaterialGenerator.bind(terrainMaterial);
    this->dataPtr->terrainGlobals->setDefaultMaterialGenerator(
        terrainMaterialGenerator);
#endif

    this->SetupShadows(true);
  }
}

/////////////////////////////////////////////////
void OgreHeightmap::SetupShadows(bool _enableShadows)
{
  auto matGen = this->dataPtr->terrainGlobals->getDefaultMaterialGenerator();

  // Assume we get a shader model 2 material profile
  Ogre::TerrainMaterialGeneratorA::SM2Profile *matProfile;
#if OGRE_VERSION_MAJOR == 1 && OGRE_VERSION_MINOR < 11
  matProfile = static_cast<IgnTerrainMatGen::SM2Profile *>(
      matGen->getActiveProfile());
#else
  matProfile = static_cast<Ogre::TerrainMaterialGeneratorA::SM2Profile *>(
      matGen->getActiveProfile());
#endif

  if (nullptr == matProfile)
  {
    // using custom material script so ignore setting shadows
    return;
  }

  matProfile->setLayerParallaxMappingEnabled(false);

  if (_enableShadows)
  {
    // Make sure PSSM is already setup
    matProfile->setReceiveDynamicShadowsEnabled(true);
    matProfile->setReceiveDynamicShadowsPSSM(
        OgreRTShaderSystem::Instance()->PSSMShadowCameraSetup());
    matProfile->setReceiveDynamicShadowsDepth(true);
    matProfile->setReceiveDynamicShadowsLowLod(false);
  }
  else
  {
    matProfile->setReceiveDynamicShadowsPSSM(nullptr);
  }
}

/////////////////////////////////////////////////
bool OgreHeightmap::InitBlendMaps(Ogre::Terrain *_terrain)
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

//////////////////////////////////////////////////
Ogre::MovableObject *OgreHeightmap::OgreObject() const
{
  return nullptr;
}

//////////////////////////////////////////////////
void OgreHeightmap::SetMaterial(MaterialPtr, bool)
{
  // no-op
}

//////////////////////////////////////////////////
MaterialPtr OgreHeightmap::Material() const
{
  return nullptr;
}

/////////////////////////////////////////////////
/////////////////////////////////////////////////
// IgnTerrainMatGen
/////////////////////////////////////////////////
/////////////////////////////////////////////////

#if OGRE_VERSION_MAJOR == 1 && OGRE_VERSION_MINOR < 11

/////////////////////////////////////////////////
IgnTerrainMatGen::IgnTerrainMatGen()
: TerrainMaterialGeneratorA()
{
  /// \TODO(anyone) - This will have to be changed if TerrainMaterialGeneratorA
  /// ever supports more profiles than only CG

  // Add custom SM2Profile SPAM
  this->mProfiles.clear();

  this->mProfiles.push_back(OGRE_NEW SM2Profile(this, "SM2",
        "Profile for rendering on Shader Model 2 capable cards "
        "(RTSS depth shadows compatible)"));

  /// \TODO(anyone) - check hardware capabilities & use fallbacks if required
  /// (more profiles needed)
  this->setActiveProfile(this->mProfiles[0]);
}

/////////////////////////////////////////////////
IgnTerrainMatGen::~IgnTerrainMatGen()
{
}

/////////////////////////////////////////////////
IgnTerrainMatGen::SM2Profile::SM2Profile(
    Ogre::TerrainMaterialGenerator *_parent, const Ogre::String &_name,
    const Ogre::String &_desc)
: TerrainMaterialGeneratorA::SM2Profile(_parent, _name, _desc)
{
  this->mShaderGen = nullptr;
}

/////////////////////////////////////////////////
IgnTerrainMatGen::SM2Profile::~SM2Profile()
{
  // Because the base SM2Profile has no virtual destructor:
  delete this->mShaderGen;
  this->mShaderGen = nullptr;
}

/////////////////////////////////////////////////
void IgnTerrainMatGen::SM2Profile::addTechnique(
    const Ogre::MaterialPtr &_mat, const Ogre::Terrain *_terrain,
    TechniqueType _tt)
{
  // Initiate specialized mShaderGen
  // Ogre::GpuProgramManager &gmgr = Ogre::GpuProgramManager::getSingleton();

  Ogre::HighLevelGpuProgramManager &hmgr =
    Ogre::HighLevelGpuProgramManager::getSingleton();

  if (!this->mShaderGen)
  {
    // By default we use the GLSL shaders.
    if (hmgr.isLanguageSupported("glsl"))
    {
      this->mShaderGen = OGRE_NEW
        IgnTerrainMatGen::SM2Profile::ShaderHelperGLSL();
    }
    else
    {
      ignerr << "No supported shader languages" << std::endl;
      return;
    }

    // check SM3 features
    this->mSM3Available =
      Ogre::GpuProgramManager::getSingleton().isSyntaxSupported("ps_3_0");
#if OGRE_VERSION_MAJOR == 1 && OGRE_VERSION_MINOR >= 8
    this->mSM4Available =
      Ogre::GpuProgramManager::getSingleton().isSyntaxSupported("ps_4_0");
#endif
  }

  // Unfortunately this doesn't work
  // Default implementation
  // TerrainMaterialGeneratorA::SM2Profile::addTechnique(mat, terrain, tt);

  // So we have to replicate the entire method:
  Ogre::Technique *tech = _mat->createTechnique();

  // Only supporting one pass
  Ogre::Pass *pass = tech->createPass();

  // Doesn't delegate to the proper method otherwise
  Ogre::HighLevelGpuProgramPtr vprog =
    ((IgnTerrainMatGen::SM2Profile::ShaderHelperGLSL*)this->mShaderGen)
    ->generateVertexProgram(this, _terrain, _tt);

  // DEBUG: std::cout << "VertShader[" << vprog->getName() << "]:\n"
  //          << vprog->getSource() << "\n\n";

  Ogre::HighLevelGpuProgramPtr fprog =
    ((IgnTerrainMatGen::SM2Profile::ShaderHelperGLSL*)this->mShaderGen)
    ->generateFragmentProgram(this, _terrain, _tt);

  // DEBUG: std::cout << "FragShader[" << fprog->getName() << "]:\n"
  //          << fprog->getSource() << "\n\n";

  pass->setVertexProgram(vprog->getName());
  pass->setFragmentProgram(fprog->getName());

  if (_tt == HIGH_LOD || _tt == RENDER_COMPOSITE_MAP)
  {
    // global normal map
    Ogre::TextureUnitState* tu = pass->createTextureUnitState();
    tu->setTextureName(_terrain->getTerrainNormalMap()->getName());
    tu->setTextureAddressingMode(Ogre::TextureUnitState::TAM_CLAMP);

    // global colour map
    if (_terrain->getGlobalColourMapEnabled() &&
        this->isGlobalColourMapEnabled())
    {
      tu = pass->createTextureUnitState(
          _terrain->getGlobalColourMap()->getName());
      tu->setTextureAddressingMode(Ogre::TextureUnitState::TAM_CLAMP);
    }

    // light map
    if (this->isLightmapEnabled())
    {
      tu = pass->createTextureUnitState(_terrain->getLightmap()->getName());
      tu->setTextureAddressingMode(Ogre::TextureUnitState::TAM_CLAMP);
    }

    // blend maps
    unsigned int maxLayers = this->getMaxLayers(_terrain);

    unsigned int numBlendTextures = std::min(
        _terrain->getBlendTextureCount(maxLayers),
        _terrain->getBlendTextureCount());

    unsigned int numLayers = std::min(
        maxLayers, static_cast<unsigned int>(_terrain->getLayerCount()));

    for (unsigned int i = 0; i < numBlendTextures; ++i)
    {
      tu = pass->createTextureUnitState(_terrain->getBlendTextureName(i));
      tu->setTextureAddressingMode(Ogre::TextureUnitState::TAM_CLAMP);
    }

    // layer textures
    for (unsigned int i = 0; i < numLayers; ++i)
    {
      // diffuse / specular
      pass->createTextureUnitState(_terrain->getLayerTextureName(i, 0));

      // normal / height
      pass->createTextureUnitState(_terrain->getLayerTextureName(i, 1));
    }
  }
  else
  {
    // LOW_LOD textures
    // composite map
    Ogre::TextureUnitState *tu = pass->createTextureUnitState();
    tu->setTextureName(_terrain->getCompositeMap()->getName());
    tu->setTextureAddressingMode(Ogre::TextureUnitState::TAM_CLAMP);
  }

  // Add shadow textures (always at the end)
  if (this->isShadowingEnabled(_tt, _terrain))
  {
    unsigned int numTextures = 1;

    if (this->getReceiveDynamicShadowsPSSM())
    {
      numTextures = this->getReceiveDynamicShadowsPSSM()->getSplitCount();
    }
    for (unsigned int i = 0; i < numTextures; ++i)
    {
      Ogre::TextureUnitState *tu = pass->createTextureUnitState();
      tu->setContentType(Ogre::TextureUnitState::CONTENT_SHADOW);
      tu->setTextureAddressingMode(Ogre::TextureUnitState::TAM_BORDER);
      tu->setTextureBorderColour(Ogre::ColourValue::White);
    }
  }
}

/////////////////////////////////////////////////
// generate() and generateForCompositeMap() are identical to
// TerrainMaterialGeneratorA implementation, the only reason for repeating
// them is that, unfortunately, addTechnique() is not declared virtual.
Ogre::MaterialPtr IgnTerrainMatGen::SM2Profile::generate(
    const Ogre::Terrain *_terrain)
{
  // re-use old material if exists
  Ogre::MaterialPtr mat = _terrain->_getMaterial();

  if (mat.isNull())
  {
    Ogre::MaterialManager &matMgr = Ogre::MaterialManager::getSingleton();

    // it's important that the names are deterministic for a given terrain, so
    // use the terrain pointer as an ID
    const Ogre::String &matName = _terrain->getMaterialName();
    mat = matMgr.getByName(matName);

    if (mat.isNull())
    {
      mat = matMgr.create(matName,
          Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    }
  }

  // clear everything
  mat->removeAllTechniques();

  // Automatically disable normal & parallax mapping if card cannot handle it
  // We do this rather than having a specific technique for it since it's
  // simpler.
  Ogre::GpuProgramManager &gmgr = Ogre::GpuProgramManager::getSingleton();

  if (!gmgr.isSyntaxSupported("ps_4_0") &&
      !gmgr.isSyntaxSupported("ps_3_0") &&
      !gmgr.isSyntaxSupported("ps_2_x") &&
      !gmgr.isSyntaxSupported("fp40") &&
      !gmgr.isSyntaxSupported("arbfp1"))
  {
    this->setLayerNormalMappingEnabled(false);
    this->setLayerParallaxMappingEnabled(false);
  }

  this->addTechnique(mat, _terrain, HIGH_LOD);

  // LOD
  if (this->mCompositeMapEnabled)
  {
    this->addTechnique(mat, _terrain, LOW_LOD);
    Ogre::Material::LodValueList lodValues;
    lodValues.push_back(
        Ogre::TerrainGlobalOptions::getSingleton().getCompositeMapDistance());

    mat->setLodLevels(lodValues);
    Ogre::Technique *lowLodTechnique = mat->getTechnique(1);
    lowLodTechnique->setLodIndex(1);
  }

  this->UpdateParams(mat, _terrain);

  return mat;
}

/////////////////////////////////////////////////
Ogre::MaterialPtr IgnTerrainMatGen::SM2Profile::generateForCompositeMap(
    const Ogre::Terrain *_terrain)
{
  // re-use old material if exists
  Ogre::MaterialPtr mat = _terrain->_getCompositeMapMaterial();

  if (mat.isNull())
  {
    Ogre::MaterialManager &matMgr = Ogre::MaterialManager::getSingleton();

    // it's important that the names are deterministic for a given terrain, so
    // use the terrain pointer as an ID
    const Ogre::String &matName = _terrain->getMaterialName() + "/comp";

    mat = matMgr.getByName(matName);

    if (mat.isNull())
    {
      mat = matMgr.create(matName,
          Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    }
  }

  // clear everything
  mat->removeAllTechniques();

  this->addTechnique(mat, _terrain, RENDER_COMPOSITE_MAP);

  this->UpdateParamsForCompositeMap(mat, _terrain);

  return mat;
}

/////////////////////////////////////////////////
void IgnTerrainMatGen::SM2Profile::UpdateParams(const Ogre::MaterialPtr &_mat,
                  const Ogre::Terrain *_terrain)
{
  static_cast<IgnTerrainMatGen::SM2Profile::ShaderHelperGLSL*>(
      this->mShaderGen)->updateParams(this, _mat, _terrain, false);
}

/////////////////////////////////////////////////
void IgnTerrainMatGen::SM2Profile::UpdateParamsForCompositeMap(
    const Ogre::MaterialPtr &_mat, const Ogre::Terrain *_terrain)
{
  // Only tested for Ogre 1.11 & 1.12
  static_cast<IgnTerrainMatGen::SM2Profile::ShaderHelperGLSL*>(
      this->mShaderGen)->updateParams(this, _mat, _terrain, true);
}

/////////////////////////////////////////////////
/////////////////////////////////////////////////
// GLSL Shader helper
/////////////////////////////////////////////////
/////////////////////////////////////////////////

/////////////////////////////////////////////////
Ogre::HighLevelGpuProgramPtr
IgnTerrainMatGen::SM2Profile::ShaderHelperGLSL::generateVertexProgram(
    const SM2Profile *_prof, const Ogre::Terrain *_terrain,
    TechniqueType _tt)
{
  Ogre::HighLevelGpuProgramPtr ret =
    this->createVertexProgram(_prof, _terrain, _tt);

  Ogre::StringStream sourceStr;
  this->generateVertexProgramSource(_prof, _terrain, _tt, sourceStr);

  ret->setSource(sourceStr.str());
  ret->load();
  this->defaultVpParams(_prof, _terrain, _tt, ret);

  return ret;
}

/////////////////////////////////////////////////
Ogre::HighLevelGpuProgramPtr
IgnTerrainMatGen::SM2Profile::ShaderHelperGLSL::generateFragmentProgram(
    const SM2Profile *_prof, const Ogre::Terrain *_terrain, TechniqueType _tt)
{
  Ogre::HighLevelGpuProgramPtr ret = this->createFragmentProgram(_prof,
      _terrain, _tt);

  Ogre::StringStream sourceStr;

  this->generateFragmentProgramSource(_prof, _terrain, _tt, sourceStr);

  ret->setSource(sourceStr.str());

  ret->load();

  this->defaultFpParams(_prof, _terrain, _tt, ret);

  Ogre::GpuProgramParametersSharedPtr params = ret->getDefaultParameters();
  params->setIgnoreMissingParams(false);

  Ogre::uint maxLayers = _prof->getMaxLayers(_terrain);
  Ogre::uint numBlendTextures = std::min(
      _terrain->getBlendTextureCount(maxLayers),
      _terrain->getBlendTextureCount());

  Ogre::uint numLayers = std::min(maxLayers,
      static_cast<Ogre::uint>(_terrain->getLayerCount()));

  int samplerCounter = 0;

  if (_tt == LOW_LOD)
    params->setNamedConstant("compositeMap", samplerCounter++);
  else
  {
    params->setNamedConstant("globalNormal", samplerCounter++);

    if (_terrain->getGlobalColourMapEnabled() &&
        _prof->isGlobalColourMapEnabled())
    {
      params->setNamedConstant("globalColourMap", samplerCounter++);
    }

    if (_prof->isLightmapEnabled())
      params->setNamedConstant("lightMap", samplerCounter++);

    for (Ogre::uint i = 0; i < numBlendTextures; ++i)
    {
      params->setNamedConstant("blendTex" +
          std::to_string(i), samplerCounter++);
    }

    for (Ogre::uint i = 0; i < numLayers; ++i)
    {
      params->setNamedConstant("difftex" +
          std::to_string(i), samplerCounter++);
      params->setNamedConstant("normtex" +
          std::to_string(i), samplerCounter++);
    }
  }

  if (_prof->isShadowingEnabled(_tt, _terrain))
  {
    Ogre::uint numTextures = 1;
    if (_prof->getReceiveDynamicShadowsPSSM())
    {
      numTextures = _prof->getReceiveDynamicShadowsPSSM()->getSplitCount();
    }

    for (Ogre::uint i = 0; i < numTextures; ++i)
    {
      params->setNamedConstant("shadowMap" +
          std::to_string(i), samplerCounter++);
    }
  }

  return ret;
}

/////////////////////////////////////////////////
void IgnTerrainMatGen::SM2Profile::ShaderHelperGLSL::updateParams(
    const SM2Profile *_prof, const Ogre::MaterialPtr &_mat,
    const Ogre::Terrain *_terrain, bool _compositeMap)
{
  Ogre::Pass *p = _mat->getTechnique(0)->getPass(0);

  if (_compositeMap)
  {
    this->updateVpParams(_prof, _terrain, RENDER_COMPOSITE_MAP,
        p->getVertexProgramParameters());
    this->updateFpParams(_prof, _terrain, RENDER_COMPOSITE_MAP,
        p->getFragmentProgramParameters());
  }
  else
  {
    // high lod
    this->updateVpParams(_prof, _terrain, HIGH_LOD,
        p->getVertexProgramParameters());
    this->updateFpParams(_prof, _terrain, HIGH_LOD,
        p->getFragmentProgramParameters());

    if (_prof->isCompositeMapEnabled())
    {
      // low lod
      p = _mat->getTechnique(1)->getPass(0);
      this->updateVpParams(_prof, _terrain, LOW_LOD,
          p->getVertexProgramParameters());
      this->updateFpParams(_prof, _terrain, LOW_LOD,
          p->getFragmentProgramParameters());
    }
  }
}

/////////////////////////////////////////////////
void IgnTerrainMatGen::SM2Profile::ShaderHelperGLSL::
generateVertexProgramSource(const SM2Profile *_prof,
    const Ogre::Terrain* _terrain, TechniqueType _tt,
    Ogre::StringStream &_outStream)
{
  this->generateVpHeader(_prof, _terrain, _tt, _outStream);

  if (_tt != LOW_LOD)
  {
    unsigned int maxLayers = _prof->getMaxLayers(_terrain);
    unsigned int numLayers = std::min(maxLayers,
        static_cast<unsigned int>(_terrain->getLayerCount()));

    for (unsigned int i = 0; i < numLayers; ++i)
      this->generateVpLayer(_prof, _terrain, _tt, i, _outStream);
  }

  this->generateVpFooter(_prof, _terrain, _tt, _outStream);
}

/////////////////////////////////////////////////
// This method is identical to
// TerrainMaterialGeneratorA::SM2Profile::ShaderHelperGLSL::generateVpHeader()
// but is needed because generateVpDynamicShadowsParams() is not declared
// virtual.
void IgnTerrainMatGen::SM2Profile::ShaderHelperGLSL::generateVpHeader(
    const SM2Profile *_prof, const Ogre::Terrain *_terrain,
    TechniqueType _tt, Ogre::StringStream &_outStream)
{
  bool compression = false;

  _outStream << "#version " << this->glslVersion << "\n\n";

#if OGRE_VERSION_MAJOR >= 1 && OGRE_VERSION_MINOR >= 8
  compression = _terrain->_getUseVertexCompression() &&
                _tt != RENDER_COMPOSITE_MAP;

  if (compression)
  {
    // The parameter "in vec4 vertex;" is automatically bound by OGRE.
    // The parameter "in vec4 uv0'" is automatically bound by OGRE.
    _outStream << this->vpInStr << " vec4 vertex;\n"
               << this->vpInStr << " vec4 uv0;\n";
  }
  else
#endif
  {
    // The parameter "in vec4 vertex;" is automatically bound by OGRE.
    // The parameter "in vec4 uv0'" is automatically bound by OGRE.
    _outStream << this->vpInStr << " vec4 vertex;\n"
               << this->vpInStr << " vec4 uv0;\n";
  }

  if (_tt != RENDER_COMPOSITE_MAP)
    // The parameter "in vec4 uv1'" is automatically bound by OGRE.
    _outStream << this->vpInStr << " vec4 uv1;\n";

  _outStream <<
    "uniform mat4 worldMatrix;\n"
    "uniform mat4 viewProjMatrix;\n"
    "uniform vec2 lodMorph;\n";

  if (compression)
  {
    _outStream <<
      "uniform mat4  posIndexToObjectSpace;\n"
      "uniform float baseUVScale;\n";
  }


  // uv multipliers
  unsigned int maxLayers = _prof->getMaxLayers(_terrain);
  unsigned int numLayers = std::min(maxLayers,
      static_cast<unsigned int>(_terrain->getLayerCount()));

  unsigned int numUVMultipliers = (numLayers / 4);

  if (numLayers % 4)
    ++numUVMultipliers;

  for (unsigned int i = 0; i < numUVMultipliers; ++i)
    _outStream << "uniform vec4 uvMul" << i << ";\n";

  _outStream <<
    this->vpOutStr << " vec4 position;\n";

  unsigned int texCoordSet = 1;
  _outStream << this->vpOutStr << " vec4 uvMisc;\n";

  // layer UV's premultiplied, packed as xy/zw
  unsigned int numUVSets = numLayers / 2;

  if (numLayers % 2)
    ++numUVSets;

  if (_tt != LOW_LOD)
  {
    for (unsigned int i = 0; i < numUVSets; ++i)
    {
      _outStream << this->vpOutStr << " vec4 layerUV" << i << ";\n";
    }
  }

  if (_prof->getParent()->getDebugLevel() && _tt != RENDER_COMPOSITE_MAP)
  {
    _outStream << this->vpOutStr << " vec2 lodInfo;\n";
  }

  bool fog = _terrain->getSceneManager()->getFogMode() != Ogre::FOG_NONE &&
             _tt != RENDER_COMPOSITE_MAP;

  if (fog)
  {
    _outStream <<
      "uniform vec4 fogParams;\n"
      << this->vpOutStr << " float fogVal;\n";
  }

  if (_prof->isShadowingEnabled(_tt, _terrain))
  {
    texCoordSet = this->generateVpDynamicShadowsParams(texCoordSet, _prof,
        _terrain, _tt, _outStream);
  }

  // check we haven't exceeded texture coordinates
  if (texCoordSet > 8)
  {
    OGRE_EXCEPT(Ogre::Exception::ERR_INVALIDPARAMS,
        "Requested options require too many texture coordinate sets! "
        "Try reducing the number of layers.",
        __FUNCTION__);
  }

  _outStream << "void main()\n"
             << "{\n";

  if (compression)
  {
    _outStream
      << "  vec4 pos = posIndexToObjectSpace * "
      << "vec4(vertex.x, vertex.y, uv0.x, 1.0);\n"

      << "  vec2 uv = vec2(vertex.x * baseUVScale, 1.0 - "
      << "(vertex.y * baseUVScale));\n";
  }
  else
  {
    _outStream
      << "  vec4 pos = vertex;\n"
      << "  vec2 uv = vec2(uv0.x, uv0.y);\n";
  }

  _outStream << "  vec4 worldPos = worldMatrix * pos;\n";
  _outStream << "  position = pos;\n";

  if (_tt != RENDER_COMPOSITE_MAP)
  {
    // determine whether to apply the LOD morph to this vertex
    // we store the deltas against all vertices so we only want to apply
    // the morph to the ones which would disappear. The target LOD which is
    // being morphed to is stored in lodMorph.y, and the LOD at which
    // the vertex should be morphed is stored in uv.w. If we subtract
    // the former from the latter, and arrange to only morph if the
    // result is negative (it will only be -1 in fact, since after that
    // the vertex will never be indexed), we will achieve our aim.
    // sign(vertexLOD - targetLOD) == -1 is to morph
    _outStream <<
      "  float toMorph = -min(0.0, sign(uv1.y - lodMorph.y));\n";

    // this will either be 1 (morph) or 0 (don't morph)
    if (_prof->getParent()->getDebugLevel())
    {
      // x == LOD level (-1 since value is target level, we want to
      // display actual)
      _outStream << "lodInfo.x = (lodMorph.y - 1.0) / "
                 << _terrain->getNumLodLevels() << ";\n";

      // y == LOD morph
      _outStream << "lodInfo.y = toMorph * lodMorph.x;\n";
    }

    // morph
    switch (_terrain->getAlignment())
    {
      case Ogre::Terrain::ALIGN_X_Y:
        _outStream << "  worldPos.z += uv1.x * toMorph * lodMorph.x;\n";
        break;
      case Ogre::Terrain::ALIGN_X_Z:
        _outStream << "  worldPos.y += uv1.x * toMorph * lodMorph.x;\n";
        break;
      case Ogre::Terrain::ALIGN_Y_Z:
        _outStream << "  worldPos.x += uv1.x * toMorph * lodMorph.x;\n";
        break;
      default:
        ignerr << "Invalid alignment\n";
    };
  }

  // generate UVs
  if (_tt != LOW_LOD)
  {
    for (unsigned int i = 0; i < numUVSets; ++i)
    {
      unsigned int layer  =  i * 2;
      unsigned int uvMulIdx = layer / 4;

      _outStream << "  layerUV" << i << ".xy = " << " uv.xy * uvMul"
                 << uvMulIdx << "." << this->GetChannel(layer) << ";\n";
      _outStream << "  layerUV" << i << ".zw = " << " uv.xy * uvMul"
                 << uvMulIdx << "." << this->GetChannel(layer+1) << ";\n";
    }
  }
}

/////////////////////////////////////////////////
// This method is identical to
// TerrainMaterialGeneratorA::SM2Profile::ShaderHelperGLSL::generateVpFooter()
// but is needed because generateVpDynamicShadows() is not declared virtual.
void IgnTerrainMatGen::SM2Profile::ShaderHelperGLSL::generateVpFooter(
    const SM2Profile *_prof, const Ogre::Terrain *_terrain,
    TechniqueType _tt, Ogre::StringStream &_outStream)
{
  _outStream << "  gl_Position = viewProjMatrix * worldPos;\n"
             << "  uvMisc.xy = uv.xy;\n";

  bool fog = _terrain->getSceneManager()->getFogMode() != Ogre::FOG_NONE &&
             _tt != RENDER_COMPOSITE_MAP;
  if (fog)
  {
    if (_terrain->getSceneManager()->getFogMode() == Ogre::FOG_LINEAR)
    {
      _outStream <<
        "  fogVal = clamp((oPos.z - fogParams.y) * fogParams.w, 0.0, 1.0);\n";
    }
    else
    {
      _outStream <<
        "  fogVal = 1 - clamp(1 / (exp(oPos.z * fogParams.x)), 0.0, 1.0);\n";
    }
  }

  if (_prof->isShadowingEnabled(_tt, _terrain))
    this->generateVpDynamicShadows(_prof, _terrain, _tt, _outStream);

  _outStream << "}\n";
}

/////////////////////////////////////////////////
void
IgnTerrainMatGen::SM2Profile::ShaderHelperGLSL::generateVpDynamicShadows(
    const SM2Profile *_prof, const Ogre::Terrain * /*_terrain*/,
    TechniqueType /*_tt*/, Ogre::StringStream &_outStream)
{
  unsigned int numTextures = 1;

  if (_prof->getReceiveDynamicShadowsPSSM())
  {
    numTextures = _prof->getReceiveDynamicShadowsPSSM()->getSplitCount();
  }

  // Calculate the position of vertex in light space
  for (unsigned int i = 0; i < numTextures; ++i)
  {
    _outStream << "  lightSpacePos" << i << " = texViewProjMatrix"
               << i << " * worldPos;\n";

    // Don't linearize depth range: RTSS PSSM implementation uses
    // view-space depth
    // if (prof->getReceiveDynamicShadowsDepth())
    // {
    //   // make linear
    //   outStream << "lightSpacePos" << i << ".z = (lightSpacePos" << i
    //             << ".z - depthRange" << i << ".x) * depthRange" << i
    //             << ".w;\n";
    // }
  }

  if (_prof->getReceiveDynamicShadowsPSSM())
  {
    _outStream << "  // pass cam depth\n  uvMisc.z = gl_Position.z;\n";
  }
}

/////////////////////////////////////////////////
void IgnTerrainMatGen::SM2Profile::ShaderHelperGLSL::defaultVpParams(
    const SM2Profile *_prof, const Ogre::Terrain *_terrain,
    TechniqueType _tt, const Ogre::HighLevelGpuProgramPtr &_prog)
{
  Ogre::GpuProgramParametersSharedPtr params = _prog->getDefaultParameters();
  params->setIgnoreMissingParams(true);

  params->setNamedAutoConstant("worldMatrix",
      Ogre::GpuProgramParameters::ACT_WORLD_MATRIX);

  params->setNamedAutoConstant("viewProjMatrix",
      Ogre::GpuProgramParameters::ACT_VIEWPROJ_MATRIX);

  params->setNamedAutoConstant("lodMorph",
      Ogre::GpuProgramParameters::ACT_CUSTOM,
      Ogre::Terrain::LOD_MORPH_CUSTOM_PARAM);

  params->setNamedAutoConstant("fogParams",
      Ogre::GpuProgramParameters::ACT_FOG_PARAMS);

  if (_prof->isShadowingEnabled(_tt, _terrain))
  {
    unsigned int numTextures = 1;
    if (_prof->getReceiveDynamicShadowsPSSM())
    {
      numTextures = _prof->getReceiveDynamicShadowsPSSM()->getSplitCount();
    }
    for (unsigned int i = 0; i < numTextures; ++i)
    {
      params->setNamedAutoConstant("texViewProjMatrix" +
          Ogre::StringConverter::toString(i),
          Ogre::GpuProgramParameters::ACT_TEXTURE_VIEWPROJ_MATRIX, i);

      // Don't add depth range params
      // if (prof->getReceiveDynamicShadowsDepth())
      // {
      //   params->setNamedAutoConstant("depthRange" +
      //       Ogre::StringConverter::toString(i),
      //       Ogre::GpuProgramParameters::ACT_SHADOW_SCENE_DEPTH_RANGE, i);
      // }
    }
  }

#if OGRE_VERSION_MAJOR >= 1 && OGRE_VERSION_MINOR >= 8
  if (_terrain->_getUseVertexCompression() && _tt != RENDER_COMPOSITE_MAP)
  {
    Ogre::Matrix4 posIndexToObjectSpace;
    _terrain->getPointTransform(&posIndexToObjectSpace);
    params->setNamedConstant("posIndexToObjectSpace", posIndexToObjectSpace);
  }
#endif
}

/////////////////////////////////////////////////
unsigned int IgnTerrainMatGen::SM2Profile::ShaderHelperGLSL::
generateVpDynamicShadowsParams(unsigned int _texCoord, const SM2Profile *_prof,
    const Ogre::Terrain * /*_terrain*/, TechniqueType /*_tt*/,
    Ogre::StringStream &_outStream)
{
  // out semantics & params
  unsigned int numTextures = 1;

  if (_prof->getReceiveDynamicShadowsPSSM())
  {
    numTextures = _prof->getReceiveDynamicShadowsPSSM()->getSplitCount();
  }

  for (unsigned int i = 0; i < numTextures; ++i)
  {
    _outStream << this->vpOutStr << " vec4 lightSpacePos" << i
               << ";\n" << "uniform mat4 texViewProjMatrix" << i << ";\n";

    // Don't add depth range params
    // if (prof->getReceiveDynamicShadowsDepth())
    // {
    //   _outStream << ", uniform float4 depthRange" << i
    //             << " // x = min, y = max, z = range, w = 1/range\n";
    // }
  }

  return _texCoord;
}

/////////////////////////////////////////////////
void IgnTerrainMatGen::SM2Profile::ShaderHelperGLSL::generateFpHeader(
    const SM2Profile *_prof, const Ogre::Terrain *_terrain,
    TechniqueType _tt, Ogre::StringStream &_outStream)
{
  _outStream << "#version " << this->glslVersion << "\n\n";

  _outStream <<
    "vec4 expand(vec4 v)\n"
    "{\n"
    "  return v * 2 - 1;\n"
    "}\n\n";

  _outStream <<
    "vec4 lit(float NdotL, float NdotH, float m)\n"
    "{\n"
    "  float specular = (NdotL > 0) ? pow(max(0.0, NdotH), m) : 0.0;\n"
    "  return vec4(1.0, max(0.0, NdotL), specular, 1.0);\n"
    "}\n";

  if (_prof->isShadowingEnabled(_tt, _terrain))
    this->generateFpDynamicShadowsHelpers(_prof, _terrain, _tt, _outStream);

  _outStream <<
    this->fpInStr << " vec4 position;\n";

  Ogre::uint texCoordSet = 1;
  _outStream << this->fpInStr << " vec4 uvMisc;\n";

  // UV's premultiplied, packed as xy/zw
  Ogre::uint maxLayers = _prof->getMaxLayers(_terrain);
  Ogre::uint numBlendTextures = std::min(
      _terrain->getBlendTextureCount(maxLayers),
      _terrain->getBlendTextureCount());
  Ogre::uint numLayers = std::min(maxLayers,
      static_cast<Ogre::uint>(_terrain->getLayerCount()));

  Ogre::uint numUVSets = numLayers / 2;

  if (numLayers % 2)
    ++numUVSets;

  if (_tt != LOW_LOD)
  {
    for (Ogre::uint i = 0; i < numUVSets; ++i)
    {
      _outStream <<
        this->fpInStr << " vec4 layerUV" << i << ";\n";
    }
  }

  if (_prof->getParent()->getDebugLevel() && _tt != RENDER_COMPOSITE_MAP)
  {
    _outStream << this->fpInStr << " vec2 lodInfo;\n";
  }

  bool fog = _terrain->getSceneManager()->getFogMode() != Ogre::FOG_NONE &&
             _tt != RENDER_COMPOSITE_MAP;

  if (fog)
  {
    _outStream <<
      "uniform vec3 fogColour;\n"
      << this->fpInStr << " float fogVal;\n";
  }

  Ogre::uint currentSamplerIdx = 0;

  _outStream <<
    // Only 1 light supported in this version
    // deferred shading profile / generator later, ok? :)
    "uniform vec3 ambient;\n"
    "uniform vec4 lightPosObjSpace;\n"
    "uniform vec3 lightDiffuseColour;\n"
    "uniform vec3 lightSpecularColour;\n"
    "uniform vec3 eyePosObjSpace;\n"
    // pack scale, bias and specular
    "uniform vec4 scaleBiasSpecular;\n";

  if (_tt == LOW_LOD)
  {
    // single composite map covers all the others below
    _outStream << "uniform sampler2D compositeMap;\n";
  }
  else
  {
    _outStream << "uniform sampler2D globalNormal;\n";

    if (_terrain->getGlobalColourMapEnabled() &&
        _prof->isGlobalColourMapEnabled())
    {
      _outStream << "uniform sampler2D globalColourMap;\n";
    }

    if (_prof->isLightmapEnabled())
    {
      _outStream << "uniform sampler2D lightMap;\n";
    }

    // Blend textures - sampler definitions
    for (Ogre::uint i = 0; i < numBlendTextures; ++i)
    {
      _outStream << "uniform sampler2D blendTex" << i << ";\n";
    }

    // Layer textures - sampler definitions & UV multipliers
    for (Ogre::uint i = 0; i < numLayers; ++i)
    {
      _outStream << "uniform sampler2D difftex" << i << ";\n";
      _outStream << "uniform sampler2D normtex" << i << ";\n";
    }
  }

  if (_prof->isShadowingEnabled(_tt, _terrain))
  {
    this->generateFpDynamicShadowsParams(&texCoordSet, &currentSamplerIdx,
        _prof, _terrain, _tt, _outStream);
  }

  // check we haven't exceeded samplers
  if (currentSamplerIdx > 16)
  {
    OGRE_EXCEPT(Ogre::Exception::ERR_INVALIDPARAMS,
        "Requested options require too many texture samplers! "
        "Try reducing the number of layers.", __FUNCTION__);
  }

  std::string outputColTypeStr = "vec4";
  if (this->glslVersion != "120")
  {
    _outStream << "out vec4 outputCol;\n";
    outputColTypeStr = "";
  }

  _outStream <<
    "void main()\n"
    "{\n"
    "  float shadow = 1.0;\n"
    "  vec2 uv = uvMisc.xy;\n"
    "  " << outputColTypeStr << " outputCol = vec4(0.0, 0.0, 0.0, 1.0);\n";

  if (_tt != LOW_LOD)
  {
    // global normal
    _outStream << "  vec3 normal = expand("
               << this->textureStr << "(globalNormal, uv)).xyz;\n";
  }

  _outStream <<
    "  vec3 lightDir =\n"
    "    lightPosObjSpace.xyz -  (position.xyz * lightPosObjSpace.w);\n"
    "  vec3 eyeDir = eyePosObjSpace - position.xyz;\n"

    // set up accumulation areas
    "  vec3 diffuse = vec3(0.0, 0.0, 0.0);\n"
    "  float specular = 0.0;\n";

  if (_tt == LOW_LOD)
  {
    // we just do a single calculation from composite map
    _outStream << "  vec4 composite = " << this->textureStr
               << "(compositeMap, uv);\n  diffuse = composite.xyz;\n";
    // TODO(anyone) - specular; we'll need normals for this!
  }
  else
  {
    // set up the blend values
    for (Ogre::uint i = 0; i < numBlendTextures; ++i)
    {
      _outStream << "  vec4 blendTexVal" << i << " = "
                 << this->textureStr << "(blendTex" << i
                 << ", uv);\n";
    }

    if (_prof->isLayerNormalMappingEnabled())
    {
      // derive the tangent space basis
      // we do this in the pixel shader because we don't have per-vertex normals
      // because of the LOD, we use a normal map
      // tangent is always +x or -z in object space depending on alignment
      switch (_terrain->getAlignment())
      {
        case Ogre::Terrain::ALIGN_X_Y:
        case Ogre::Terrain::ALIGN_X_Z:
          _outStream << "  vec3 tangent = vec3(1.0, 0.0, 0.0);\n";
          break;
        case Ogre::Terrain::ALIGN_Y_Z:
          _outStream << "  vec3 tangent = vec3(0.0, 0.0, -1.0);\n";
          break;
        default:
          ignerr << "Invalid terrain alignment\n";
          break;
      };

      _outStream << "  vec3 binormal = normalize(cross(tangent, normal));\n";
      // note, now we need to re-cross to derive tangent again because it
      // wasn't orthonormal
      _outStream << "  tangent = normalize(cross(normal, binormal));\n";
      // derive final matrix
      /*_outStream << "  mat3 TBN = mat3(tangent.x, tangent.y, tangent.z,"
                                      "binormal.x, binormal.y, binormal.z,"
                                      "normal.x, normal.y, normal.z);\n";
                                      */

      // set up lighting result placeholders for interpolation
      _outStream << "  vec4 litRes, litResLayer;\n";
      _outStream << "  vec3 TSlightDir, TSeyeDir, TShalfAngle, TSnormal;\n";
      if (_prof->isLayerParallaxMappingEnabled())
        _outStream << "  float displacement;\n";
      // move
      _outStream << "  TSlightDir = normalize(vec3(dot(tangent, lightDir),"
                                              "dot(binormal, lightDir),"
                                              "dot(normal, lightDir)));\n";
      _outStream << "  TSeyeDir = normalize(vec3(dot(tangent, eyeDir),"
                                           "dot(binormal, eyeDir),"
                                           "dot(normal, eyeDir)));\n";
    }
    else
    {
      // simple per-pixel lighting with no normal mapping
      _outStream << "  lightDir = normalize(lightDir);\n";
      _outStream << "  eyeDir = normalize(eyeDir);\n";
      _outStream << "  vec3 halfAngle = normalize(lightDir + eyeDir);\n";

       _outStream << "  vec4 litRes = lit(dot(lightDir, normal), "
         "dot(halfAngle, normal), scaleBiasSpecular.z);\n";
    }
  }
}

/////////////////////////////////////////////////
void
IgnTerrainMatGen::SM2Profile::ShaderHelperGLSL::generateFpDynamicShadowsParams(
    Ogre::uint *_texCoord, Ogre::uint *_sampler, const SM2Profile *_prof,
    const Ogre::Terrain * /*_terrain*/, TechniqueType _tt,
    Ogre::StringStream &_outStream)
{
  if (_tt == HIGH_LOD)
    this->mShadowSamplerStartHi = *_sampler;
  else if (_tt == LOW_LOD)
    this->mShadowSamplerStartLo = *_sampler;

  // in semantics & params
  Ogre::uint numTextures = 1;
  if (_prof->getReceiveDynamicShadowsPSSM())
  {
    numTextures = _prof->getReceiveDynamicShadowsPSSM()->getSplitCount();
    _outStream << "uniform vec4 pssmSplitPoints;\n";
  }

  for (Ogre::uint i = 0; i < numTextures; ++i)
  {
    _outStream << this->fpInStr <<
      " vec4 lightSpacePos" << i << ";\n" <<
      "uniform sampler2D shadowMap" << i << ";\n";

    *_sampler = *_sampler + 1;
    *_texCoord = *_texCoord + 1;

    if (_prof->getReceiveDynamicShadowsDepth())
    {
      _outStream <<
        "uniform float inverseShadowmapSize" << i << ";\n";
    }
  }
}

/////////////////////////////////////////////////
void IgnTerrainMatGen::SM2Profile::ShaderHelperGLSL::generateFpLayer(
    const SM2Profile *_prof, const Ogre::Terrain * /*_terrain*/,
    TechniqueType _tt, Ogre::uint _layer,
    Ogre::StringStream &_outStream)
{
  Ogre::uint uvIdx = _layer / 2;
  Ogre::String uvChannels = (_layer % 2) ? ".zw" : ".xy";
  Ogre::uint blendIdx = (_layer-1) / 4;
  Ogre::String blendChannel = this->GetChannel(_layer-1);
  Ogre::String blendWeightStr = Ogre::String("blendTexVal") +
    Ogre::StringConverter::toString(blendIdx) + "." + blendChannel;

  // generate early-out conditional
  // Disable - causing some issues even when trying to force the use of texldd
  //   if (layer && prof->_isSM3Available())
  //   _outStream << "  if (" << blendWeightStr << " > 0.0003)\n  {\n";

  // generate UV
  _outStream << "  vec2 uv" << _layer << " = layerUV" << uvIdx
             << uvChannels << ";\n";

  // calculate lighting here if normal mapping
  if (_prof->isLayerNormalMappingEnabled())
  {
    if (_prof->isLayerParallaxMappingEnabled() && _tt != RENDER_COMPOSITE_MAP)
    {
      // modify UV - note we have to sample an extra time
      _outStream << "  displacement = " << this->textureStr
                 << "(normtex" << _layer << ", uv" << _layer << ").w\n"
        "   * scaleBiasSpecular.x + scaleBiasSpecular.y;\n";
      _outStream << "  uv" << _layer << " += TSeyeDir.xy * displacement;\n";
    }

    // access TS normal map
    _outStream << "  TSnormal = expand(" << this->textureStr
               << "(normtex" << _layer << ", uv" << _layer << ")).xyz;\n";
    _outStream << "  TShalfAngle = normalize(TSlightDir + TSeyeDir);\n";

    _outStream << "  litResLayer = lit(dot(TSlightDir, TSnormal), "
      "dot(TShalfAngle, TSnormal), scaleBiasSpecular.z);\n";

    if (!_layer)
      _outStream << "  litRes = litResLayer;\n";
    else
      _outStream << "  litRes = mix(litRes, litResLayer, "
                 << blendWeightStr << ");\n";
  }

  // sample diffuse texture
  _outStream << "  vec4 diffuseSpecTex" << _layer << " = "
             << this->textureStr << "(difftex" << _layer << ", uv"
             << _layer << ");\n";

  // apply to common
  if (!_layer)
  {
    _outStream << "  diffuse = diffuseSpecTex0.xyz;\n";
    if (_prof->isLayerSpecularMappingEnabled())
      _outStream << "  specular = diffuseSpecTex0.w;\n";
  }
  else
  {
     _outStream << "  diffuse = mix(diffuse, diffuseSpecTex" << _layer
                << ".xyz, " << blendWeightStr << ");\n";

    if (_prof->isLayerSpecularMappingEnabled())
    {
       _outStream << "  specular = mix(specular, diffuseSpecTex" << _layer
                  << ".w, " << blendWeightStr << ");\n";
    }
  }

  // End early-out
  // Disable - causing some issues even when trying to force the use of texldd
  //   if (layer && prof->_isSM3Available())
  //   _outStream << "  } // early-out blend value\n";
}

/////////////////////////////////////////////////
void IgnTerrainMatGen::SM2Profile::ShaderHelperGLSL::generateFpFooter(
    const SM2Profile *_prof, const Ogre::Terrain *_terrain,
    TechniqueType _tt, Ogre::StringStream &_outStream)
{
  if (_tt == LOW_LOD)
  {
    if (_prof->isShadowingEnabled(_tt, _terrain))
    {
      this->generateFpDynamicShadows(_prof, _terrain, _tt, _outStream);
      _outStream << "  outputCol.xyz = diffuse * rtshadow;\n";
    }
    else
    {
      _outStream << "  outputCol.xyz = diffuse;\n";
    }
  }
  else
  {
    if (_terrain->getGlobalColourMapEnabled() &&
        _prof->isGlobalColourMapEnabled())
    {
      // sample colour map and apply to diffuse
      _outStream << "  diffuse *= " << this->textureStr
                 << "(globalColourMap, uv).xyz;\n";
    }

    if (_prof->isLightmapEnabled())
    {
      // sample lightmap
      _outStream << "  shadow = " << this->textureStr
                 << "(lightMap, uv).x;\n";
    }

    if (_prof->isShadowingEnabled(_tt, _terrain))
    {
      this->generateFpDynamicShadows(_prof, _terrain, _tt, _outStream);
    }

    // diffuse lighting
    _outStream << "  outputCol.xyz += ambient * diffuse + litRes.y * "
                  "lightDiffuseColour * diffuse * shadow;\n";

    // specular default
    if (!_prof->isLayerSpecularMappingEnabled())
      _outStream << "  specular = 1.0;\n";

    if (_tt == RENDER_COMPOSITE_MAP)
    {
      // Lighting embedded in alpha
      _outStream << "  outputCol.w = shadow;\n";
    }
    else
    {
      // Apply specular
      _outStream << "  outputCol.xyz += litRes.z * lightSpecularColour * "
                    "specular * shadow;\n";

      if (_prof->getParent()->getDebugLevel())
      {
        _outStream << "  outputCol.xy += lodInfo.xy;\n";
      }
    }
  }

  bool fog = _terrain->getSceneManager()->getFogMode() != Ogre::FOG_NONE &&
             _tt != RENDER_COMPOSITE_MAP;
  if (fog)
  {
    _outStream << "  outputCol.xyz = mix(outputCol.xyz, fogColour, fogVal);\n";
  }

  if (this->glslVersion == "120")
    _outStream << "  gl_FragColor = outputCol;\n";

  // Final return
  _outStream << "\n}\n";
}

/////////////////////////////////////////////////
void
IgnTerrainMatGen::SM2Profile::ShaderHelperGLSL::generateFpDynamicShadowsHelpers(
    const SM2Profile *_prof, const Ogre::Terrain * /*_terrain*/,
    TechniqueType /*_tt*/, Ogre::StringStream &_outStream)
{
  // TODO(anyone) make filtering configurable
  _outStream <<
    "// Simple PCF\n"
    "// Number of samples in one dimension (square for total samples)\n"
    "#define NUM_SHADOW_SAMPLES_1D 2.0\n"
    "#define SHADOW_FILTER_SCALE 1.0\n"

    "#define SHADOW_SAMPLES NUM_SHADOW_SAMPLES_1D*NUM_SHADOW_SAMPLES_1D\n"

    "vec4 offsetSample(vec4 uv, vec2 offset, float invMapSize)\n"
    "{\n"
    "  return vec4(uv.xy + offset * invMapSize * uv.w, uv.z, uv.w);\n"
    "}\n";

  if (_prof->getReceiveDynamicShadowsDepth())
  {
    _outStream <<
      "float calcDepthShadow(sampler2D shadowMap, vec4 uv, "
      "float invShadowMapSize)\n"
      "{\n"
      "  // 4-sample PCF\n"
      "  float shadow = 0.0;\n"
      "  float offset = (NUM_SHADOW_SAMPLES_1D/2.0 - 0.5) *SHADOW_FILTER_SCALE;"
      "\n"
      "  for (float y = -offset; y <= offset; y += SHADOW_FILTER_SCALE)\n"
      "    for (float x = -offset; x <= offset; x += SHADOW_FILTER_SCALE)\n"
      "    {\n"
      "      vec4 newUV = offsetSample(uv, vec2(x, y), invShadowMapSize);\n"
      "      // manually project and assign derivatives\n"
      "      // to avoid gradient issues inside loops\n"
      "      newUV = newUV / newUV.w;\n";
      // The following line used to be:
      // "      float depth = tex2d(shadowMap, newUV.xy).x;\n"
    if (this->glslVersion == "120")
      _outStream <<
          "      float depth = texture2D(shadowMap, newUV.xy).x;\n";
    else
    {
      _outStream <<
          "      float depth = textureGrad(shadowMap, newUV.xy, "
          " vec2(1.0, 1.0), vec2(1.0, 1.0)).x;\n";
    }
    _outStream <<
      // "      if (depth >= 1.0 || depth >= uv.z)\n"
      "      if (depth >= 1.0 || depth >= newUV.z)\n"
      "        shadow += 1.0;\n"
      "    }\n"
      "  shadow /= (SHADOW_SAMPLES); \n"
      "  return shadow;\n"
      "}\n";
  }
  else
  {
    _outStream <<
      "float calcSimpleShadow(sampler2D shadowMap, vec4 shadowMapPos)\n"
      "{\n"
      "  return " << this->textureStr
                  << "Proj(shadowMap, shadowMapPos).x;\n"
      "}\n";
  }

  if (_prof->getReceiveDynamicShadowsPSSM())
  {
    Ogre::uint numTextures =
      _prof->getReceiveDynamicShadowsPSSM()->getSplitCount();

    if (_prof->getReceiveDynamicShadowsDepth())
    {
      _outStream << "float calcPSSMDepthShadow(";
    }
    else
    {
      _outStream << "float calcPSSMSimpleShadow(";
    }

    _outStream << "\n  ";

    for (Ogre::uint i = 0; i < numTextures; ++i)
      _outStream << "sampler2D shadowMap" << i << ", ";

    _outStream << "\n  ";

    for (Ogre::uint i = 0; i < numTextures; ++i)
      _outStream << "vec4 lsPos" << i << ", ";

    if (_prof->getReceiveDynamicShadowsDepth())
    {
      _outStream << "\n  ";
      for (Ogre::uint i = 0; i < numTextures; ++i)
        _outStream << "float invShadowmapSize" << i << ", ";
    }

    _outStream << "\n"
      "  vec4 pssmSplitPoints, float camDepth)\n"
      "{\n"
      "  float shadow = 1.0;\n"
      "  // calculate shadow\n";

    for (Ogre::uint i = 0; i < numTextures; ++i)
    {
      if (!i)
      {
        _outStream << "  if (camDepth <= pssmSplitPoints."
          << this->GetChannel(i) << ")\n";
      }
      else if (i < numTextures-1)
      {
        _outStream << "  else if (camDepth <= pssmSplitPoints."
          << this->GetChannel(i) << ")\n";
      }
      else
        _outStream << "  else\n";

      _outStream << "  {\n";

      if (_prof->getReceiveDynamicShadowsDepth())
      {
        _outStream << "    shadow = calcDepthShadow(shadowMap" << i
          << ", lsPos" << i << ", invShadowmapSize" << i << ");\n";
      }
      else
      {
        _outStream << "    shadow = calcSimpleShadow(shadowMap" << i
          << ", lsPos" << i << ");\n";
      }
      _outStream << "  }\n";
    }

    _outStream << "  return shadow;\n"
                  "}\n\n\n";
  }
}

/////////////////////////////////////////////////
void IgnTerrainMatGen::SM2Profile::ShaderHelperGLSL::generateFpDynamicShadows(
    const SM2Profile *_prof, const Ogre::Terrain * /*_terrain*/,
    TechniqueType /*_tt*/, Ogre::StringStream &_outStream)
{
  if (_prof->getReceiveDynamicShadowsPSSM())
  {
    Ogre::uint numTextures =
      _prof->getReceiveDynamicShadowsPSSM()->getSplitCount();

    _outStream << "  float camDepth = uvMisc.z;\n";

    if (_prof->getReceiveDynamicShadowsDepth())
    {
      _outStream << "  float rtshadow = calcPSSMDepthShadow(";
    }
    else
    {
      _outStream << "  float rtshadow = calcPSSMSimpleShadow(";
    }

    for (Ogre::uint i = 0; i < numTextures; ++i)
      _outStream << "shadowMap" << i << ", ";

    _outStream << "\n    ";

    for (Ogre::uint i = 0; i < numTextures; ++i)
      _outStream << "lightSpacePos" << i << ", ";

    if (_prof->getReceiveDynamicShadowsDepth())
    {
      _outStream << "\n    ";

      for (Ogre::uint i = 0; i < numTextures; ++i)
        _outStream << "inverseShadowmapSize" << i << ", ";
    }
    _outStream << "\n" <<
      "    pssmSplitPoints, camDepth);\n";
  }
  else
  {
    if (_prof->getReceiveDynamicShadowsDepth())
    {
      _outStream <<
        "  float rtshadow = calcDepthShadow(shadowMap0, lightSpacePos0, "
        "inverseShadowmapSize0);";
    }
    else
    {
      _outStream <<
        "  float rtshadow = calcSimpleShadow(shadowMap0, lightSpacePos0);";
    }
  }

  _outStream << "  shadow = rtshadow;//min(shadow, rtshadow);\n";
}

/////////////////////////////////////////////////
void
IgnTerrainMatGen::SM2Profile::ShaderHelperGLSL::generateFragmentProgramSource(
    const SM2Profile *_prof, const Ogre::Terrain *_terrain,
    TechniqueType _tt, Ogre::StringStream &_outStream)
{
  this->generateFpHeader(_prof, _terrain, _tt, _outStream);

  if (_tt != LOW_LOD)
  {
    Ogre::uint maxLayers = _prof->getMaxLayers(_terrain);
    Ogre::uint numLayers = std::min(maxLayers,
        static_cast<Ogre::uint>(_terrain->getLayerCount()));

    for (Ogre::uint i = 0; i < numLayers; ++i)
      this->generateFpLayer(_prof, _terrain, _tt, i, _outStream);
  }

  this->generateFpFooter(_prof, _terrain, _tt, _outStream);
}

/////////////////////////////////////////////////
void IgnTerrainMatGen::SM2Profile::ShaderHelperGLSL::updateVpParams(
    const SM2Profile *_prof, const Ogre::Terrain *_terrain,
#if OGRE_VERSION_MAJOR >= 1 && OGRE_VERSION_MINOR >= 8
    TechniqueType _tt,
#else
    TechniqueType /*_tt*/,
#endif
    const Ogre::GpuProgramParametersSharedPtr &_params)
{
  _params->setIgnoreMissingParams(true);
  Ogre::uint maxLayers = _prof->getMaxLayers(_terrain);
  Ogre::uint numLayers = std::min(maxLayers,
      static_cast<Ogre::uint>(_terrain->getLayerCount()));

  Ogre::uint numUVMul = numLayers / 4;

  if (numLayers % 4)
    ++numUVMul;

  for (Ogre::uint i = 0; i < numUVMul; ++i)
  {
    Ogre::Vector4 uvMul(
        _terrain->getLayerUVMultiplier(i * 4),
        _terrain->getLayerUVMultiplier(i * 4 + 1),
        _terrain->getLayerUVMultiplier(i * 4 + 2),
        _terrain->getLayerUVMultiplier(i * 4 + 3));
    _params->setNamedConstant("uvMul" +
        Ogre::StringConverter::toString(i), uvMul);
  }

#if OGRE_VERSION_MAJOR >= 1 && OGRE_VERSION_MINOR >= 8
  if (_terrain->_getUseVertexCompression() && _tt != RENDER_COMPOSITE_MAP)
  {
    Ogre::Real baseUVScale = 1.0f / (_terrain->getSize() - 1);
    _params->setNamedConstant("baseUVScale", baseUVScale);
  }
#endif
}

/////////////////////////////////////////////////
Ogre::String IgnTerrainMatGen::SM2Profile::ShaderHelperGLSL::GetChannel(
Ogre::uint _idx)
{
  Ogre::uint rem = _idx % 4;
  switch (rem)
  {
    case 0:
    default:
      return "x";
    case 1:
      return "y";
    case 2:
      return "z";
    case 3:
      return "w";
  };
}

// #if OGRE_VERSION_MAJOR == 1 && OGRE_VERSION_MINOR < 11
#endif

/////////////////////////////////////////////////
/////////////////////////////////////////////////
// TerrainMaterial
/////////////////////////////////////////////////
/////////////////////////////////////////////////

//////////////////////////////////////////////////
TerrainMaterial::TerrainMaterial(const std::string &_materialName)
  : materialName(_materialName)
{
  this->mProfiles.push_back(OGRE_NEW Profile(this, "OgreMaterial",
      "Profile for rendering Ogre standard material"));
  this->setActiveProfile("OgreMaterial");
}

//////////////////////////////////////////////////
void TerrainMaterial::setMaterialByName(const std::string &_materialname)
{
  this->materialName = _materialname;
}

//////////////////////////////////////////////////
void TerrainMaterial::setGridSize(const unsigned int _size)
{
  if (_size == 0)
  {
    ignerr << "Unable to set a grid size of zero" << std::endl;
    return;
  }

  this->gridSize = _size;
}

//////////////////////////////////////////////////
TerrainMaterial::Profile::Profile(Ogre::TerrainMaterialGenerator *_parent,
    const Ogre::String &_name, const Ogre::String &_desc)
    : Ogre::TerrainMaterialGenerator::Profile(_parent, _name, _desc)
{
}

//////////////////////////////////////////////////
TerrainMaterial::Profile::~Profile()
{
}

//////////////////////////////////////////////////
bool TerrainMaterial::Profile::isVertexCompressionSupported() const
{
  return false;
}

//////////////////////////////////////////////////
Ogre::MaterialPtr TerrainMaterial::Profile::generate(
    const Ogre::Terrain *_terrain)
{
  const Ogre::String& matName = _terrain->getMaterialName();

  Ogre::MaterialPtr mat =
      Ogre::MaterialManager::getSingleton().getByName(matName);
  if (!mat.isNull())
      Ogre::MaterialManager::getSingleton().remove(matName);

  TerrainMaterial *parent =
      dynamic_cast<TerrainMaterial *>(getParent());

  // Set Ogre material
  mat = Ogre::MaterialManager::getSingleton().getByName(parent->materialName);

  // clone the material
  mat = mat->clone(matName);
  if (!mat->isLoaded())
    mat->load();

  // size of grid in one direction
  unsigned int gridWidth =
      static_cast<unsigned int>(std::sqrt(parent->gridSize));
  // factor to be applied to uv transformation: scale and translation
  double factor = 1.0 / gridWidth;
  // static counter to keep track which terrain slot we are currently in
  static int gridCount = 0;

  for (unsigned int i = 0; i < mat->getNumTechniques(); ++i)
  {
    Ogre::Technique *tech = mat->getTechnique(i);
    for (unsigned int j = 0; j < tech->getNumPasses(); ++j)
    {
      Ogre::Pass *pass = tech->getPass(j);

      // check if there is a fragment shader
      if (!pass->hasFragmentProgram())
        continue;

      Ogre::GpuProgramParametersSharedPtr params =
          pass->getFragmentProgramParameters();
      if (params.isNull())
        continue;

      // set up shadow split points in a way that is consistent with the
      // default ogre terrain material generator
      Ogre::PSSMShadowCameraSetup* pssm =
          OgreRTShaderSystem::Instance()->PSSMShadowCameraSetup();
      unsigned int numTextures =
          static_cast<unsigned int>(pssm->getSplitCount());
      Ogre::Vector4 splitPoints;
      const Ogre::PSSMShadowCameraSetup::SplitPointList& splitPointList =
          pssm->getSplitPoints();
      // populate from split point 1 not 0, and include shadowFarDistance
      for (unsigned int t = 0u; t < numTextures; ++t)
        splitPoints[t] = splitPointList[t+1];
      params->setNamedConstant("pssmSplitPoints", splitPoints);

      // set up uv transform
      double xTrans = static_cast<int>(gridCount / gridWidth) * factor;
      double yTrans = (gridWidth - 1 - (gridCount % gridWidth)) * factor;
      // explicitly set all matrix elements to avoid uninitialized values
      Ogre::Matrix4 uvTransform(factor, 0.0, 0.0, xTrans,
                                0.0, factor, 0.0, yTrans,
                                0.0, 0.0, 1.0, 0.0,
                                0.0, 0.0, 0.0, 1.0);
      params->setNamedConstant("uvTransform", uvTransform);
    }
  }
  gridCount++;

  // Get default pass
  Ogre::Pass *p = mat->getTechnique(0)->getPass(0);

  // Add terrain's global normalmap to renderpass so the
  // fragment program can find it.
  Ogre::TextureUnitState *tu = p->createTextureUnitState(matName+"/nm");

  Ogre::TexturePtr nmtx = _terrain->getTerrainNormalMap();
  tu->_setTexturePtr(nmtx);

  return mat;
}

//////////////////////////////////////////////////
Ogre::MaterialPtr TerrainMaterial::Profile::generateForCompositeMap(
    const Ogre::Terrain *_terrain)
{
  return _terrain->_getCompositeMapMaterial();
}

//////////////////////////////////////////////////
void TerrainMaterial::Profile::setLightmapEnabled(
    bool /*_enabled*/)
{
}

//////////////////////////////////////////////////
Ogre::uint8 TerrainMaterial::Profile::getMaxLayers(
    const Ogre::Terrain * /*_terrain*/) const
{
  return 0;
}

//////////////////////////////////////////////////
void TerrainMaterial::Profile::updateParams(const Ogre::MaterialPtr &/*_mat*/,
    const Ogre::Terrain * /*_terrain*/)
{
}

//////////////////////////////////////////////////
void TerrainMaterial::Profile::updateParamsForCompositeMap(
    const Ogre::MaterialPtr &/*_mat*/, const Ogre::Terrain * /*_terrain*/)
{
}

//////////////////////////////////////////////////
void TerrainMaterial::Profile::requestOptions(Ogre::Terrain *_terrain)
{
  _terrain->_setMorphRequired(true);
  // enable global normal map
  _terrain->_setNormalMapRequired(true);
  _terrain->_setLightMapRequired(false);
  _terrain->_setCompositeMapRequired(false);
}
