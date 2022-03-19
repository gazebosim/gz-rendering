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

#include "ignition/rendering/ShaderParams.hh"
#include "ignition/rendering/ogre/OgreRenderEngine.hh"
#include "ignition/rendering/ogre/OgreConversions.hh"
#include "ignition/rendering/ogre/OgreMapVisual.hh"
#include "ignition/rendering/ogre/OgreMaterial.hh"
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

// TODO:
//   1. Add the concept of swatches.
//   2. Allow user to set map data.
//   3. Allow user to set palette (map, occupancy, raw)

class ignition::rendering::OgreMapVisualPrivate
{
  public: void SetPaletteColor(std::vector<unsigned char> &_palette,
               unsigned char _position, unsigned char _r, unsigned char _g,
               unsigned char _b, unsigned char _a);

  public: void SetPaletteIllegalPositiveValues(
    std::vector<unsigned char> &_palette, unsigned char _r, unsigned char _g,
    unsigned char _b, unsigned char _a);

  public: void SetPaletteIllegalNegativeValues(
              std::vector<unsigned char> &_palette);

  public: void CreateMapPalette();

  public: void CreateCostmapPalette();

  public: void CreateRawPalette();

  public: std::vector<unsigned char> mapPalette;
  public: std::vector<unsigned char> costmapPalette;
  public: std::vector<unsigned char> rawPalette;

  /// \brief Map visual materal
  public: Ogre::Material *material;

  public: Ogre::TexturePtr texture;

  public: Ogre::TexturePtr paletteTexture;
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
}

//////////////////////////////////////////////////
void OgreMapVisual::Init()
{
  BaseMapVisual::Init();
  this->Create();
}

//////////////////////////////////////////////////
void OgreMapVisualPrivate::SetPaletteColor(std::vector<unsigned char> &_palette,
    unsigned char _position, unsigned char _r, unsigned char _g,
    unsigned char _b, unsigned char _a)
{
  _palette[4 * _position + 0] = _r;
  _palette[4 * _position + 1] = _g;
  _palette[4 * _position + 2] = _b;
  _palette[4 * _position + 3] = _a;
}

//////////////////////////////////////////////////
void OgreMapVisualPrivate::SetPaletteIllegalPositiveValues(
    std::vector<unsigned char> &_palette, unsigned char _r, unsigned char _g,
    unsigned char _b, unsigned char _a)
{
  // Set color for illegal positive values
  for (unsigned char i = 101; i <= 127; i++)
    this->SetPaletteColor(_palette, i, _r, _g, _b, _a);
}

