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


#include <ignition/common/Console.hh>

#include "ignition/rendering/RenderPassSystem.hh"
#include "ignition/rendering/ogre/OgreIncludes.hh"
#include "ignition/rendering/ogre/OgreDistortionPass.hh"

using namespace ignition;
using namespace rendering;

//////////////////////////////////////////////////
OgreDistortionPass::OgreDistortionPass()
{
}

//////////////////////////////////////////////////
OgreDistortionPass::~OgreDistortionPass()
{
}

//////////////////////////////////////////////////
void OgreDistortionPass::PreRender()
{
}

//////////////////////////////////////////////////
void OgreDistortionPass::CreateRenderPass()
{
  if (!this->ogreCamera)
  {
    ignerr << "No camera set for applying Distortion Pass" << std::endl;
    return;
  }

  if (this->distortionInstance)
  {
    ignerr << "Distortion pass already created. " << std::endl;
    return;
  }

  // If no distortion is required, immediately return.
  if (ignition::math::equal(this->k1, 0.0) &&
      ignition::math::equal(this->k2, 0.0) &&
      ignition::math::equal(this->k3, 0.0) &&
      ignition::math::equal(this->p1, 0.0) &&
      ignition::math::equal(this->p2, 0.0))
  {
    return;
  }

  float viewportWidth = this->ogreCamera->getViewport()->getWidth();
  float viewportHeight = this->ogreCamera->getViewport()->getHeight();

  // seems to work best with a square distortion map texture
  unsigned int texSide = viewportHeight > viewportWidth ? viewportHeight :
      viewportWidth;
  // calculate focal length from largest fov
  const double fov = viewportHeight > viewportWidth ?
      this->ogreCamera->getFOVy().valueRadians() :
      (this->ogreCamera->getFOVy().valueRadians() *
      this->ogreCamera->getAspectRatio());
  const double focalLength = texSide/(2*tan(fov/2));
  this->distortionTexWidth = texSide;
  this->distortionTexHeight = texSide;
  unsigned int imageSize =
      this->distortionTexWidth * this->distortionTexHeight;
  double colStepSize = 1.0 / this->distortionTexWidth;
  double rowStepSize = 1.0 / this->distortionTexHeight;

  // Half step-size vector to add to the value being placed in distortion map.
  // Necessary for compositor to correctly interpolate pixel values.
  const auto halfTexelSize =
      0.5 * ignition::math::Vector2d(rowStepSize, colStepSize);

  // initialize distortion map
  this->distortionMap.resize(imageSize);
  for (unsigned int i = 0; i < this->distortionMap.size(); ++i)
  {
    this->distortionMap[i] = -1;
  }

  ignition::math::Vector2d distortionCenterCoordinates(
      this->lensCenter.X() * this->distortionTexWidth,
      this->lensCenter.Y() * this->distortionTexWidth);

  // declare variables before the loop
  const auto unsetPixelVector =  ignition::math::Vector2d(-1, -1);
  ignition::math::Vector2d normalizedLocation,
      distortedLocation,
      newDistortedCoordinates,
      currDistortedCoordinates;
  unsigned int distortedIdx,
      distortedCol,
      distortedRow;
  double normalizedColLocation, normalizedRowLocation;

  // fill the distortion map
  for (unsigned int mapRow = 0; mapRow < this->distortionTexHeight;
    ++mapRow)
  {
    normalizedRowLocation = mapRow*rowStepSize;
    for (unsigned int mapCol = 0; mapCol < this->distortionTexWidth;
      ++mapCol)
    {
      normalizedColLocation = mapCol*colStepSize;

      normalizedLocation[0] = normalizedColLocation;
      normalizedLocation[1] = normalizedRowLocation;

      if (this->legacyMode)
      {
        distortedLocation = this->Distort(
            normalizedLocation,
            this->lensCenter,
            this->k1, this->k2, this->k3,
            this->p1, this->p2);
      }
      else
      {
        distortedLocation = this->Distort(
            normalizedLocation,
            this->lensCenter,
            this->k1, this->k2, this->k3,
            this->p1, this->p2,
            this->distortionTexWidth,
            focalLength);
      }

      // compute the index in the distortion map
      distortedCol = round(distortedLocation.X() * 
        this->distortionTexWidth);
      distortedRow = round(distortedLocation.Y() *
        this->distortionTexHeight);

      // Note that the following makes sure that, for significant distortions,
      // there is not a problem where the distorted image seems to fold over
      // itself. This is accomplished by favoring pixels closer to the center
      // of distortion, and this change applies to both the legacy and
      // nonlegacy distortion modes.

      // Make sure the distorted pixel is within the texture dimensions
      if (distortedCol < this->distortionTexWidth &&
          distortedRow < this->distortionTexHeight)
      {
        distortedIdx = distortedRow * this->distortionTexWidth +
          distortedCol;

        // check if the index has already been set
        if (this->distortionMap[distortedIdx] != unsetPixelVector)
        {
          // grab current coordinates that map to this destination
          currDistortedCoordinates =
            this->distortionMap[distortedIdx] *
            this->distortionTexWidth;

          // grab new coordinates to map to
          newDistortedCoordinates[0] = mapCol;
          newDistortedCoordinates[1] = mapRow;

          // use the new mapping if it is closer to the center of the distortion
          if (newDistortedCoordinates.Distance(distortionCenterCoordinates) <
              currDistortedCoordinates.Distance(distortionCenterCoordinates))
          {
            this->distortionMap[distortedIdx] = normalizedLocation +
              halfTexelSize;
          }
        }
        else
        {
          this->distortionMap[distortedIdx] = normalizedLocation +
            halfTexelSize;
        }
      }
      // else: mapping is outside of the image bounds.
      // This is expected and normal to ensure
      // no black borders; carry on
    }
  }

  // set up the distortion instance
  this->distortionMaterial =
    Ogre::MaterialManager::getSingleton().getByName(
        "Distortion");
  this->distortionMaterial =
    this->distortionMaterial->clone(
        this->ogreCamera->getName() + "_Distortion");

  // create the distortion map texture for the distortion instance
  std::string texName = this->ogreCamera->getName() + "_distortionTex";
  Ogre::TexturePtr renderTexture =
      Ogre::TextureManager::getSingleton().createManual(
          texName,
          "General",
          Ogre::TEX_TYPE_2D,
          this->distortionTexWidth,
          this->distortionTexHeight,
          0,
          Ogre::PF_FLOAT32_RGB);
  Ogre::HardwarePixelBufferSharedPtr pixelBuffer = renderTexture->getBuffer();

  // fill the distortion map, while interpolating to fill dead pixels
  pixelBuffer->lock(Ogre::HardwareBuffer::HBL_NORMAL);
  const Ogre::PixelBox &pixelBox = pixelBuffer->getCurrentLock();

#if OGRE_VERSION_MAJOR > 1 || OGRE_VERSION_MINOR >= 11
  // Ogre 1.11 changed Ogre::PixelBox::data from void* to uchar*, hence
  // reinterpret_cast is required here. static_cast is not allowed between
  // pointers of unrelated types (see, for instance, Standard § 3.9.1
  // Fundamental types)
  float *pDest = reinterpret_cast<float *>(pixelBox.data);
#else
  float *pDest = static_cast<float *>(pixelBox.data);
#endif

  for (unsigned int i = 0; i < this->distortionTexHeight; ++i)
  {
    for (unsigned int j = 0; j < this->distortionTexWidth; ++j)
    {
      ignition::math::Vector2d vec =
          this->distortionMap[i*this->distortionTexWidth+j];

      // perform interpolation on-the-fly:
      // check for empty mapping within the region and correct it by
      // interpolating the eight neighboring distortion map values.

      if (vec.X() < -0.5 && vec.Y() < -0.5)
      {
        ignition::math::Vector2d left =
            this->DistortionMapValueClamped(j-1, i);
        ignition::math::Vector2d right =
            this->DistortionMapValueClamped(j+1, i);
        ignition::math::Vector2d bottom =
            this->DistortionMapValueClamped(j, i+1);
        ignition::math::Vector2d top =
            this->DistortionMapValueClamped(j, i-1);

        ignition::math::Vector2d topLeft =
            this->DistortionMapValueClamped(j-1, i-1);
        ignition::math::Vector2d topRight =
            this->DistortionMapValueClamped(j+1, i-1);
        ignition::math::Vector2d bottomLeft =
            this->DistortionMapValueClamped(j-1, i+1);
        ignition::math::Vector2d bottomRight =
            this->DistortionMapValueClamped(j+1, i+1);


        ignition::math::Vector2d interpolated;
        double divisor = 0;
        if (right.X() > -0.5)
        {
          divisor++;
          interpolated += right;
        }
        if (left.X() > -0.5)
        {
          divisor++;
          interpolated += left;
        }
        if (top.X() > -0.5)
        {
          divisor++;
          interpolated += top;
        }
        if (bottom.X() > -0.5)
        {
          divisor++;
          interpolated += bottom;
        }

        if (bottomRight.X() > -0.5)
        {
          divisor += 0.707;
          interpolated += bottomRight * 0.707;
        }
        if (bottomLeft.X() > -0.5)
        {
          divisor += 0.707;
          interpolated += bottomLeft * 0.707;
        }
        if (topRight.X() > -0.5)
        {
          divisor += 0.707;
          interpolated += topRight * 0.707;
        }
        if (topLeft.X() > -0.5)
        {
          divisor += 0.707;
          interpolated += topLeft * 0.707;
        }

        if (divisor > 0.5)
        {
          interpolated /= divisor;
        }
        *pDest++ = ignition::math::clamp(interpolated.X(), 0.0, 1.0);
        *pDest++ = ignition::math::clamp(interpolated.Y(), 0.0, 1.0);
      }
      else
      {
        *pDest++ = vec.X();
        *pDest++ = vec.Y();
      }

      // Z coordinate
      *pDest++ = 0;
    }
  }
  pixelBuffer->unlock();

  this->CalculateAndApplyDistortionScale();

  // set up the distortion map texture to be used in the pixel shader.
  this->distortionMaterial->getTechnique(0)->getPass(0)->
      createTextureUnitState(texName, 1);

  // create compositor instance
  this->distortionInstance =
    Ogre::CompositorManager::getSingleton().addCompositor(
        this->ogreCamera->getViewport(), "RenderPass/Distortion");
  this->distortionInstance->getTechnique()->getOutputTargetPass()->
      getPass(0)->setMaterial(this->distortionMaterial);

  this->distortionInstance->setEnabled(this->enabled);
}

