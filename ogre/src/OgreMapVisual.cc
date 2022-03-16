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

#include "ignition/rendering/ogre/OgreRenderEngine.hh"
#include "ignition/rendering/ogre/OgreConversions.hh"
#include "ignition/rendering/ogre/OgreMapVisual.hh"
#include "ignition/rendering/ogre/OgreScene.hh"

#ifdef _MSC_VER
  #pragma warning(push, 0)
#endif
#include <OgreMaterialManager.h>
#include <OgreRoot.h>
#include <OgreSceneNode.h>
#include <OgreTechnique.h>
#ifdef _MSC_VER
  #pragma warning(pop)
#endif

class ignition::rendering::OgreMapVisualPrivate
{
  /// \brief Map visual materal
  public: Ogre::Material *material;

  public: Ogre::TexturePtr texture;

  /// \brief Map surface visual
  public: VisualPtr mapVis = nullptr;
};

using namespace ignition;
using namespace rendering;

//////////////////////////////////////////////////
OgreMapVisual::OgreMapVisual()
  : dataPtr(new OgreMapVisualPrivate)
{
}

//////////////////////////////////////////////////
OgreMapVisual::~OgreMapVisual()
{
  // no ops
}

//////////////////////////////////////////////////
void OgreMapVisual::PreRender()
{
  // no ops
}

//////////////////////////////////////////////////
void OgreMapVisual::Destroy()
{
  if (this->dataPtr->mapVis != nullptr)
  {
    this->dataPtr->mapVis->Destroy();
    this->dataPtr->mapVis.reset();
  }
}

//////////////////////////////////////////////////
void OgreMapVisual::Init()
{
  BaseMapVisual::Init();
  this->Create();
}

//////////////////////////////////////////////////
void OgreMapVisual::SetPaletteColor(std::vector<unsigned char> &_palette,
    unsigned char _position,
    unsigned char _r, unsigned char _g, unsigned char _b, unsigned char _a)
{
  _palette[4 * _position + 0] = _r;
  _palette[4 * _position + 1] = _g;
  _palette[4 * _position + 2] = _b;
  _palette[4 * _position + 3] = _a;
}

