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
#ifndef IGNITION_RENDERING_BASE_BASECAMERA_HH_
#define IGNITION_RENDERING_BASE_BASECAMERA_HH_

#include <string>

#include <ignition/math/Matrix3.hh>
#include <ignition/math/Pose3.hh>

#include <ignition/common/Event.hh>
#include <ignition/common/Console.hh>
#include <ignition/common/SuppressWarning.hh>

#include "ignition/rendering/Camera.hh"
#include "ignition/rendering/Image.hh"
#include "ignition/rendering/RenderEngine.hh"
#include "ignition/rendering/Scene.hh"
#include "ignition/rendering/base/BaseRenderTarget.hh"

namespace ignition
{
  namespace rendering
  {
    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
    template <class T>
    class BaseDepthCamera;

    template <class T>
    class BaseCamera :
      public virtual Camera,
      public virtual T
    {
      protected: BaseCamera();

      public: virtual ~BaseCamera();

      public: virtual unsigned int ImageWidth() const override;

      public: virtual void SetImageWidth(const unsigned int _width) override;

      public: virtual unsigned int ImageHeight() const override;

      public: virtual void SetImageHeight(const unsigned int _height) override;

      public: virtual PixelFormat ImageFormat() const override;

      public: virtual unsigned int ImageMemorySize() const override;

      public: virtual void SetImageFormat(PixelFormat _format) override;

      public: virtual math::Angle HFOV() const override;

      public: virtual void SetHFOV(const math::Angle &_hfov) override;

      public: virtual double AspectRatio() const override;

      public: virtual void SetAspectRatio(const double _ratio) override;

      public: virtual unsigned int AntiAliasing() const override;

      public: virtual void SetAntiAliasing(const unsigned int _aa) override;

      public: virtual double FarClipPlane() const override;

      public: virtual void SetFarClipPlane(const double _far) override;

      public: virtual double NearClipPlane() const override;

      public: virtual void SetNearClipPlane(const double _near) override;

      // Documentation inherited.
      public: virtual void PreRender() override;

      // Documentation inherited.
      public: virtual void PostRender() override;

      public: virtual void Update() override;

      public: virtual Image CreateImage() const override;

      public: virtual void Capture(Image &_image) override;

      public: virtual void Copy(Image &_image) const override;

      public: virtual bool SaveFrame(const std::string &_name) override;

      public: virtual common::ConnectionPtr ConnectNewImageFrame(
                  Camera::NewFrameListener _listener) override;

      public: virtual RenderWindowPtr CreateRenderWindow() override;

      // Documentation inherited.
      public: virtual VisualPtr VisualAt(const ignition::math::Vector2i
                  &_mousePos) override;

      // Documentation inherited.
      public: virtual math::Matrix4d ProjectionMatrix() const override;

      // Documentation inherited.
      public: virtual math::Matrix4d ViewMatrix() const override;

      // Documentation inherited.
      // \sa Camera::SetMaterial(const MaterialPtr &) override;
      public: virtual void SetMaterial(const MaterialPtr &_material)
                  override;

      // Documentation inherited.
      public: virtual void SetTrackTarget(const NodePtr &_target,
                  const math::Vector3d &_offset,
                  const bool _worldFrame) override;

      // Documentation inherited.
      public: virtual NodePtr TrackTarget() const override;

      // Documentation inherited.
      public: virtual void SetTrackOffset(const math::Vector3d &_offset)
                  override;

      // Documentation inherited.
      public: virtual math::Vector3d TrackOffset() const override;

      // Documentation inherited.
      public: virtual void SetTrackPGain(const double _pGain) override;

      // Documentation inherited.
      public: virtual double TrackPGain() const override;

      // Documentation inherited.
      public: virtual void SetFollowTarget(const NodePtr &_target,
                  const math::Vector3d &_Offset, const bool _worldFrame)
                  override;

      // Documentation inherited.
      public: virtual NodePtr FollowTarget() const override;

      // Documentation inherited.
      public: virtual void SetFollowOffset(const math::Vector3d &_offset)
                 override;

      // Documentation inherited.
      public: virtual math::Vector3d FollowOffset() const override;

      // Documentation inherited.
      public: virtual void SetFollowPGain(const double _pGain) override;

      // Documentation inherited.
      public: virtual double FollowPGain() const override;

      // Documentation inherited.
      public: virtual unsigned int RenderTextureGLId() const override;

      // Documentation inherited.
      public: virtual void AddRenderPass(const RenderPassPtr &_pass) override;

      // Documentation inherited.
      public: virtual void RemoveRenderPass(const RenderPassPtr &_pass)
          override;

      // Documentation inherited.
      public: virtual unsigned int RenderPassCount() const override;

      // Documentation inherited.
      public: virtual RenderPassPtr RenderPassByIndex(unsigned int _index)
          const override;

      protected: virtual void *CreateImageBuffer() const;

      protected: virtual void Load() override;

      protected: virtual void Reset();

      protected: virtual RenderTargetPtr RenderTarget() const = 0;

      IGN_COMMON_WARN_IGNORE__DLL_INTERFACE_MISSING
      protected: common::EventT<void(const void *, unsigned int, unsigned int,
                     unsigned int, const std::string &)> newFrameEvent;

      protected: ImagePtr imageBuffer;

      /// \brief Near clipping plane distance
      protected: double nearClip = 0.01;

      /// \brief Far clipping plane distance
      protected: double farClip = 1000.0;

      /// \brief Aspect ratio
      protected: double aspect = 1.3333333;

      /// \brief Horizontal camera field of view
      protected: math::Angle hfov;

      /// \brief Anti-aliasing
      protected: unsigned int antiAliasing = 0u;

      /// \brief Target node to track if camera tracking is on.
      protected: NodePtr trackNode;

      /// \brief Track point relative to target in world frame.
      protected: bool trackWorldFrame = false;

      /// \brief Set camera to track a point offset in target node's local or
      /// world frame depending on trackWorldFrame.
      protected: math::Vector3d trackOffset;

      /// \brief P gain for tracking. Determines how fast the camera rotates
      /// to look at the target node. Valid range: [0-1]
      protected: double trackPGain = 1.0;

      /// \brief Target node to follow
      protected: NodePtr followNode;
      IGN_COMMON_WARN_RESUME__DLL_INTERFACE_MISSING

      /// \brief Follow target in world frame.
      protected: bool followWorldFrame = false;

      /// \brief P gain for follow mode. Determines how fast the camera moves
      /// to follow the target node. Valid range: [0-1]
      protected: double followPGain = 1.0;

      /// \brief Offset distance between camera and target node being followed
      protected: math::Vector3d followOffset;

      friend class BaseDepthCamera<T>;
    };

