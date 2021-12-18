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

#include "Ogre2IgnHlmsPbsPrivate.hh"

#include <ignition/common/Util.hh>

#include <CommandBuffer/OgreCbShaderBuffer.h>
#include <CommandBuffer/OgreCommandBuffer.h>
#include <OgreRenderQueue.h>
#include <Vao/OgreConstBufferPacked.h>
#include <Vao/OgreVaoManager.h>

namespace Ogre
{
  /// \brief The slot where to bind currPerObjectDataBuffer
  /// HlmsPbs might consume slot 3, so we always use slot 4 for simplicity
  /// \internal
  static const uint16 kPerObjectDataBufferSlot = 4u;

  IgnHlmsPbs::IgnHlmsPbs(Archive *dataFolder, ArchiveVec *libraryFolders) :
    HlmsPbs(dataFolder, libraryFolders)
  {
  }

  /////////////////////////////////////////////////
  IgnHlmsPbs::~IgnHlmsPbs() {}

  /////////////////////////////////////////////////
  void IgnHlmsPbs::preparePassHash(const CompositorShadowNode * /*_shadowNode*/,
                                   bool _casterPass, bool /*_dualParaboloid*/,
                                   SceneManager * /*_sceneManager*/,
                                   Hlms *_hlms)
  {
    if (!_casterPass &&
        this->ignOgreRenderingMode == ignition::rendering::IORM_SOLID_COLOR)
    {
      _hlms->_setProperty("ign_render_solid_color", 1);
    }
  }

  /////////////////////////////////////////////////
  void IgnHlmsPbs::notifyPropertiesMergedPreGenerationStep()
  {
    HlmsPbs::notifyPropertiesMergedPreGenerationStep();

    setProperty("IgnPerObjectDataSlot", kPerObjectDataBufferSlot);
  }

  /////////////////////////////////////////////////
  void IgnHlmsPbs::hlmsTypeChanged(bool _casterPass,
                                   CommandBuffer *_commandBuffer,
                                   const HlmsDatablock * /*_datablock*/)
  {
    if (_casterPass ||
        this->ignOgreRenderingMode != ignition::rendering::IORM_SOLID_COLOR)
    {
      return;
    }

    this->BindObjectDataBuffer(_commandBuffer);
  }

  /////////////////////////////////////////////////
  uint32 IgnHlmsPbs::fillBuffersForV1(const HlmsCache *_cache,
                                      const QueuedRenderable &_queuedRenderable,
                                      bool _casterPass, uint32 _lastCacheHash,
                                      CommandBuffer *_commandBuffer)
  {
    const uint32 instanceIdx = HlmsPbs::fillBuffersForV1(
      _cache, _queuedRenderable, _casterPass, _lastCacheHash, _commandBuffer);

    if (this->ignOgreRenderingMode == ignition::rendering::IORM_SOLID_COLOR &&
        !_casterPass)
    {
      Vector4 customParam =
        _queuedRenderable.renderable->getCustomParameter(1u);
      float *dataPtr = this->MapObjectDataBufferFor(
        instanceIdx, _commandBuffer, this->mVaoManager, this->mConstBuffers,
        this->mCurrentConstBuffer, this->mStartMappedConstBuffer);
      dataPtr[0] = customParam.x;
      dataPtr[1] = customParam.y;
      dataPtr[2] = customParam.z;
      dataPtr[3] = customParam.w;
    }

    return instanceIdx;
  }

  /////////////////////////////////////////////////
  uint32 IgnHlmsPbs::fillBuffersForV2(const HlmsCache *_cache,
                                      const QueuedRenderable &_queuedRenderable,
                                      bool _casterPass, uint32 _lastCacheHash,
                                      CommandBuffer *_commandBuffer)
  {
    const uint32 instanceIdx = HlmsPbs::fillBuffersForV2(
      _cache, _queuedRenderable, _casterPass, _lastCacheHash, _commandBuffer);

    if (this->ignOgreRenderingMode == ignition::rendering::IORM_SOLID_COLOR &&
        !_casterPass)
    {
      Vector4 customParam =
        _queuedRenderable.renderable->getCustomParameter(1u);
      float *dataPtr = this->MapObjectDataBufferFor(
        instanceIdx, _commandBuffer, this->mVaoManager, this->mConstBuffers,
        this->mCurrentConstBuffer, this->mStartMappedConstBuffer);
      dataPtr[0] = customParam.x;
      dataPtr[1] = customParam.y;
      dataPtr[2] = customParam.z;
      dataPtr[3] = customParam.w;
    }

    return instanceIdx;
  }

  /////////////////////////////////////////////////
  void IgnHlmsPbs::preCommandBufferExecution(CommandBuffer *_commandBuffer)
  {
    this->UnmapObjectDataBuffer();
    HlmsPbs::preCommandBufferExecution(_commandBuffer);
  }

  /////////////////////////////////////////////////
  void IgnHlmsPbs::frameEnded()
  {
    HlmsPbs::frameEnded();

    this->currPerObjectDataBuffer = nullptr;
    this->lastMainConstBuffer = nullptr;
    this->currPerObjectDataPtr = nullptr;
  }

  /////////////////////////////////////////////////
  void IgnHlmsPbs::GetDefaultPaths(String &_outDataFolderPath,
                                   StringVector &_outLibraryFoldersPaths)
  {
    HlmsPbs::getDefaultPaths(_outDataFolderPath, _outLibraryFoldersPaths);

    _outLibraryFoldersPaths.push_back("Hlms/Ignition/Pbs");
  }
}  // namespace Ogre
