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

#include "Ogre2IgnHlmsSharedPrivate.hh"

#ifdef _MSC_VER
  #pragma warning(push, 0)
#endif
#include <Hlms/Pbs/OgreHlmsPbs.h>
#include <OgreHlmsListener.h>
#ifdef _MSC_VER
  #pragma warning(pop)
#endif

namespace Ogre
{
  class IGNITION_RENDERING_OGRE2_HIDDEN IgnHlmsPbs
    : public HlmsPbs,
      public HlmsListener,
      public ignition::rendering::IgnHlmsShared
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
  };
}  // namespace Ogre

#endif
