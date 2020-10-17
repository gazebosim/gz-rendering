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

#include <ignition/common/Console.hh>

#include "ignition/rendering/ogre/OgreHeightmap.hh"
#include "ignition/rendering/ogre/OgreMaterial.hh"
#include "ignition/rendering/ogre/OgreScene.hh"

class ignition::rendering::OgreHeightmapPrivate
{
  /// \brief Heightmap materal
  public: OgreMaterialPtr material;

  /// \brief Ogre manual object used to render the heightmap.
  public: Ogre::ManualObject *manualObject = nullptr;

  public: common::HeightmapData *data{nullptr};
};

using namespace ignition;
using namespace rendering;

//////////////////////////////////////////////////
OgreHeightmap::OgreHeightmap(common::HeightmapData *_data)
    : dataPtr(new OgreHeightmapPrivate)
{
  this->dataPtr->data = _data;
}

//////////////////////////////////////////////////
OgreHeightmap::~OgreHeightmap()
{
}

//////////////////////////////////////////////////
void OgreHeightmap::Init()
{
//  if (this->dataPtr->terrainGlobals != nullptr)
//    return;

  auto capabilities =
      Ogre::Root::getSingleton().getRenderSystem()->getCapabilities();
  Ogre::DriverVersion glVersion;
  glVersion.build = 0;
  glVersion.major = 3;
  glVersion.minor = 0;
  glVersion.release = 0;
  if (capabilities->isDriverOlderThanVersion(glVersion))
  {
//    glslVersion = "120";
//    vpInStr = "attribute";
//    vpOutStr = "varying";
//    fpInStr = "varying";
//    textureStr = "texture2D";
  }

  // The terraingGroup is composed by a number of terrains (1 by default)
//  int nTerrains = 1;

//  this->dataPtr->terrainGlobals = new Ogre::TerrainGlobalOptions();
//
//#if (OGRE_VERSION_MAJOR == 1 && OGRE_VERSION_MINOR >= 8) || \
//    OGRE_VERSION_MAJOR > 1
//  // Vertex compression breaks anything, e.g. Gpu laser, that tries to build
//  // a depth map.
//  this->dataPtr->terrainGlobals->setUseVertexCompressionWhenAvailable(false);
//#endif
//
//  // There is an issue with OGRE terrain LOD if heights are not relative to 0.
//  // So we move the heightmap so that its min elevation = 0 before feeding to
//  // ogre. It is later translated back by the setOrigin call.
//  double minElevation = 0.0;
//
//  // try loading heightmap data locally
//  if (!this->dataPtr->filename.empty())
//  {
//    this->dataPtr->heightmapData = common::HeightmapDataLoader::LoadTerrainFile(
//        this->dataPtr->filename);
//
//    if (this->dataPtr->heightmapData)
//    {
//      // TODO add a virtual HeightmapData::GetMinElevation function to avoid the
//      // ifdef check. i.e. heightmapSizeZ = GetMaxElevation - GetMinElevation
//      double heightmapSizeZ = this->dataPtr->heightmapData->GetMaxElevation();
//#ifdef HAVE_GDAL
//      auto demData =
//          dynamic_cast<common::Dem *>(this->dataPtr->heightmapData);
//      if (demData)
//      {
//        heightmapSizeZ = heightmapSizeZ - demData->GetMinElevation();
//        if (this->dataPtr->terrainSize == ignition::math::Vector3d::Zero)
//        {
//          this->dataPtr->terrainSize = ignition::math::Vector3d(
//              demData->GetWorldWidth(), demData->GetWorldHeight(),
//              heightmapSizeZ);
//        }
//        minElevation = demData->GetMinElevation();
//      }
//#endif
//
//      // these params need to be the same as physics/HeightmapShape.cc
//      // in order to generate consistent height data
//      bool flipY = false;
//      // sampling size along image width and height
//      unsigned int vertSize = (this->dataPtr->heightmapData->GetWidth() *
//          this->dataPtr->sampling) - this->dataPtr->sampling + 1;
//      ignition::math::Vector3d scale;
//      scale.X(this->dataPtr->terrainSize.X() / vertSize);
//      scale.Y(this->dataPtr->terrainSize.Y() / vertSize);
//
//      if (ignition::math::equal(heightmapSizeZ, 0.0))
//        scale.Z(1.0);
//      else
//        scale.Z(fabs(this->dataPtr->terrainSize.Z()) / heightmapSizeZ);
//
//      // Construct the heightmap lookup table
//      std::vector<float> lookup;
//      this->dataPtr->heightmapData->FillHeightMap(this->dataPtr->sampling,
//          vertSize, this->dataPtr->terrainSize, scale, flipY, lookup);
//
//      for (unsigned int y = 0; y < vertSize; ++y)
//      {
//        for (unsigned int x = 0; x < vertSize; ++x)
//        {
//          int index = (vertSize - y - 1) * vertSize + x;
//          this->dataPtr->heights.push_back(lookup[index] - minElevation);
//        }
//      }
//
//      this->dataPtr->dataSize = vertSize;
//    }
//  }
//
//  // if heightmap fails to load locally, get the data from the server side
//  if (this->dataPtr->heights.empty())
//  {
//    gzmsg << "Heightmap could not be loaded locally "
//          << "(is it in the GAZEBO_RESOURCE_PATH?)- requesting data from "
//          << "the server" << std::endl;
//
//    msgs::Geometry geomMsg;
//
//    boost::shared_ptr<msgs::Response> response = transport::request(
//       this->dataPtr->scene->Name(), "heightmap_data");
//
//    if (response->response() != "error" &&
//        response->type() == geomMsg.GetTypeName())
//    {
//      geomMsg.ParseFromString(response->serialized_data());
//
//      // Copy the height data.
//      this->dataPtr->terrainSize = msgs::ConvertIgn(geomMsg.heightmap().size());
//      this->dataPtr->heights.resize(geomMsg.heightmap().heights().size());
//      memcpy(&this->dataPtr->heights[0], geomMsg.heightmap().heights().data(),
//          sizeof(this->dataPtr->heights[0]) *
//          geomMsg.heightmap().heights().size());
//
//      this->dataPtr->dataSize = geomMsg.heightmap().width();
//    }
//  }
//
//  if (this->dataPtr->heights.empty())
//  {
//    gzerr << "Failed to load terrain. Heightmap data is empty" << std::endl;
//    return;
//  }
//
//  if (!ignition::math::isPowerOfTwo(this->dataPtr->dataSize - 1))
//  {
//    gzerr << "Heightmap image size must be square, with a size of 2^n+1"
//        << std::endl;
//    return;
//  }
//
//  boost::filesystem::path imgPath;
//  boost::filesystem::path terrainName;
//  boost::filesystem::path terrainDirPath;
//  boost::filesystem::path prefix;
//  if (!this->dataPtr->filename.empty())
//  {
//    // Get the full path of the image heightmap
//    imgPath = this->dataPtr->filename;
//    terrainName = imgPath.filename().stem();
//    terrainDirPath = this->dataPtr->gzPagingDir / terrainName;
//
//    // Add the top level terrain paging directory to the OGRE
//    // ResourceGroupManager
//    boost::filesystem::path actualPagingDir =
//        this->dataPtr->gzPagingDir.make_preferred();
//    if (!Ogre::ResourceGroupManager::getSingleton().resourceLocationExists(
//          actualPagingDir.string(), "General"))
//    {
//      Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
//          actualPagingDir.string(), "FileSystem", "General", true);
//      Ogre::ResourceGroupManager::getSingleton().initialiseResourceGroup(
//          "General");
//    }
//  }
//
//  // If the paging is enabled we modify the number of subterrains
//  if (this->dataPtr->useTerrainPaging)
//  {
//    this->dataPtr->splitTerrain = true;
//    nTerrains = this->dataPtr->numTerrainSubdivisions;
//    prefix = terrainDirPath / "gazebo_terrain_cache";
//  }
//  else
//  {
//    // Note: ran into problems with LOD height glitches if heightmap size is
//    // larger than 4096 so split it into chunks
//    // Note: dataSize should be 2^n + 1
//    if (this->dataPtr->maxPixelError > 0 && this->dataPtr->dataSize > 4096u)
//    {
//      this->dataPtr->splitTerrain = true;
//      if (this->dataPtr->dataSize == 4097u)
//        this->dataPtr->numTerrainSubdivisions = 4u;
//      else
//        this->dataPtr->numTerrainSubdivisions = 16u;
//     nTerrains = this->dataPtr->numTerrainSubdivisions;
//
//      gzmsg << "Large heightmap used with LOD. It will be subdivided into " <<
//          this->dataPtr->numTerrainSubdivisions << " terrains." << std::endl;
//    }
//    prefix = terrainDirPath / "gazebo_terrain";
//  }
//
//  double sqrtN = sqrt(nTerrains);
//
//  // Create terrain group, which holds all the individual terrain instances.
//  // Param 1: Pointer to the scene manager
//  // Param 2: Alignment plane
//  // Param 3: Number of vertices along one edge of the terrain (2^n+1).
//  //          Terrains must be square, with each side a power of 2 in size
//  // Param 4: World size of each terrain instance, in meters.
//
//  this->dataPtr->terrainGroup = new Ogre::TerrainGroup(
//      this->dataPtr->scene->OgreSceneManager(), Ogre::Terrain::ALIGN_X_Y,
//      1 + ((this->dataPtr->dataSize - 1) / sqrtN),
//      this->dataPtr->terrainSize.X() / (sqrtN));
//
//  this->dataPtr->terrainGroup->setFilenameConvention(
//    Ogre::String(prefix.string()), Ogre::String("dat"));
//
//  Ogre::Vector3 orig = Conversions::Convert(this->dataPtr->terrainOrigin);
//  ignition::math::Vector3d origin(orig.x -0.5 * this->dataPtr->terrainSize.X() +
//      0.5 * this->dataPtr->terrainSize.X() / sqrtN,
//      orig.y -0.5 * this->dataPtr->terrainSize.X() +
//      0.5 * this->dataPtr->terrainSize.X() / sqrtN,
//      orig.z + minElevation);
//
//  this->dataPtr->terrainGroup->setOrigin(Conversions::Convert(origin));
//  this->ConfigureTerrainDefaults();
//
//  if (!this->dataPtr->heights.empty())
//  {
//    UserCameraPtr userCam = this->dataPtr->scene->GetUserCamera(0);
//
//    // Move the camera above the terrain only if the user did not modify the
//    // camera position in the world file
//    if (userCam && !userCam->IsCameraSetInWorldFile())
//    {
//      double h = *std::max_element(
//        &this->dataPtr->heights[0],
//        &this->dataPtr->heights[0] + this->dataPtr->heights.size());
//
//      ignition::math::Vector3d camPos(5, -5, h + 200);
//      ignition::math::Vector3d lookAt(0, 0, h);
//      auto mat = ignition::math::Matrix4d::LookAt(camPos, lookAt);
//
//      userCam->SetWorldPose(mat.Pose());
//    }
//  }
//
//  this->dataPtr->terrainHashChanged = this->PrepareTerrain(terrainDirPath);
//
//  if (this->dataPtr->useTerrainPaging)
//  {
//    if (this->dataPtr->terrainHashChanged)
//    {
//      // Split the terrain. Every subterrain will be saved on disk and paged
//      this->SplitHeights(this->dataPtr->heights, nTerrains,
//          this->dataPtr->subTerrains);
//    }
//
//    this->dataPtr->pageManager = OGRE_NEW Ogre::PageManager();
//    this->dataPtr->pageManager->setPageProvider(
//        &this->dataPtr->dummyPageProvider);
//
//    // Add cameras
//    for (unsigned int i = 0; i < this->dataPtr->scene->CameraCount(); ++i)
//    {
//      this->dataPtr->pageManager->addCamera(
//          this->dataPtr->scene->GetCamera(i)->OgreCamera());
//    }
//    for (unsigned int i = 0; i < this->dataPtr->scene->UserCameraCount();
//        ++i)
//    {
//      this->dataPtr->pageManager->addCamera(
//          this->dataPtr->scene->GetUserCamera(i)->OgreCamera());
//    }
//
//    this->dataPtr->terrainPaging =
//        OGRE_NEW Ogre::TerrainPaging(this->dataPtr->pageManager);
//    this->dataPtr->world = this->dataPtr->pageManager->createWorld();
//    this->dataPtr->terrainPaging->createWorldSection(
//        this->dataPtr->world, this->dataPtr->terrainGroup,
//        this->dataPtr->loadRadiusFactor * this->dataPtr->terrainSize.X(),
//        this->dataPtr->holdRadiusFactor * this->dataPtr->terrainSize.X(),
//        0, 0, sqrtN - 1, sqrtN - 1);
//  }
//
//  gzmsg << "Loading heightmap: " << terrainName.string() << std::endl;
//  common::Time time = common::Time::GetWallTime();
//
//  for (int y = 0; y <= sqrtN - 1; ++y)
//    for (int x = 0; x <= sqrtN - 1; ++x)
//      this->DefineTerrain(x, y);
//
//  // use gazebo shaders
//  this->CreateMaterial();
//
//  // Sync load since we want everything in place when we start
//  this->dataPtr->terrainGroup->loadAllTerrains(true);
//
//  gzmsg << "Heightmap loaded. Process took: "
//        <<  (common::Time::GetWallTime() - time).Double()
//        << " seconds" << std::endl;
//
//  // Calculate blend maps
//  if (this->dataPtr->terrainsImported)
//  {
//    Ogre::TerrainGroup::TerrainIterator ti =
//      this->dataPtr->terrainGroup->getTerrainIterator();
//    while (ti.hasMoreElements())
//    {
//      Ogre::Terrain *t = ti.getNext()->instance;
//      this->InitBlendMaps(t);
//    }
//  }
//
//  this->dataPtr->terrainGroup->freeTemporaryResources();
//
//  // save the terrain once its loaded
//  if (this->dataPtr->terrainsImported)
//  {
//    this->dataPtr->connections.push_back(
//        event::Events::ConnectPreRender(
//        std::bind(&Heightmap::SaveHeightmap, this)));
//  }
}

//////////////////////////////////////////////////
Ogre::MovableObject *OgreHeightmap::OgreObject() const
{
  return this->dataPtr->manualObject;
}

//////////////////////////////////////////////////
void OgreHeightmap::SetMaterial(MaterialPtr, bool)
{
  // TODO
}

//////////////////////////////////////////////////
MaterialPtr OgreHeightmap::Material() const
{
  return this->dataPtr->material;
}

