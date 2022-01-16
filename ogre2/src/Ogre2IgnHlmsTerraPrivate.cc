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

#include "Ogre2IgnHlmsTerraPrivate.hh"

#include <ignition/common/Console.hh>
#include <ignition/common/Filesystem.hh>
#include <ignition/common/Util.hh>

#include "Terra/Terra.h"

#ifdef _MSC_VER
#  pragma warning(push, 0)
#endif
#include <CommandBuffer/OgreCbShaderBuffer.h>
#include <CommandBuffer/OgreCommandBuffer.h>
#include <OgreRenderQueue.h>
#include <Vao/OgreConstBufferPacked.h>
#include <Vao/OgreVaoManager.h>
#ifdef _MSC_VER
#  pragma warning(pop)
#endif

using namespace ignition;
using namespace rendering;

namespace Ogre
{
  /// \brief The slot where to bind currPerObjectDataBuffer
  /// HlmsPbs might consume slot 3, so we always use slot 4 for simplicity
  /// \internal
  static const uint16 kPerObjectDataBufferSlot = 4u;

  Ogre2IgnHlmsTerra::Ogre2IgnHlmsTerra(
    Archive *dataFolder, ArchiveVec *libraryFolders,
    Ogre2IgnHlmsSphericalClipMinDistance *_sphericalClipMinDistance) :
    HlmsTerra(dataFolder, libraryFolders)
  {
    this->customizations.push_back(_sphericalClipMinDistance);
  }

  /////////////////////////////////////////////////
  void Ogre2IgnHlmsTerra::preparePassHash(
    const CompositorShadowNode *_shadowNode, bool _casterPass,
    bool _dualParaboloid, SceneManager *_sceneManager, Hlms *_hlms)
  {
    if (!_casterPass &&
        (this->ignOgreRenderingMode == IORM_SOLID_COLOR ||
         this->ignOgreRenderingMode == IORM_SOLID_THERMAL_COLOR_TEXTURED))
    {
      _hlms->_setProperty("ign_render_solid_color", 1);

      if (this->ignOgreRenderingMode == IORM_SOLID_THERMAL_COLOR_TEXTURED)
        _hlms->_setProperty("ign_render_solid_color_textured", 1);
    }

    // Allow additional listener-only customizations to inject their stuff
    for (Ogre::HlmsListener *listener : this->customizations)
    {
      listener->preparePassHash(_shadowNode, _casterPass, _dualParaboloid,
                                _sceneManager, _hlms);
    }
  }

  /////////////////////////////////////////////////
  uint32 Ogre2IgnHlmsTerra::getPassBufferSize(
    const Ogre::CompositorShadowNode *_shadowNode, bool _casterPass,
    bool _dualParaboloid, Ogre::SceneManager *_sceneManager) const
  {
    uint32 bufferSize = 0u;

    // Allow additional listener-only customizations to inject their stuff
    for (Ogre::HlmsListener *listener : this->customizations)
    {
      bufferSize += listener->getPassBufferSize(_shadowNode, _casterPass,
                                                _dualParaboloid, _sceneManager);
    }
    return bufferSize;
  }

  /////////////////////////////////////////////////
  float *Ogre2IgnHlmsTerra::preparePassBuffer(
    const Ogre::CompositorShadowNode *_shadowNode, bool _casterPass,
    bool _dualParaboloid, Ogre::SceneManager *_sceneManager,
    float *_passBufferPtr)
  {
    // Allow additional listener-only customizations to inject their stuff
    for (Ogre::HlmsListener *listener : this->customizations)
    {
      _passBufferPtr =
        listener->preparePassBuffer(_shadowNode, _casterPass, _dualParaboloid,
                                    _sceneManager, _passBufferPtr);
    }
    return _passBufferPtr;
  }

