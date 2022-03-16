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

#ifdef __APPLE__
  #define GL_SILENCE_DEPRECATION
  #include <OpenGL/gl.h>
  #include <OpenGL/glext.h>
#else
#ifndef _WIN32
  #include <GL/gl.h>
#endif
#endif

#include <ignition/common/Console.hh>

#include "ignition/rendering/ogre2/Ogre2RenderEngine.hh"
#include "ignition/rendering/ogre2/Ogre2Conversions.hh"
#include "ignition/rendering/ogre2/Ogre2MapVisual.hh"
#include "ignition/rendering/ogre2/Ogre2Scene.hh"

#ifdef _MSC_VER
  #pragma warning(push, 0)
#endif
#include <OgreItem.h>
#include <OgreMaterialManager.h>
#include <OgreRoot.h>
#include <OgreSceneNode.h>
#include <OgreTechnique.h>
#include <OgreTextureGpuManager.h>
#include <OgreImage2.h>
#ifdef _MSC_VER
  #pragma warning(pop)
#endif

class ignition::rendering::Ogre2MapVisualPrivate
{
  /// \brief inertia visual materal
  public: Ogre2MaterialPtr material = nullptr;

  /// \brief Map surface visual
  public: VisualPtr mapVis = nullptr;
};

using namespace ignition;
using namespace rendering;

//////////////////////////////////////////////////
Ogre2MapVisual::Ogre2MapVisual()
  : dataPtr(new Ogre2MapVisualPrivate)
{
}

//////////////////////////////////////////////////
Ogre2MapVisual::~Ogre2MapVisual()
{
  // no ops
}

//////////////////////////////////////////////////
void Ogre2MapVisual::PreRender()
{
  // no ops
}

//////////////////////////////////////////////////
void Ogre2MapVisual::Destroy()
{
  if (this->dataPtr->mapVis != nullptr)
  {
    this->dataPtr->mapVis->Destroy();
    this->dataPtr->mapVis.reset();
  }
}

//////////////////////////////////////////////////
void Ogre2MapVisual::Init()
{
  BaseMapVisual::Init();
  this->Create();
}

//////////////////////////////////////////////////
void Ogre2MapVisual::Create()
{
  std::cout << "Create!\n";
  this->dataPtr->mapVis = this->Scene()->CreateVisual();
  this->dataPtr->mapVis->AddGeometry(this->Scene()->CreatePlane());
  this->dataPtr->mapVis->SetMaterial("Default/TransPurple");
  this->AddChild(this->dataPtr->mapVis);

  this->dataPtr->mapVis->SetLocalScale(math::Vector3d(10, 10, 1));
  // this->dataPtr->mapVis->SetLocalPosition();
  // this->dataPtr->mapVis->SetLocalRotation();

  int width = 100;
  int height = 100;

  int pixelCount = width * height;

  auto pixels = std::vector<unsigned char>(pixelCount, 255);

  /*auto pixel_data = pixels.begin();
  for (size_t map_row = y_; map_row < y_ + height_; map_row++)
  {
    size_t pixel_index = map_row * map_width + x_;
    size_t pixels_to_copy = std::min(width_, map_size - pixel_index);

    auto row_start = map.data.begin() + pixel_index;
    std::copy(row_start, row_start + pixels_to_copy, pixel_data);
    pixel_data += pixels_to_copy;
    if (pixel_index + pixels_to_copy >= map_size) {
      break;
    }
  }*/

   auto engine = Ogre2RenderEngine::Instance();
   auto ogreRoot = engine->OgreRoot();
   Ogre::TextureGpuManager *textureMgr =
     ogreRoot->getRenderSystem()->getTextureGpuManager();

   Ogre::TextureGpu *texture  = textureMgr->createOrRetrieveTexture(
       "maptexture",
       Ogre::GpuPageOutStrategy::SaveToSystemRam,
       Ogre::TextureFlags::ManualTexture,
       Ogre::TextureTypes::Type2D);
   if (texture == nullptr)
     std::cerr << "Unable to create texture\n";
   texture->setResolution(width, height);

  this->dataPtr->mapVis->Material()->SetTexture("mapTexture");
   Ogre::Image2 image;
   unsigned char *dataDest = reinterpret_cast<unsigned char*>(
       OGRE_MALLOC_SIMD(pixelCount, Ogre::MEMCATEGORY_RESOURCE));

   for (int i = 0; i < pixelCount; i += 1)
     dataDest[i] = 255;

   if (!dataDest)
     std::cerr << "Unable to create data dest\n";

   image.loadDynamicImage(dataDest,
       width, height, 1u, Ogre::TextureTypes::Type2D,
       Ogre::PFG_R8_UINT, false);
   //image.loadDynamicImage(dataDest, false, texture);

   std::cout << "UploadTo\n";
   try
   {
     texture->setPixelFormat(image.getPixelFormat());
     texture->scheduleTransitionTo(Ogre::GpuResidency::Resident, &image,false);
     texture->writeContentsToFile("/home/nkoenig/map.png", 0, 1, true);

     // image.uploadTo(texture, 0, 0, 0, 1);
   }
   catch(Ogre::Exception& e)
   {
     std::cout << "Upload FAILED!!!\n";
     std::cout << e.what() << std::endl;
     std::cout << texture->getWidth() << " : " << image.getWidth() << std::endl;
     std::cout << texture->getHeight() << " : " << image.getHeight() << std::endl;
     std::cout << texture->getDepthOrSlices() << " : " << image.getDepthOrSlices() << std::endl;
    std::cout << texture->getDepthOrSlices() << " : 0" << std::endl;
    //std::cout << Ogre::PixelFormatGpuUtils::getFamily( texture->getPixelFormat() ) << " : " << Ogre::PixelFormatGpuUtils::getFamily( image.getPixelFormat() )
   }
   std::cout << "Done UploadTo\n";
   /*Ogre::DataStreamPtr pixel_stream(new Ogre::MemoryDataStream(
         pixels.data(), pixelCount));

   this->dataPtr->texture = Ogre::TextureManager::getSingleton().loadRawData(
     "MapTexture",// + std::to_string(texture_count_++),
     "map_visual",
     pixelStream,
     static_cast<uint16_t>(width), static_cast<uint16_t>(height),
     Ogre::PF_L8, Ogre::TEX_TYPE_2D, 0);
     */
}

//////////////////////////////////////////////////
void Ogre2MapVisual::SetVisible(bool _visible)
{
  //this->dataPtr->visible = _visible;
  //this->ogreNode->setVisible(this->dataPtr->visible);
}