//////////////////////////////////////////////////
void OgreDistortionPass::Destroy()
{
}

//////////////////////////////////////////////////
ignition::math::Vector2d OgreDistortionPass::Distort(
    const ignition::math::Vector2d &_in,
    const ignition::math::Vector2d &_center, double _k1, double _k2, double _k3,
    double _p1, double _p2)
{
  return Distort(_in, _center, _k1, _k2, _k3, _p1, _p2, 1u, 1.0);
}

//////////////////////////////////////////////////
ignition::math::Vector2d OgreDistortionPass::Distort(
    const ignition::math::Vector2d &_in,
    const ignition::math::Vector2d &_center, double _k1, double _k2, double _k3,
    double _p1, double _p2, unsigned int _width, double _f)
{
  // apply Brown's distortion model, see
  // http://en.wikipedia.org/wiki/Distortion_%28optics%29#Software_correction

  ignition::math::Vector2d normalized2d = (_in - _center)*(_width/_f);
  ignition::math::Vector3d normalized(normalized2d.X(), normalized2d.Y(), 0);
  double rSq = normalized.X() * normalized.X() +
               normalized.Y() * normalized.Y();

  // radial
  ignition::math::Vector3d dist = normalized * (1.0 +
      _k1 * rSq +
      _k2 * rSq * rSq +
      _k3 * rSq * rSq * rSq);

  // tangential
  dist.X() += _p2 * (rSq + 2 * (normalized.X()*normalized.X())) +
      2 * _p1 * normalized.X() * normalized.Y();
  dist.Y() += _p1 * (rSq + 2 * (normalized.Y()*normalized.Y())) +
      2 * _p2 * normalized.X() * normalized.Y();

  return ((_center*_width) +
    ignition::math::Vector2d(dist.X(), dist.Y())*_f)/_width;
}