  /////////////////////////////////////////////////
  void Ogre2IgnHlmsTerra::shaderCacheEntryCreated(
    const String &_shaderProfile, const HlmsCache *_hlmsCacheEntry,
    const HlmsCache &_passCache, const HlmsPropertyVec &_properties,
    const QueuedRenderable &_queuedRenderable)
  {
    // Allow additional listener-only customizations to inject their stuff
    for (Ogre::HlmsListener *listener : this->customizations)
    {
      listener->shaderCacheEntryCreated(_shaderProfile, _hlmsCacheEntry,
                                        _passCache, _properties,
                                        _queuedRenderable);
    }
  }

  /////////////////////////////////////////////////
  void Ogre2IgnHlmsTerra::notifyPropertiesMergedPreGenerationStep()
  {
    HlmsTerra::notifyPropertiesMergedPreGenerationStep();

    setProperty("IgnPerObjectDataSlot", kPerObjectDataBufferSlot);
  }

  /////////////////////////////////////////////////
  void Ogre2IgnHlmsTerra::hlmsTypeChanged(bool _casterPass,
                                          CommandBuffer *_commandBuffer,
                                          const HlmsDatablock *_datablock)
  {
    // Allow additional listener-only customizations to inject their stuff
    for (Ogre::HlmsListener *listener : this->customizations)
    {
      listener->hlmsTypeChanged(_casterPass, _commandBuffer, _datablock);
    }

    if (_casterPass ||
        (this->ignOgreRenderingMode != IORM_SOLID_COLOR &&
         this->ignOgreRenderingMode != IORM_SOLID_THERMAL_COLOR_TEXTURED))
    {
      return;
    }

    this->BindObjectDataBuffer(_commandBuffer, kPerObjectDataBufferSlot);
  }

  /////////////////////////////////////////////////
  uint32 Ogre2IgnHlmsTerra::fillBuffersForV1(
    const HlmsCache *_cache, const QueuedRenderable &_queuedRenderable,
    bool _casterPass, uint32 _lastCacheHash, CommandBuffer *_commandBuffer)
  {
    const uint32 instanceIdx = HlmsTerra::fillBuffersForV1(
      _cache, _queuedRenderable, _casterPass, _lastCacheHash, _commandBuffer);

    if ((this->ignOgreRenderingMode == IORM_SOLID_COLOR ||
         this->ignOgreRenderingMode == IORM_SOLID_THERMAL_COLOR_TEXTURED) &&
        !_casterPass)
    {
      const Ogre::Terra *terra =
        static_cast<const Ogre::Terra *>(_queuedRenderable.movableObject);

      Vector4 customParam;
      try
      {
        customParam = terra->SolidColor(1u);
      }
      catch (ItemIdentityException &)
      {
        // This error can trigger for two reasons:
        //
        //  1. We forgot to call setSolidColor(1u, ...)
        //  2. This object should not be rendered and we should've called
        //     movableObject->setVisible(false) or use RenderQueue IDs
        //     or visibility flags to prevent rendering it
        ignerr << "A module is trying to render an object without "
                  "specifying a parameter. Please report this bug at "
                  "https://github.com/ignitionrobotics/ign-rendering/issues\n";
        throw;
      }
      float *dataPtr = this->MapObjectDataBufferFor(
        instanceIdx, _commandBuffer, this->mVaoManager, this->mConstBuffers,
        this->mCurrentConstBuffer, this->mStartMappedConstBuffer,
        kPerObjectDataBufferSlot);
      dataPtr[0] = customParam.x;
      dataPtr[1] = customParam.y;
      dataPtr[2] = customParam.z;

      if (this->ignOgreRenderingMode == IORM_SOLID_THERMAL_COLOR_TEXTURED &&
          terra->HasSolidColor(2u))
      {
        IGN_ASSERT(customParam.w >= 0.0f,
                   "customParam.w can't be negative for "
                   "IORM_SOLID_THERMAL_COLOR_TEXTURED");

        // Negate customParam.w to tell the shader we wish to multiply
        // against the diffuse texture. We substract 0.5f to avoid -0.0 = 0.0
        dataPtr[3] = -customParam.w - 0.5f;
      }
      else
      {
        dataPtr[3] = customParam.w;
      }
    }

    return instanceIdx;
  }