    //////////////////////////////////////////////////
    template <class T>
    BaseCamera<T>::BaseCamera()
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    BaseCamera<T>::~BaseCamera()
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    unsigned int BaseCamera<T>::ImageWidth() const
    {
      return this->RenderTarget()->Width();
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseCamera<T>::SetImageWidth(const unsigned int _width)
    {
      this->RenderTarget()->SetWidth(_width);
    }

    //////////////////////////////////////////////////
    template <class T>
    unsigned int BaseCamera<T>::ImageHeight() const
    {
      return this->RenderTarget()->Height();
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseCamera<T>::SetImageHeight(const unsigned int _height)
    {
      this->RenderTarget()->SetHeight(_height);
    }

    //////////////////////////////////////////////////
    template <class T>
    unsigned int BaseCamera<T>::ImageMemorySize() const
    {
      PixelFormat format = this->ImageFormat();
      unsigned int width = this->ImageWidth();
      unsigned int height = this->ImageHeight();
      return PixelUtil::MemorySize(format, width, height);
    }

    //////////////////////////////////////////////////
    template <class T>
    PixelFormat BaseCamera<T>::ImageFormat() const
    {
      return this->RenderTarget()->Format();
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseCamera<T>::SetImageFormat(PixelFormat _format)
    {
      this->RenderTarget()->SetFormat(_format);
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseCamera<T>::PreRender()
    {
      T::PreRender();

      this->RenderTarget()->PreRender();

      // camera following
      if (this->followNode)
      {
        // tether camera fixed in world frame
        if (this->followWorldFrame)
        {
          math::Vector3d targetCamPos =
              this->followNode->WorldPosition() + this->followOffset;
          math::Vector3d pos = this->WorldPosition() +
              (targetCamPos - this->WorldPosition()) * this->followPGain;
          this->SetWorldPosition(pos);
        }
        // tether camera fixed in target's local frame
        else
        {
          math::Pose3d targetCamPose = math::Pose3d(this->followOffset,
              this->WorldRotation());
          targetCamPose += this->followNode->WorldPose();

          math::Vector3d pos = this->WorldPosition() +
              (targetCamPose.Pos() - this->WorldPosition()) * this->followPGain;
          this->SetWorldPosition(pos);
        }
      }

      // camera tracking
      if (this->trackNode)
      {
        math::Vector3d eye = this->WorldPosition();
        math::Pose3d targetPose = math::Pose3d(this->trackOffset,
            math::Quaterniond::Identity);
        if (this->trackWorldFrame)
        {
          targetPose.Pos() += this->trackNode->WorldPosition();
        }
        else
        {
          targetPose += this->trackNode->WorldPose();
        }

        math::Pose3d p =
            math::Matrix4d::LookAt(eye, targetPose.Pos()).Pose();

        math::Quaterniond q = p.Rot();
        // skip slerp if we don't need it
        if (!math::equal(this->trackPGain, 1.0))
        {
          q = math::Quaterniond::Slerp(
              this->trackPGain, this->WorldRotation(), p.Rot(), true);
        }
        this->SetWorldRotation(q);
      }
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseCamera<T>::PostRender()
    {
      this->RenderTarget()->PostRender();
    }

    //////////////////////////////////////////////////
    template <class T>
    Image BaseCamera<T>::CreateImage() const
    {
      PixelFormat format = this->ImageFormat();
      unsigned int width = this->ImageWidth();
      unsigned int height = this->ImageHeight();
      return Image(width, height, format);
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseCamera<T>::Update()
    {
      this->Scene()->PreRender();
      this->Render();
      this->PostRender();
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseCamera<T>::Capture(Image &_image)
    {
      this->Update();
      this->Copy(_image);
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseCamera<T>::Copy(Image &_image) const
    {
      this->RenderTarget()->Copy(_image);
    }

    //////////////////////////////////////////////////
    template <class T>
    bool BaseCamera<T>::SaveFrame(const std::string &/*_name*/)
    {
      return false;
    }

    //////////////////////////////////////////////////
    template <class T>
    common::ConnectionPtr BaseCamera<T>::ConnectNewImageFrame(
        Camera::NewFrameListener _listener)
    {
      return newFrameEvent.Connect(_listener);
    }

    //////////////////////////////////////////////////
    template <class T>
    void *BaseCamera<T>::CreateImageBuffer() const
    {
      // TODO(anyone): determine proper type
      unsigned int size = this->ImageMemorySize();
      return new unsigned char *[size];
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseCamera<T>::Load()
    {
      T::Load();
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseCamera<T>::Reset()
    {
      math::Angle fov;
      fov.Degree(60);
      this->SetImageWidth(1);
      this->SetImageHeight(1);
      this->SetImageFormat(PF_R8G8B8);
      this->SetAspectRatio(1.33333);
      this->SetAntiAliasing(0u);
      this->SetHFOV(fov);
      this->SetNearClipPlane(0.01);
      this->SetFarClipPlane(1000);
    }

    //////////////////////////////////////////////////
    template <class T>
    RenderWindowPtr BaseCamera<T>::CreateRenderWindow()
    {
      // Does nothing by default
      ignerr << "Render window not supported for render engine: " <<
          this->Scene()->Engine()->Name() << std::endl;
      return RenderWindowPtr();
    }

    //////////////////////////////////////////////////
    template <class T>
    math::Matrix4d BaseCamera<T>::ProjectionMatrix() const
    {
      // perspective projection
      double ratio = this->AspectRatio();
      double fov = this->HFOV().Radian();
      double vfov =  2.0 * std::atan(std::tan(fov / 2.0) / ratio);
      double f = 1.0;
      double _near = this->NearClipPlane();
      double _far = this->FarClipPlane();
      double top = _near * std::tan(0.5*vfov) / f;
      double height = 2 * top;
      double width = ratio * height;
      double left = -0.5 * width;
      double right = left + width;
      double bottom = top - height;

      double invw = 1.0 / (right - left);
      double invh = 1.0 / (top - bottom);
      double invd = 1.0 / (_far - _near);
      double x = 2 * _near * invw;
      double y = 2 * _near * invh;
      double a = (right + left) * invw;
      double b = (top + bottom) * invh;
      double c = -(_far + _near) * invd;
      double d = -2 * _far * _near * invd;
      math::Matrix4d result;
      result(0, 0) = x;
      result(0, 2) = a;
      result(1, 1) = y;
      result(1, 2) = b;
      result(2, 2) = c;
      result(2, 3) = d;
      result(3, 2) = -1;

      // TODO(anyone): compute projection matrix for orthographic camera

      return result;
    }

    //////////////////////////////////////////////////
    template <class T>
    math::Matrix4d BaseCamera<T>::ViewMatrix() const
    {
      math::Matrix3d r(this->WorldPose().Rot());
      // transform from y up to z up
      math::Matrix3d tf(0, 0, -1,
                       -1, 0,  0,
                        0, 1,  0);
      r = r * tf;
      r.Transpose();
      math::Vector3d t = r  * this->WorldPose().Pos() * -1;
      math::Matrix4d result;
      result = r;
      result.SetTranslation(t);
      result(3, 3) = 1.0;
      return result;
    }

    //////////////////////////////////////////////////
    template <class T>
    math::Angle BaseCamera<T>::HFOV() const
    {
      return this->hfov;
    }

    //////////////////////////////////////////////////
    template <class T>
    VisualPtr BaseCamera<T>::VisualAt(const ignition::math::Vector2i
        &/*_mousePos*/)
    {
      ignerr << "VisualAt not implemented for the render engine" << std::endl;
      return VisualPtr();
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseCamera<T>::SetHFOV(const math::Angle &_hfov)
    {
      this->hfov = _hfov;
    }

    //////////////////////////////////////////////////
    template <class T>
    double BaseCamera<T>::AspectRatio() const
    {
      return this->aspect;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseCamera<T>::SetAspectRatio(const double _aspect)
    {
      this->aspect = _aspect;
    }

    //////////////////////////////////////////////////
    template <class T>
    unsigned int BaseCamera<T>::AntiAliasing() const
    {
      return this->antiAliasing;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseCamera<T>::SetAntiAliasing(const unsigned int _aa)
    {
      this->antiAliasing = _aa;
    }

    //////////////////////////////////////////////////
    template <class T>
    double BaseCamera<T>::FarClipPlane() const
    {
      return this->farClip;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseCamera<T>::SetFarClipPlane(const double _far)
    {
      this->farClip = _far;
    }

    //////////////////////////////////////////////////
    template <class T>
    double BaseCamera<T>::NearClipPlane() const
    {
      return this->nearClip;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseCamera<T>::SetNearClipPlane(const double _near)
    {
      this->nearClip = _near;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseCamera<T>::SetTrackTarget(const NodePtr &_target,
        const math::Vector3d &_offset, const bool _worldFrame)
    {
      this->trackNode = _target;
      this->trackWorldFrame = _worldFrame;
      this->trackOffset = _offset;
    }

    //////////////////////////////////////////////////
    template <class T>
    NodePtr BaseCamera<T>::TrackTarget() const
    {
      return this->trackNode;
    }

    //////////////////////////////////////////////////
    template <class T>
    math::Vector3d BaseCamera<T>::TrackOffset() const
    {
      return this->trackOffset;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseCamera<T>::SetTrackOffset(const math::Vector3d &_offset)
    {
      this->trackOffset = _offset;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseCamera<T>::SetTrackPGain(const double _pGain)
    {
      this->trackPGain = math::clamp(_pGain, 0.0, 1.0);
    }

    //////////////////////////////////////////////////
    template <class T>
    double BaseCamera<T>::TrackPGain() const
    {
      return this->trackPGain;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseCamera<T>::SetFollowTarget(const NodePtr &_target,
        const math::Vector3d &_offset, const bool _worldFrame)
    {
      this->followNode = _target;
      this->followWorldFrame = _worldFrame;
      this->followOffset = _offset;
    }

    //////////////////////////////////////////////////
    template <class T>
    NodePtr BaseCamera<T>::FollowTarget() const
    {
      return this->followNode;
    }

    //////////////////////////////////////////////////
    template <class T>
    math::Vector3d BaseCamera<T>::FollowOffset() const
    {
      return this->followOffset;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseCamera<T>::SetFollowOffset(const math::Vector3d &_offset)
    {
      this->followOffset = _offset;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseCamera<T>::SetFollowPGain(const double _pGain)
    {
      this->followPGain = math::clamp(_pGain, 0.0, 1.0);
    }

    //////////////////////////////////////////////////
    template <class T>
    double BaseCamera<T>::FollowPGain() const
    {
      return this->followPGain;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseCamera<T>::SetMaterial(const MaterialPtr &/*_material*/)
    {
      ignerr << "SetMaterial not implemented for current render"
          << " engine" << std::endl;
    }

    //////////////////////////////////////////////////
    template <class T>
    unsigned int BaseCamera<T>::RenderTextureGLId() const
    {
      ignerr << "RenderTextureGLId is not supported by current render"
          << " engine" << std::endl;
      return 0u;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseCamera<T>::AddRenderPass(const RenderPassPtr &_pass)
    {
      this->RenderTarget()->AddRenderPass(_pass);
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseCamera<T>::RemoveRenderPass(const RenderPassPtr &_pass)
    {
      this->RenderTarget()->RemoveRenderPass(_pass);
    }

    //////////////////////////////////////////////////
    template <class T>
    unsigned int BaseCamera<T>::RenderPassCount() const
    {
      return this->RenderTarget()->RenderPassCount();
    }

    //////////////////////////////////////////////////
    template <class T>
    RenderPassPtr BaseCamera<T>::RenderPassByIndex(unsigned int _index) const
    {
      return this->RenderTarget()->RenderPassByIndex(_index);
    }
    }
  }
}
#endif
