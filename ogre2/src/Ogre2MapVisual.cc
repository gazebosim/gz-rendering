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
#include "ignition/rendering/ogre2/Ogre2Material.hh"
#include "ignition/rendering/ogre2/Ogre2Scene.hh"

#ifdef _MSC_VER
  #pragma warning(push, 0)
#endif
#include <OgreCommon.h>
#include <Hlms/Pbs/OgreHlmsPbs.h>
#include <Hlms/Pbs/OgreHlmsPbsDatablock.h>
#include <OgreItem.h>
#include <OgreManualObject2.h>
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
  public: void SetPaletteColor(std::vector<unsigned char> &_palette,
               unsigned char _position, unsigned char _r, unsigned char _g,
               unsigned char _b, unsigned char _a);

  public: void SetPaletteIllegalPositiveValues(
    std::vector<unsigned char> &_palette, unsigned char _r, unsigned char _g,
    unsigned char _b, unsigned char _a);

  public: void SetPaletteIllegalNegativeValues(
              std::vector<unsigned char> &_palette);


  public: void CreateCostmapPalette();

  public: std::vector<unsigned char> costmapPalette;

  /// \brief inertia visual materal
  public: Ogre2MaterialPtr material = nullptr;

  /// \brief Map surface visual
  public: VisualPtr mapVis = nullptr;

  /// \brief Ogre item created from the dynamic geometry
  public: Ogre::Item *ogreItem = nullptr;
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
}

//////////////////////////////////////////////////
void Ogre2MapVisual::Init()
{
  BaseMapVisual::Init();
  this->Create();
}

//////////////////////////////////////////////////
void Ogre2MapVisualPrivate::SetPaletteColor(std::vector<unsigned char> &_palette,
    unsigned char _position, unsigned char _r, unsigned char _g,
    unsigned char _b, unsigned char _a)
{
  _palette[4 * _position + 0] = _r;
  _palette[4 * _position + 1] = _g;
  _palette[4 * _position + 2] = _b;
  _palette[4 * _position + 3] = _a;
}

//////////////////////////////////////////////////
void Ogre2MapVisualPrivate::SetPaletteIllegalPositiveValues(
    std::vector<unsigned char> &_palette, unsigned char _r, unsigned char _g,
    unsigned char _b, unsigned char _a)
{
  // Set color for illegal positive values
  for (unsigned char i = 101; i <= 127; i++)
    this->SetPaletteColor(_palette, i, _r, _g, _b, _a);
}

//////////////////////////////////////////////////
void Ogre2MapVisualPrivate::SetPaletteIllegalNegativeValues(
    std::vector<unsigned char> &_palette)
{
  // set shades from red to yellow
  for (unsigned char i = 128; i <= 254; i++)
  {
    this->SetPaletteColor(_palette, i,
        255, (255 * (i - 128)) / (254 - 128), 0, 255);
  }
}

//////////////////////////////////////////////////
void Ogre2MapVisualPrivate::CreateCostmapPalette()
{
  // Allocate the color palette
  this->costmapPalette.assign(256 * 4, 0);

  // Make the texture palette
  this->SetPaletteColor(this->costmapPalette, 0, 0, 0, 0, 0);

  // Grey map values in the range 0-100. This supports a scaled map display
  for (unsigned char i = 1; i <= 98; i++)
  {
    unsigned char v = (255 * i) / 100;
    this->SetPaletteColor(this->costmapPalette, i, v, 0, 255 - v, 255);
  }

  // Use cyan for obstacle values
  this->SetPaletteColor(this->costmapPalette, 99, 0, 255, 255, 255);

  // Use purple for lethal obstacles
  this->SetPaletteColor(this->costmapPalette, 100, 255, 0, 255, 255);

  // Set illegal positive values to green
  this->SetPaletteIllegalPositiveValues(this->costmapPalette,
      0, 255, 0, 255);

  // Set colors for illegal negative values.
  this->SetPaletteIllegalNegativeValues(this->costmapPalette);

  // Set color for legal negative value minus one.
  this->SetPaletteColor(this->costmapPalette, 255, 0x70, 0x89, 0x86, 255);
}