//////////////////////////////////////////////////
ignition::math::Vector2d
    OgreDistortionPass::DistortionMapValueClamped(
    const int x, const int y) const
{
  if (x < 0 || x >= static_cast<int>(this->distortionTexWidth) ||
      y < 0 || y >= static_cast<int>(this->distortionTexHeight))
  {
    return ignition::math::Vector2d(-1, -1);
  }
  ignition::math::Vector2d res =
      this->distortionMap[y*this->distortionTexWidth+x];
  return res;
}

//////////////////////////////////////////////////
void OgreDistortionPass::CalculateAndApplyDistortionScale()
{
  if (this->distortionMaterial.isNull())
    return;

  // Scale up image if cropping enabled and valid
  if (this->distortionCrop && this->k1 < 0)
  {
    // I believe that if not used with a square distortion texture, this
    // calculation will result in stretching of the final output image.
    ignition::math::Vector2d boundA = this->Distort(
        ignition::math::Vector2d(0, 0),
        this->lensCenter,
        this->k1, this->k2, this->k3,
        this->p1, this->p2);
    ignition::math::Vector2d boundB = this->Distort(
        ignition::math::Vector2d(1, 1),
        this->lensCenter,
        this->k1, this->k2, this->k3,
        this->p1, this->p2);
    ignition::math::Vector2d newScale = boundB - boundA;
    // If distortionScale is extremely small, don't crop
    if (newScale.X() < 1e-7 || newScale.Y() < 1e-7)
    {
          ignerr << "Distortion model attempted to apply a scale parameter of ("
                << this->distortionScale.X() << ", "
                << this->distortionScale.Y()
                << ", which is invalid.\n";
    }
    else
      this->distortionScale = newScale;
  }
  // Otherwise no scaling
  else
    this->distortionScale = ignition::math::Vector2d(1, 1);
}

IGN_RENDERING_REGISTER_RENDER_PASS(OgreDistortionPass, DistortionPass)
