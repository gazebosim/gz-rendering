/*
 * Copyright (C) 2015 Open Source Robotics Foundation
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
#ifndef IGNITION_RENDERING_CAMERA_HH_
#define IGNITION_RENDERING_CAMERA_HH_

#include <string>
#include <ignition/common/Event.hh>
#include "ignition/rendering/Image.hh"
#include "ignition/rendering/PixelFormat.hh"
#include "ignition/rendering/Sensor.hh"
#include "ignition/rendering/Scene.hh"

namespace ignition
{
  namespace rendering
  {
    /// \class Camera Camera.hh ignition/rendering/Camera.hh
    /// \brief Posable camera used for rendering the scene graph
    class IGNITION_VISIBLE Camera :
      public virtual Sensor
    {
      /// \brief Callback function for new frame render event listeners
      public: typedef std::function<void(const void*, unsigned int,
          unsigned int, unsigned int, const std::string&)> NewFrameListener;

      /// \brief Deconstructor
      public: virtual ~Camera() { }

      /// \brief Get the image width in pixels
      /// \return The image width in pixels
      public: virtual unsigned int ImageWidth() const = 0;

      /// \brief Set the image width in pixels
      /// \param[in] _width New image width in pixels
      public: virtual void SetImageWidth(unsigned int _width) = 0;

      /// \brief Get the image height in pixels
      /// \return The image height in pixels
      public: virtual unsigned int ImageHeight() const = 0;

      /// \brief Set the image height in pixels
      /// \param[in] _height New image height in pixels
      public: virtual void SetImageHeight(unsigned int _height) = 0;

      /// \brief Get the image pixel format. If the image pixel format has not
      /// been set with a valid value, PF_UNKNOWN will be returned.
      /// \return The image pixel format
      public: virtual PixelFormat ImageFormat() const = 0;

      /// \brief Set the image pixel format
      /// \param[in] _format New image pixel format
      public: virtual void SetImageFormat(PixelFormat _format) = 0;

      /// \brief Get the image channel depth
      /// \return The image channel depth
      public: virtual unsigned int ImageDepth() const = 0;

      /// \brief Get the total image memory size in bytes
      /// \return The image memory size in bytes
      public: virtual unsigned int ImageMemorySize() const = 0;

      /// \brief Get the camera's horizontal field-of-view
      /// \return Angle containing the camera's horizontal field-of-view
      public: virtual math::Angle HFOV() const = 0;

      /// \brief Set the camera's horizontal field-of-view
      /// \param[in] _angle Desired horizontal field-of-view
      public: virtual void SetHFOV(const math::Angle &_angle) = 0;

      /// \brief Get the camera's aspect ratio
      /// \return The camera's aspect ratio
      public: virtual double AspectRatio() const = 0;

      /// \brief Get the scene that the camera is rendering
      public: virtual ScenePtr GetScene() const = 0;

      /// \brief Get the visual at a given position in the camera render
      /// \param[in] _mousePos position of the mouse in the current camera
      // render
      public: virtual VisualPtr VisualAt(const math::Vector2i &_mousePos) = 0;

      /// \brief Set the camera's aspect ratio. This value determines the
      /// cameras vertical field-of-view. It is often the \code image_height /
      /// image_width \endcode but this is not necessarily true.
      /// \return The camera's aspect ratio
      public: virtual void SetAspectRatio(double _ratio) = 0;

      // TODO: add auto-aspect ratio

      /// \brief Get the level of anti-aliasing used during rendering
      /// \return The level of anti-aliasing used during rendering
      public: virtual unsigned int AntiAliasing() const = 0;

      /// \brief Set the level of anti-aliasing used during rendering. If a
      /// value of 0 is given, no anti-aliasing will be performed. Higher values
      /// can significantly slow-down rendering times, depending on the
      /// underlying render engine.
      /// \param[in] _aa Level of anti-aliasing used during rendering
      public: virtual void SetAntiAliasing(unsigned int _aa) = 0;

      /// \brief Renders the current scene using this camera. This function
      /// assumes PreRender() has already been called on the parent Scene,
      /// allowing the camera and the scene itself to prepare for rendering.
      public: virtual void Render() = 0;

      /// \brief Preforms any necessary final rendering work. Once rendering is
      /// complete the camera will alert any listeners of the new frame event.
      /// This function should only be called after a call to Render has
      /// successfully been executed.
      public: virtual void PostRender() = 0;

      /// \brief Created an empty image buffer for capturing images. The
      /// resulting image will have sufficient memory allocated for subsequent
      /// calls to this camera's Capture function. However, any changes to this
      /// cameras properties may invalidate the condition.
      /// \return A newly allocated Image for storing this cameras images
      public: virtual Image CreateImage() const = 0;

      /// \brief Renders a new frame and writes the results to the given image.
      /// This is a convenience function for single-camera scenes. It wraps the
      /// pre-render, render, post-render, and get-image calls into a single
      /// function. This should be used in applications with multiple cameras
      /// or multiple consumers of a single camera's images.
      /// \param[out] _image Output image buffer
      public: virtual void Capture(Image &_image) = 0;

      /// \brief Writes the last rendered image to the given image buffer. This
      /// function can be called multiple times after PostRender has been
      /// called, without rendering the scene again. Calling this function
      /// before a single image has been rendered will have undefined behavior.
      /// \param[out] _image Output image buffer
      public: virtual void Copy(Image &_image) const = 0;

      /// \brief Writes the previously rendered frame to a file. This function
      /// can be called multiple times after PostRender has been called,
      /// without rendering the scene again. Calling this function before a
      /// single image has been rendered will have undefined behavior.
      /// \param[in] _name Name of the output file
      public: virtual bool SaveFrame(const std::string &_name) = 0;

      /// \brief Subscribes a new listener to this camera's new frame event
      /// \param[in] _listener New camera listener callback
      public: virtual common::ConnectionPtr ConnectNewImageFrame(
                  NewFrameListener _listener) = 0;
    };
  }
}
#endif
