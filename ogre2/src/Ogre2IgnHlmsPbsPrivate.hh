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
#ifndef IGNITION_RENDERING_OGRE2_OGRE2IGNHLMSPBSPRIVATE_HH_
#define IGNITION_RENDERING_OGRE2_OGRE2IGNHLMSPBSPRIVATE_HH_

#include "ignition/rendering/config.hh"
#include "ignition/rendering/ogre2/Export.hh"

#include <Hlms/Pbs/OgreHlmsPbs.h>
#include <OgreHlmsListener.h>

namespace Ogre
{
  /// \enum IgnOgreRenderingMode
  /// \brief Rendering modes so that IgnHlms implementations
  /// follow alternate code paths or extra customizations
  /// when they're enabled
  enum IGNITION_RENDERING_OGRE2_HIDDEN IgnOgreRenderingMode
  {
    /// \brief Normal rendering
    IORM_NORMAL,

    /// \brief Render a solid color explicit per object.
    /// Used by e.g. Segmentation camera mode
    IORM_SOLID_COLOR,

    /// \brief Total number of rendering modes
    IORM_COUNT,
  };

  class IGNITION_RENDERING_OGRE2_HIDDEN IgnHlmsPbs : public HlmsPbs,
                                                     public HlmsListener
  {
    public: IgnHlmsPbs(Archive *dataFolder, ArchiveVec *libraryFolders);
    public: virtual ~IgnHlmsPbs() override;

    public: using HlmsPbs::preparePassHash;

    public: virtual void preparePassHash(
        const CompositorShadowNode *_shadowNode,
        bool _casterPass,
        bool _dualParaboloid,
        SceneManager *_sceneManager,
        Hlms *_hlms) override;

    /// \brief Override HlmsListener::hlmsTypeChanged so we can
    /// bind buffers which carry per-object data when in IORM_SOLID_COLOR
    /// \param _casterPass true if this is a caster pass
    /// \param _commandBuffer command buffer so we can add commands
    /// \param _datablock material of the object that caused IgnHlmsPbs to be
    /// bound again
    public: virtual void hlmsTypeChanged(
        bool _casterPass,
        CommandBuffer *_commandBuffer,
        const HlmsDatablock *_datablock) override;

    public: virtual uint32 fillBuffersForV1(
      const HlmsCache *_cache,
      const QueuedRenderable &_queuedRenderable,
      bool _casterPass, uint32 _lastCacheHash,
      CommandBuffer *_commandBuffer ) override;

    public: virtual uint32 fillBuffersForV2(
      const HlmsCache *_cache,
      const QueuedRenderable &_queuedRenderable,
      bool _casterPass, uint32 _lastCacheHash,
      CommandBuffer *_commandBuffer ) override;

    /// \brief Binds currPerObjectDataBuffer to the right slot.
    /// Does nothing if it's nullptr
    /// \param _commandBuffer Cmd buffer to bind to
    /// \internal
    private: void BindObjectDataBuffer(CommandBuffer *_commandBuffer);

    /// \brief Vector of buffers holding per-object data.
    /// When one runs out, we push a new one. On the next frame
    /// we reuse them all from 0
    ///
    /// It also calls BindObjectDataBuffer if a new one must be created
    /// \param _instanceIdx Instance to write to
    /// \return Pointer to write data for that instance
    /// \internal
    private: float *MapObjectDataBufferFor(uint32_t _instanceIdx,
                                           CommandBuffer *_commandBuffer);

    /// \brief Unmaps the current buffer holding per-object data from memory
    private: void UnmapObjectDataBuffer();

    public: virtual void preCommandBufferExecution(
        CommandBuffer *_commandBuffer) override;

    public: virtual void frameEnded() override;

    /// \brief Vector of buffers holding per-object data.
    /// When one runs out, we push a new one. On the next frame
    /// we reuse them all from 0
    /// \internal
    private: std::vector<ConstBufferPacked *> perObjectDataBuffers;

    /// \brief The slot where to bind currPerObjectDataBuffer
    /// \internal
    private: uint16_t perObjectDataBufferSlot = 0u;

    /// \brief The buffer currently use. Can be nullptr
    /// It is contained in perObjectDataBuffers
    /// \internal
    private: ConstBufferPacked *currPerObjectDataBuffer = nullptr;

    /// \brief The last content of mCurrentConstBuffer. If it changes
    /// we need a new currPerObjectDataBuffer too (because drawId will
    /// be reset from 0)
    /// \internal
    private: ConstBufferPacked *lastMainConstBuffer = nullptr;

    /// \brief The mapped contents of currPerObjectDataBuffer
    /// \internal
    private: float *currPerObjectDataPtr = nullptr;

    /// \brief See IgnOgreRenderingMode. Public variable.
    /// Modifying it takes change on the next render
    public: IgnOgreRenderingMode ignOgreRenderingMode = IORM_NORMAL;
  };
}  // namespace Ogre

#endif
