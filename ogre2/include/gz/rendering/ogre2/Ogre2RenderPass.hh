/*
 * Copyright (C) 2019 Open Source Robotics Foundation
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
#ifndef GZ_RENDERING_OGRE2_OGRE2RENDERPASS_HH_
#define GZ_RENDERING_OGRE2_OGRE2RENDERPASS_HH_

#include <memory>
#include <string>

#include "gz/rendering/base/BaseRenderPass.hh"
#include "gz/rendering/ogre2/Export.hh"
#include "gz/rendering/ogre2/Ogre2Object.hh"

namespace Ogre
{
  class CompositorWorkspace;
}

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    //
    // forward declaration
    class Ogre2RenderPassPrivate;

    /* \class Ogre2RenderPass Ogre2RenderPass.hh \
     * gz/rendering/ogre2/Ogre2RenderPass.hh
     */
    /// \brief Ogre2 Implementation of a render pass.
    ///
    /// The ogre2 compositor chain in gz-rendering is set up as follows:
    ///  * Base scene pass -> [0..N] RenderPass'es -> Final compositor pass.
    /// This is set up by Ogre2RenderTarget which loads the compositor workspace
    /// definition from script. The base scene pass node is responsible for
    /// rendering the initial scene and passes its output to any RenderPass'es
    /// that are added to the RenderTarget. Each RenderPass has its own ogre
    /// compositor node that receives the output from the previous RenderPass
    /// as input, applies its own pass over the input, and sends the result to
    /// the next RenderPass. Note that the Ogre2RenderPass class provides the
    /// node definition only and the actual node creation work is done in the
    /// Ogre2RenderTarget class when the whole workspace is constructed.
    ///
    /// A render pass node has two input channels: 0, the result of the
    /// previous node, and 1, the texture to write its own result to. It maps
    /// its result to output channel 0 and its input to output channel 1. A
    /// pass that returns true from RequiresDepth() gets a third input
    /// channel, kDepthInputChannel, with the depth of the scene.
    class GZ_RENDERING_OGRE2_VISIBLE Ogre2RenderPass :
      public BaseRenderPass<Ogre2Object>
    {
      /// \brief Constructor
      protected: Ogre2RenderPass();

      /// \brief Destructor
      public: virtual ~Ogre2RenderPass();

      // Documentation inherited.
      public: void Destroy() override;

      /// \brief Get the ogre compositor node definition name for this
      /// render pass
      public: std::string OgreCompositorNodeDefinitionName() const;

      /// \brief Notifies Ogre2RenderPass that a new workspace has been created
      /// \param[in] _workspace workspace that was created
      public: virtual void WorkspaceAdded(
            Ogre::CompositorWorkspace *_workspace);

      /// \brief Notifies Ogre2RenderPass that a workspace will be destroyed
      /// \param[in] _workspace workspace about to be destroyed
      public: virtual void WorkspaceRemoved(
            Ogre::CompositorWorkspace *_workspace);

      /// \brief Create the render pass using ogre compositor
      public: virtual void CreateRenderPass();

      /// \brief Input channel of the compositor node that receives the depth
      /// of the scene, for a pass that returns true from RequiresDepth().
      public: static constexpr unsigned int kDepthInputChannel = 2u;

      /// \brief Whether this pass reads the depth of the scene.
      ///
      /// When true, the camera's scene pass writes its depth to a texture of
      /// its own, and that texture is connected to input channel
      /// kDepthInputChannel of this pass's compositor node, whatever passes
      /// come before it. The node definition must declare that channel. The
      /// texture is the depth the scene was rendered with, resolved to one
      /// sample when the camera uses anti-aliasing, and holds the hardware
      /// depth `d` in its red channel. The depth along the camera's axis is
      /// `B / (d - A)`, with `(A, B)` from
      /// `Ogre::Camera::getProjectionParamsAB()`; that holds whether or not
      /// the render system uses reverse depth. Pixels where nothing was drawn
      /// keep the clear value, which maps to the far clip distance.
      ///
      /// Cameras and render windows provide the depth. Other sensors, e.g.
      /// depth and wide angle cameras, do not, and leave such a pass out of
      /// their chain with a warning. Cameras without such a pass render as
      /// before.
      /// \return True if the pass reads the depth of the scene. False by
      /// default.
      public: virtual bool RequiresDepth() const;

      /// \brief Name of the ogre compositor node definition
      protected: std::string ogreCompositorNodeDefName;

      /// \brief Pointer to private data class
      private: std::unique_ptr<Ogre2RenderPassPrivate> dataPtr;
    };
    }
  }
}
#endif
