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
#include "ignition/rendering/ogre2/Ogre2IgnOgreRenderingMode.hh"

#include <Hlms/Pbs/OgreHlmsPbs.h>
#include <OgreHlmsListener.h>

namespace Ogre
{
  typedef ignition::rendering::IgnOgreRenderingMode IgnOgreRenderingMode;

  class IGNITION_RENDERING_OGRE2_HIDDEN IgnHlmsPbs : public HlmsPbs,
                                                     public HlmsListener
  {
    public: IgnHlmsPbs(Archive *dataFolder, ArchiveVec *libraryFolders);
    public: virtual ~IgnHlmsPbs() override;

    public: using HlmsPbs::preparePassHash;

    /// \brief Override HlmsListener to add customizations.
    /// We can't override HlmsPbs because adding properties before
    /// calling it will be cleared. And adding it afterwards is too late.
    /// The listener gets called right in the middle
    ///
    /// \param[in] _shadowNode see base class
    /// \param[in] _casterPass see base class
    /// \param[in] _dualParaboloid see base class
    /// \param[in] _sceneManager see base class
    /// \param[in] _hlms see base class
    public: virtual void preparePassHash(
        const CompositorShadowNode *_shadowNode,
        bool _casterPass,
        bool _dualParaboloid,
        SceneManager *_sceneManager,
        Hlms *_hlms) override;

    /// \brief Override to calculate which slots are used
    public: virtual void notifyPropertiesMergedPreGenerationStep() override;

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

    //// \brief Same as HlmsPbs::getDefaultPaths, but we also append
    /// our own paths with customizations
    /// \param[out] _outDataFolderPath Path (as a String) used for
    /// creating the "dataFolder" Archive the constructor will need
    /// \param[out] _outLibraryFoldersPaths
    /// Vector of String used for creating the ArchiveVector "libraryFolders"
    /// the constructor will need. Our own stuff is appended here
    public: static void GetDefaultPaths(String &_outDataFolderPath,
                                        StringVector &_outLibraryFoldersPaths);

    /// \brief Vector of buffers holding per-object data.
    /// When one runs out, we push a new one. On the next frame
    /// we reuse them all from 0
    /// \internal
    private: std::vector<ConstBufferPacked *> perObjectDataBuffers;

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
    public: IgnOgreRenderingMode ignOgreRenderingMode =
        ignition::rendering::IORM_NORMAL;
  };
}  // namespace Ogre

#endif