//////////////////////////////////////////////////
void OgreMapVisualPrivate::SetPaletteIllegalNegativeValues(
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
void OgreMapVisualPrivate::CreateMapPalette()
{
  // Allocate the color palette
  this->mapPalette.assign(256 * 4, 0);

  // Grey map values in the range 0-100. This supports a scaled map display
  for (unsigned char i = 0; i <= 100; ++i)
  {
    unsigned char v = 255 - (255 * i) / 100;
    this->SetPaletteColor(this->mapPalette, i, v, v, v, 255);
  }

  // Set illegal positive values to green
  this->SetPaletteIllegalPositiveValues(this->mapPalette,
      0, 255, 0, 255);

  // Set colors for illegal negative values.
  this->SetPaletteIllegalNegativeValues(this->mapPalette);

  // Set color for legal negative value minus one.
  this->SetPaletteColor(this->mapPalette, 255, 0x70, 0x89, 0x86, 255);
}

//////////////////////////////////////////////////
void OgreMapVisualPrivate::CreateCostmapPalette()
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
void OgreMapVisualPrivate::CreateRawPalette()
{
  // Allocate the color palette
  this->rawPalette.assign(256 * 4, 0);

  for (int i = 0; i < 256; ++i)
    this->SetPaletteColor(this->rawPalette, i, i, i, i, 255);
}

//////////////////////////////////////////////////
void OgreMapVisual::Create()
{
  // Create the map texture palette
  this->dataPtr->CreateMapPalette();

  // Create the costmap texture palette
  this->dataPtr->CreateCostmapPalette();

  // Create the raw texture palette
  this->dataPtr->CreateRawPalette();

  // TEST CODE
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

  // Create the map texture
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
  }
  catch(Ogre::Exception& e)
  {
    std::cout << e.what() << std::endl;
  }

  // Create the palette texture
  Ogre::DataStreamPtr paletteStream(new Ogre::MemoryDataStream(
        this->dataPtr->mapPalette.data(), 256*4));
  try
  {
    this->dataPtr->paletteTexture =
      Ogre::TextureManager::getSingleton().loadRawData(
      "PaletteTexture1",
      Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
      paletteStream, 256, 1, Ogre::PF_BYTE_RGBA, Ogre::TEX_TYPE_1D, 0);
  }
  catch(Ogre::Exception& e)
  {
    std::cout << e.what() << std::endl;
  }

  // Create the material.
  auto mat = std::dynamic_pointer_cast<OgreMaterial>(
      this->Scene()->CreateMaterial("MapMaterial"));

  const char *env = std::getenv("IGN_RENDERING_RESOURCE_PATH");
  std::string resourcePath = (env) ? std::string(env) :
      IGN_RENDERING_RESOURCE_PATH;

  // The vertex and fragment shaders used to render map data.
  std::string vertPath = common::joinPaths(
      resourcePath, "ogre", "media", "materials", "programs",
      "indexed_8bit_image_vs.glsl");
  std::string fragPath = common::joinPaths(
      resourcePath, "ogre", "media", "materials", "programs",
      "indexed_8bit_image_fs.glsl");

  mat->SetVertexShader(vertPath);
  mat->SetFragmentShader(fragPath);
  mat->SetReceiveShadows(false);
  mat->Material()->getTechnique(0)->setLightingEnabled(false);
  mat->Material()->setDepthBias(-16.0f, 0.0f);
  mat->Material()->setCullingMode(Ogre::CULL_NONE);
  mat->SetDepthWriteEnabled(false);

  Ogre::Pass *pass = mat->Material()->getTechnique(0)->getPass(0);
  Ogre::TextureUnitState *textureUnit = nullptr;

  // Set the pixel texture. This is the map data. The material automatically
  // creates one texture unit state, so we can just get it.
  textureUnit = pass->getTextureUnitState(0);
  textureUnit->setTexture(this->dataPtr->texture);
  textureUnit->setTextureFiltering(Ogre::TFO_NONE);

  // Set the costmap palette texture. We need to create another texture unit
  // to hold the palette
  textureUnit = pass->createTextureUnitState();
  textureUnit->setTexture(this->dataPtr->paletteTexture);
  textureUnit->setTextureFiltering(Ogre::TFO_NONE);

  // Update the fragment shader parameters so that the shader knows what
  // textures to sample from.
  Ogre::GpuProgramParametersSharedPtr fragParams =
    pass->getFragmentProgramParameters();
  fragParams->setNamedConstant("eight_bit_image", 0);
  fragParams->setNamedConstant("palette", 1);

  // Create the manual object that will hold the map material.
  auto manualObject =
    this->scene->OgreSceneManager()->createManualObject("MapPlane");
  manualObject->begin(
      "MapMaterial", Ogre::RenderOperation::OT_TRIANGLE_LIST);
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
  manualObject->end();

  // Add the manual object.
  this->ogreNode->attachObject(manualObject);
  this->ogreNode->setScale(10, 10, 1);
}

//////////////////////////////////////////////////
void OgreMapVisual::SetVisible(bool /*_visible*/)
{
  //this->dataPtr->visible = _visible;
  //this->ogreNode->setVisible(this->dataPtr->visible);
}
