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

#include "Ogre2IgnHlmsUnlitPrivate.hh"

#include <ignition/common/Filesystem.hh>
#include <ignition/common/Util.hh>

#include <CommandBuffer/OgreCbShaderBuffer.h>
#include <CommandBuffer/OgreCommandBuffer.h>
#include <OgreRenderQueue.h>
#include <Vao/OgreConstBufferPacked.h>
#include <Vao/OgreVaoManager.h>

using namespace ignition;
using namespace rendering;

namespace Ogre
{
  /// \brief The slot where to bind currPerObjectDataBuffer
  /// Note it's different from HlmsPbs!
  /// \internal
  static const uint16 kPerObjectDataBufferSlot = 3u;

  IgnHlmsUnlit::IgnHlmsUnlit(Archive *dataFolder, ArchiveVec *libraryFolders) :
    HlmsUnlit(dataFolder, libraryFolders)
  {
  }

  /////////////////////////////////////////////////
  void IgnHlmsUnlit::preparePassHash(
    const CompositorShadowNode * /*_shadowNode*/, bool _casterPass,
    bool /*_dualParaboloid*/, SceneManager * /*_sceneManager*/, Hlms *_hlms)
  {
    if (!_casterPass && this->ignOgreRenderingMode == IORM_SOLID_COLOR)
    {
      _hlms->_setProperty("ign_render_solid_color", 1);
    }
  }

  /////////////////////////////////////////////////
  void IgnHlmsUnlit::notifyPropertiesMergedPreGenerationStep()
  {
    HlmsUnlit::notifyPropertiesMergedPreGenerationStep();

    setProperty("IgnPerObjectDataSlot", kPerObjectDataBufferSlot);
  }

  /////////////////////////////////////////////////
  void IgnHlmsUnlit::hlmsTypeChanged(bool _casterPass,
                                     CommandBuffer *_commandBuffer,
                                     const HlmsDatablock * /*_datablock*/)
  {
    if (_casterPass || this->ignOgreRenderingMode != IORM_SOLID_COLOR)
    {
      return;
    }

    this->BindObjectDataBuffer(_commandBuffer, kPerObjectDataBufferSlot);
  }

  /////////////////////////////////////////////////
  uint32 IgnHlmsUnlit::fillBuffersForV1(
    const HlmsCache *_cache, const QueuedRenderable &_queuedRenderable,
    bool _casterPass, uint32 _lastCacheHash, CommandBuffer *_commandBuffer)
  {
    const uint32 instanceIdx = HlmsUnlit::fillBuffersForV1(
      _cache, _queuedRenderable, _casterPass, _lastCacheHash, _commandBuffer);

    if (this->ignOgreRenderingMode == IORM_SOLID_COLOR && !_casterPass)
    {
      Vector4 customParam =
        _queuedRenderable.renderable->getCustomParameter(1u);
      float *dataPtr = this->MapObjectDataBufferFor(
        instanceIdx, _commandBuffer, this->mVaoManager, this->mConstBuffers,
        this->mCurrentConstBuffer, this->mStartMappedConstBuffer,
        kPerObjectDataBufferSlot);
      dataPtr[0] = customParam.x;
      dataPtr[1] = customParam.y;
      dataPtr[2] = customParam.z;
      dataPtr[3] = customParam.w;
    }

    return instanceIdx;
  }

  /////////////////////////////////////////////////
  uint32 IgnHlmsUnlit::fillBuffersForV2(
    const HlmsCache *_cache, const QueuedRenderable &_queuedRenderable,
    bool _casterPass, uint32 _lastCacheHash, CommandBuffer *_commandBuffer)
  {
    const uint32 instanceIdx = HlmsUnlit::fillBuffersForV2(
      _cache, _queuedRenderable, _casterPass, _lastCacheHash, _commandBuffer);

    if (this->ignOgreRenderingMode == IORM_SOLID_COLOR && !_casterPass)
    {
      Vector4 customParam =
        _queuedRenderable.renderable->getCustomParameter(1u);
      float *dataPtr = this->MapObjectDataBufferFor(
        instanceIdx, _commandBuffer, this->mVaoManager, this->mConstBuffers,
        this->mCurrentConstBuffer, this->mStartMappedConstBuffer,
        kPerObjectDataBufferSlot);
      dataPtr[0] = customParam.x;
      dataPtr[1] = customParam.y;
      dataPtr[2] = customParam.z;
      dataPtr[3] = customParam.w;
    }

    return instanceIdx;
  }

  /////////////////////////////////////////////////
  void IgnHlmsUnlit::preCommandBufferExecution(CommandBuffer *_commandBuffer)
  {
    this->UnmapObjectDataBuffer();
    HlmsUnlit::preCommandBufferExecution(_commandBuffer);
  }

  /////////////////////////////////////////////////
  void IgnHlmsUnlit::frameEnded()
  {
    HlmsUnlit::frameEnded();

    this->currPerObjectDataBuffer = nullptr;
    this->lastMainConstBuffer = nullptr;
    this->currPerObjectDataPtr = nullptr;
  }

  /////////////////////////////////////////////////
  void IgnHlmsUnlit::GetDefaultPaths(String &_outDataFolderPath,
                                     StringVector &_outLibraryFoldersPaths)
  {
    HlmsUnlit::getDefaultPaths(_outDataFolderPath, _outLibraryFoldersPaths);

    _outLibraryFoldersPaths.push_back(
      common::joinPaths("Hlms", "Ignition", "SolidColor"));
    _outLibraryFoldersPaths.push_back(
      common::joinPaths("Hlms", "Ignition", "SphericalClipMinDistance"));
    // For now use the same template as Pbs since they're the same code
    // We'll change it if they need to diverge
    _outLibraryFoldersPaths.push_back(
      common::joinPaths("Hlms", "Ignition", "Pbs"));
  }
}  // namespace Ogre