//////////////////////////////////////////////////
void Ogre2MapVisual::Create()
{
  // Create the costmap texture palette
  //
  // Add this back in when shaders workd
  // this->dataPtr->CreateCostmapPalette();

  // TEST CODE that creates a image composed of dots
  int width = 100;
  int height = 100;
  int pixelCount = width * height;
  auto pixels = std::vector<unsigned char>(pixelCount, 100);
  for (int y = 0; y <height; ++y)
  {
    for (int x =0; x<width; ++x)
    {
      if (math::isEven(x) && math::isOdd(y))
         pixels[y*width+x] = 10;
      else
        pixels[y*width+x] = 200;
    }
  }

  // Get the texture manager.
  auto engine = Ogre2RenderEngine::Instance();
  auto ogreRoot = engine->OgreRoot();
  Ogre::TextureGpuManager *textureMgr =
    ogreRoot->getRenderSystem()->getTextureGpuManager();

  // Create an image based on the pixels.
  Ogre::Image2 image;
  image.loadDynamicImage(pixels.data(),
      width, height, 1u, Ogre::TextureTypes::Type2D,
      Ogre::PFG_R8_UINT, false);

  // Create the texture.
  Ogre::TextureGpu *texture;
  try
  {
    texture = textureMgr->createOrRetrieveTexture(
        "MapTexture",
        Ogre::GpuPageOutStrategy::SaveToSystemRam,
        Ogre::TextureFlags::ManualTexture,
        Ogre::TextureTypes::Type2D);

    texture->setResolution(image.getWidth(), image.getHeight());
    texture->setNumMipmaps(image.getNumMipmaps());
    texture->setPixelFormat(image.getPixelFormat());
    texture->scheduleTransitionTo(Ogre::GpuResidency::Resident);
    image.uploadTo(texture, 0, texture->getNumMipmaps()-1);
    texture->writeContentsToFile("/home/nkoenig/map.png", 0, 1, true);
  }
  catch(Ogre::Exception& e)
  {
    std::cout << e.what() << std::endl;
  }

  // Create the material.
  auto mat = std::dynamic_pointer_cast<Ogre2Material>(
      this->Scene()->CreateMaterial("MapMaterial"));

  // This line doesn't work, because `mat` requires a texture file, but
  // I have texture that is in memory.
  //mat->SetTexture("MapTexture");
  mat->SetReceiveShadows(false);
  mat->SetDepthWriteEnabled(false);

  // Try to add the texture to a texture unit.
  Ogre::Pass *pass = mat->Material()->getTechnique(0)->getPass(0);
  Ogre::TextureUnitState *textureUnit = nullptr;
  textureUnit = pass->createTextureUnitState();
  textureUnit->setTexture(texture);
  //textureUnit->setTextureFiltering(Ogre::TFO_NONE);

  // Create the manual object that will hold the map material.
  // The `begin` line should associate the material with this object.
  auto manualObject =
    this->scene->OgreSceneManager()->createManualObject();
  manualObject->begin(
      *mat->Datablock()->getNameStr(), Ogre::OT_TRIANGLE_LIST);
  manualObject->position(0, 0, 0);
  manualObject->textureCoord(0, 0);
  manualObject->normal(0, 0, 1);

  manualObject->position(1, 1, 0);
  manualObject->textureCoord(1, 1);
  manualObject->normal(0, 0, 1);

  manualObject->position(0, 1, 0);
  manualObject->textureCoord(0, 1);
  manualObject->normal(0, 0, 1);

  manualObject->position(0, 0, 0);
  manualObject->textureCoord(0, 0);
  manualObject->normal(0, 0, 1);

  manualObject->position(1, 0, 0);
  manualObject->textureCoord(1, 0);
  manualObject->normal(0, 0, 1);

  manualObject->position(1, 1, 0);
  manualObject->textureCoord(1, 1);
  manualObject->normal(0, 0, 1);

  manualObject->triangle(0, 1, 2);
  manualObject->triangle(3, 4, 5);
  manualObject->end();

  // Attache the manual object to the scene node.
  this->ogreNode->attachObject(manualObject);
  this->ogreNode->setScale(10, 10, 1);
}

//////////////////////////////////////////////////
void Ogre2MapVisual::SetVisible(bool _visible)
{
  //this->dataPtr->visible = _visible;
  //this->ogreNode->setVisible(this->dataPtr->visible);
}