  /////////////////////////////////////////////////
  uint32 Ogre2IgnHlmsTerra::fillBuffersForV2(
    const HlmsCache *_cache, const QueuedRenderable &_queuedRenderable,
    bool _casterPass, uint32 _lastCacheHash, CommandBuffer *_commandBuffer)
  {
    const uint32 instanceIdx = HlmsTerra::fillBuffersForV2(
      _cache, _queuedRenderable, _casterPass, _lastCacheHash, _commandBuffer);

    if ((this->ignOgreRenderingMode == IORM_SOLID_COLOR ||
         this->ignOgreRenderingMode == IORM_SOLID_THERMAL_COLOR_TEXTURED) &&
        !_casterPass)
    {
      const Ogre::Terra *terra =
        static_cast<const Ogre::Terra *>(_queuedRenderable.movableObject);

      Vector4 customParam;
      try
      {
        customParam = terra->SolidColor(1u);
      }
      catch (ItemIdentityException &)
      {
        // This error can trigger for two reasons:
        //
        //  1. We forgot to call setSolidColor(1u, ...)
        //  2. This object should not be rendered and we should've called
        //     movableObject->setVisible(false) or use RenderQueue IDs
        //     or visibility flags to prevent rendering it
        ignerr << "A module is trying to render an object without "
                  "specifying a parameter. Please report this bug at "
                  "https://github.com/ignitionrobotics/ign-rendering/issues\n";
        throw;
      }
      float *dataPtr = this->MapObjectDataBufferFor(
        instanceIdx, _commandBuffer, this->mVaoManager, this->mConstBuffers,
        this->mCurrentConstBuffer, this->mStartMappedConstBuffer,
        kPerObjectDataBufferSlot);
      dataPtr[0] = customParam.x;
      dataPtr[1] = customParam.y;
      dataPtr[2] = customParam.z;
      dataPtr[3] = customParam.w;

      if (this->ignOgreRenderingMode == IORM_SOLID_THERMAL_COLOR_TEXTURED &&
          terra->HasSolidColor(2u))
      {
        IGN_ASSERT(customParam.w >= 0.0f,
                   "customParam.w can't be negative for "
                   "IORM_SOLID_THERMAL_COLOR_TEXTURED");

        // Negate customParam.w to tell the shader we wish to multiply
        // against the diffuse texture. We substract 0.5f to avoid -0.0 = 0.0
        dataPtr[3] = -customParam.w - 0.5f;
      }
      else
      {
        dataPtr[3] = customParam.w;
      }
    }

    return instanceIdx;
  }

  /////////////////////////////////////////////////
  void Ogre2IgnHlmsTerra::preCommandBufferExecution(
    CommandBuffer *_commandBuffer)
  {
    this->UnmapObjectDataBuffer();
    HlmsTerra::preCommandBufferExecution(_commandBuffer);
  }

  /////////////////////////////////////////////////
  void Ogre2IgnHlmsTerra::frameEnded()
  {
    HlmsTerra::frameEnded();

    this->currPerObjectDataBuffer = nullptr;
    this->lastMainConstBuffer = nullptr;
    this->currPerObjectDataPtr = nullptr;
  }

  /////////////////////////////////////////////////
  void Ogre2IgnHlmsTerra::GetDefaultPaths(String &_outDataFolderPath,
                                          StringVector &_outLibraryFoldersPaths)
  {
    HlmsTerra::getDefaultPaths(_outDataFolderPath, _outLibraryFoldersPaths);

    _outLibraryFoldersPaths.push_back(
      common::joinPaths("Hlms", "Ignition", "SolidColor"));
    _outLibraryFoldersPaths.push_back(
      common::joinPaths("Hlms", "Ignition", "SphericalClipMinDistance"));
    _outLibraryFoldersPaths.push_back(
      common::joinPaths("Hlms", "Terra", "ign"));
    _outLibraryFoldersPaths.push_back(
      common::joinPaths("Hlms", "Ignition", "Pbs"));
  }
}  // namespace Ogre