//////////////////////////////////////////////////
void OgreMapVisual::Create()
{
  // Make the texture palette
  std::vector<unsigned char> costmapTextureBytes;
  this->SetPaletteColor(costmapTextureBytes, 0, 0, 0, 0, 0);
  for (unsigned char i = 1; i <= 98; i++)
  {
    unsigned char v = (255 * i) / 100;
    this->SetPaletteColor(costmapTextureBytes,
        i, v, 0, 255 - v, 255);
  }

  // obstacle values in cyan
  this->SetPaletteColor(costmapTextureBytes, 99, 0, 255, 255, 255);
  // lethal obstacle values in purple
  this->SetPaletteColor(costmapTextureBytes,100, 255, 0, 255, 255);

  std::cout << "OgreMapVisual::Create!\n";
  // this->dataPtr->mapVis->SetLocalPosition();
  // this->dataPtr->mapVis->SetLocalRotation();

  int width = 100;
  int height = 100;

  int pixelCount = width * height;

  auto pixels = std::vector<unsigned char>(pixelCount, 255);
  /*for (int i = 0; i < pixelCount; i += 2)
     pixels[i] = 128;
     */

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

  Ogre::DataStreamPtr pixelStream(new Ogre::MemoryDataStream(
         pixels.data(), pixelCount));

  try
  {
   this->dataPtr->texture = Ogre::TextureManager::getSingleton().loadRawData(
     "MapTexture1",// + std::to_string(texture_count_++),
     Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
     pixelStream,
     static_cast<uint16_t>(width), static_cast<uint16_t>(height),
     Ogre::PF_L8, Ogre::TEX_TYPE_2D, 0);


  /*std::string matName = "Indexed8BitImage";
  Ogre::ResourcePtr res =
    Ogre::MaterialManager::getSingleton().load(matName,
        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

  // OGRE 1.9 changes the shared pointer definition
  #if OGRE_VERSION_LT_1_11_0
  this->dataPtr->material = res.staticCast<Ogre::Material>();
  #else
  this->dataPtr->material = std::static_pointer_cast<Ogre::Material>(res);
  #endif
  this->dataPtr->material->load();
  static int thermalMatNameCount = 0;
  this->dataPtr->material = this->dataPtr->material->clone(
      matName + "_" + std::to_string(thermalMatNameCount++));
  this->dataPtr->material->load();
  */
/*
  this->dataPtr->material = dynamic_cast<Ogre::Material *>(
      Ogre::MaterialManager::getSingleton().getByName(matName).get());
  // clone the material since we're modifying it's definitions
  this->dataPtr->material = this->dataPtr->material->clone(
      matName + "_" + this->dataPtr->material->getName()).get();
  this->dataPtr->material->load();

   // this->dataPtr->material = this->dataPtr->material->clone("MapTexture1");
   this->dataPtr->material->setReceiveShadows(false);
   this->dataPtr->material->getTechnique(0)->setLightingEnabled(false);
   this->dataPtr->material->setDepthBias(-16.0f, 0.0f);
   this->dataPtr->material->setCullingMode(Ogre::CULL_NONE);
   this->dataPtr->material->setDepthWriteEnabled(false);

   Ogre::Pass *pass = this->dataPtr->material->getTechnique(0)->getPass(0);
   Ogre::TextureUnitState * texUnit = nullptr;
   if (pass->getNumTextureUnitStates() > 0)
      texUnit = pass->getTextureUnitState(0);
   else
      texUnit = pass->createTextureUnitState();

   texUnit->setTextureName(this->dataPtr->texture->getName());
   texUnit->setTextureFiltering(Ogre::TFO_NONE);

   std::cout << "Setting the material\n";
   this->dataPtr->mapVis->SetMaterial(this->dataPtr->material->getName());
   */
  }
  catch(Ogre::Exception& e)
  {
    std::cout << "FAILEd\n";
     std::cout << e.what() << std::endl;
  }
  this->dataPtr->mapVis = this->Scene()->CreateVisual();
  this->dataPtr->mapVis->AddGeometry(this->Scene()->CreatePlane());

  MaterialPtr mat = this->Scene()->CreateMaterial("MapMaterial");
  // mat->SetAmbient(1.0, 0.0, 0.0, 1.0);
  // mat->SetDiffuse(1.0, 0.0, 0.0, 1.0);

  const char *env = std::getenv("IGN_RENDERING_RESOURCE_PATH");
  std::string resourcePath = (env) ? std::string(env) :
      IGN_RENDERING_RESOURCE_PATH;

  std::string vertPath = common::joinPaths(
      resourcePath, "ogre", "media", "materials", "programs",
      "indexed_8bit_image_vs.glsl");
  std::string fragPath = common::joinPaths(
      resourcePath, "ogre", "media", "materials", "programs",
      "indexed_8bit_image_fs.glsl");

  mat->SetVertexShader(vertPath);
  mat->SetFragmentShader(fragPath);
  mat->SetTexture("MapTexture1");
  this->dataPtr->mapVis->SetMaterial("MapMaterial");
  this->AddChild(this->dataPtr->mapVis);

  this->dataPtr->mapVis->SetLocalScale(math::Vector3d(10, 10, 1));

}

//////////////////////////////////////////////////
void OgreMapVisual::SetVisible(bool /*_visible*/)
{
  //this->dataPtr->visible = _visible;
  //this->ogreNode->setVisible(this->dataPtr->visible);
}


//////////////////////////////////////////////////
Ogre::TexturePtr OgreMapVisual::MakePaletteTexture(
    std::vector<unsigned char> _bytes)
{
  Ogre::DataStreamPtr byteStream(
      new Ogre::MemoryDataStream(_bytes.data(), 256 * 4));

  static int texCount = 0;
  std::string texName = "MapPaletteTexture" + std::to_string(texCount++);
  return Ogre::TextureManager::getSingleton().loadRawData(
    texName, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
    byteStream, 256, 1, Ogre::PF_BYTE_RGBA, Ogre::TEX_TYPE_1D, 0);
}
