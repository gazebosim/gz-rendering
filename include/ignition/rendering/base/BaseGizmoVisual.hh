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

#ifndef IGNITION_RENDERING_BASE_BASEGIZMOVISUAL_HH_
#define IGNITION_RENDERING_BASE_BASEGIZMOVISUAL_HH_

//#include "ignition/rendering/base/BaseObject.hh"
#include "ignition/rendering/base/BaseScene.hh"
#include "ignition/rendering/base/BaseNode.hh"
#include "ignition/rendering/ArrowVisual.hh"
#include "ignition/rendering/GizmoVisual.hh"

namespace ignition
{
  namespace rendering
  {
    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
    //
    /* \class BaseGizmoVisual BaseGizmoVisuall.hh \
     * ignition/rendering/base/BaseGizmoVisual.hh
     */
    /// \brief A base implementation of the GizmoVisual class
    template <class T>
    class BaseGizmoVisual :
      public virtual GizmoVisual,
      public virtual T
    {
      /// \brief Constructor
      protected: BaseGizmoVisual();

      /// \brief Destructor
      public: virtual ~BaseGizmoVisual();

      // Documentation inherited
      public: virtual void Init() override;

      // Documentation inherited
      public: virtual void PreRender() override;

      // Documentation inherited
      public: virtual void SetMode(TransformMode _mode) override;

      // Documentation inherited
      public: virtual TransformMode Mode() const override;

      // Documentation inherited
      public: virtual void SetActiveAxis(const math::Vector3d &_axis) override;

      // Documentation inherited
      public: virtual math::Vector3d ActiveAxis() const override;

      // Documentation inherited
      public: virtual TransformAxis AxisById(unsigned int _id) const;

      /// \brief Reset the gizmo visual state
      public: virtual void Reset();

      /// \brief Create materials used by the gizmo visual
      protected: void CreateMaterials();

      /// \brief Create gizmo visual for translation
      protected: void CreateTranslationVisual();

      /// \brief Create gizmo visual for rotation
      protected: void CreateRotationVisual();

      /// \brief Create gizmo visual for scale
      /// \return Rotation visual
      protected: void CreateScaleVisual();

      /// \brief Current gizmo mode
      protected: TransformMode mode = TransformMode::TM_NONE;

      /// \brief A map of gizmo modes and their visuals
      protected: std::map<TransformAxis, VisualPtr> visuals;

      /// \brief Currently active visual.
      protected: VisualPtr activeVis;

      /// \brief Flag to indicate the mode has changed.
      protected: bool modeDirty = false;

      /// \brief Active axis
      protected: math::Vector3d axis = math::Vector3d::Zero;

      /// \brief A map of axis enums to materials
      protected: std::map<unsigned int, MaterialPtr> materials;

      /// \brief Material used by axes
      protected: enum AxisMaterial
                 {
                   /// \brief X axis
                   AM_X,
                   /// \brief Y axis
                   AM_Y,
                   /// \brief Z axis
                   AM_Z,
                   /// \brief Active axis
                   AM_ACTIVE
                 };

      /// \brief Only the scene can create a GizmoVisual
      private: friend class BaseScene;
    };

    //////////////////////////////////////////////////
    template <class T>
    BaseGizmoVisual<T>::BaseGizmoVisual()
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    BaseGizmoVisual<T>::~BaseGizmoVisual()
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseGizmoVisual<T>::Init()
    {
      T::Init();

      this->CreateMaterials();
      this->CreateTranslationVisual();
      this->CreateRotationVisual();
      this->CreateScaleVisual();

      for (auto v : this->visuals)
        v.second->SetVisible(false);
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseGizmoVisual<T>::Reset()
    {
      this->visuals[TransformAxis::TA_TRANSLATION_X]->SetMaterial(
          this->materials[AM_X], false);
      this->visuals[TransformAxis::TA_TRANSLATION_Y]->SetMaterial(
          this->materials[AM_Y], false);
      this->visuals[TransformAxis::TA_TRANSLATION_Z]->SetMaterial(
          this->materials[AM_Z], false);
      this->visuals[TransformAxis::TA_ROTATION_X]->SetMaterial(
          this->materials[AM_X], false);
      this->visuals[TransformAxis::TA_ROTATION_Y]->SetMaterial(
          this->materials[AM_Y], false);
      this->visuals[TransformAxis::TA_ROTATION_Z]->SetMaterial(
          this->materials[AM_Z], false);
      this->visuals[TransformAxis::TA_SCALE_X]->SetMaterial(
          this->materials[AM_X], false);
      this->visuals[TransformAxis::TA_SCALE_Y]->SetMaterial(
          this->materials[AM_Y], false);
      this->visuals[TransformAxis::TA_SCALE_Z]->SetMaterial(
          this->materials[AM_Z], false);

      for (auto v : this->visuals)
        v.second->SetVisible(false);
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseGizmoVisual<T>::PreRender()
    {
      if (!this->modeDirty)
        return;

      this->Reset();

      if (this->mode == TransformMode::TM_NONE)
      {
        this->modeDirty = false;
        return;
      }

      if (this->mode & TransformMode::TM_TRANSLATION)
      {
        this->visuals[TransformAxis::TA_TRANSLATION_X]->SetVisible(true);
        this->visuals[TransformAxis::TA_TRANSLATION_Y]->SetVisible(true);
        this->visuals[TransformAxis::TA_TRANSLATION_Z]->SetVisible(true);
        if (this->axis.X() > 0)
        {
          this->visuals[TransformAxis::TA_TRANSLATION_X]->SetMaterial(
              this->materials[AM_ACTIVE]);
        }
        if (this->axis.Y() > 0)
        {
          this->visuals[TransformAxis::TA_TRANSLATION_Y]->SetMaterial(
              this->materials[AM_ACTIVE]);
        }
        if (this->axis.Z() > 0)
        {
          this->visuals[TransformAxis::TA_TRANSLATION_Z]->SetMaterial(
              this->materials[AM_ACTIVE]);
        }
      }
      else if (this->mode & TransformMode::TM_ROTATION)
      {
        this->visuals[TransformAxis::TA_ROTATION_X]->SetVisible(true);
        this->visuals[TransformAxis::TA_ROTATION_Y]->SetVisible(true);
        this->visuals[TransformAxis::TA_ROTATION_Z]->SetVisible(true);
        if (this->axis.X() > 0)
        {
          this->visuals[TransformAxis::TA_ROTATION_X]->SetMaterial(
              this->materials[AM_ACTIVE]);
        }
        if (this->axis.Y() > 0)
        {
          this->visuals[TransformAxis::TA_ROTATION_Y]->SetMaterial(
              this->materials[AM_ACTIVE]);
        }
        if (this->axis.Z() > 0)
        {
          this->visuals[TransformAxis::TA_ROTATION_Z]->SetMaterial(
              this->materials[AM_ACTIVE]);
        }

      }
      else if (this->mode & TransformMode::TM_SCALE)
      {
        this->visuals[TransformAxis::TA_SCALE_X]->SetVisible(true);
        this->visuals[TransformAxis::TA_SCALE_Y]->SetVisible(true);
        this->visuals[TransformAxis::TA_SCALE_Z]->SetVisible(true);
        if (this->axis.X() > 0)
        {
          this->visuals[TransformAxis::TA_SCALE_X]->SetMaterial(
              this->materials[AM_ACTIVE]);
        }
        if (this->axis.Y() > 0)
        {
          this->visuals[TransformAxis::TA_SCALE_Y]->SetMaterial(
              this->materials[AM_ACTIVE]);
        }
        if (this->axis.Z() > 0)
        {
          this->visuals[TransformAxis::TA_SCALE_Z]->SetMaterial(
              this->materials[AM_ACTIVE]);
        }
      }

      this->modeDirty = false;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseGizmoVisual<T>::SetMode(TransformMode _mode)
    {
      if (this->mode == _mode)
        return;

      this->mode = _mode;
      // clear active axis when mode changes
      this->axis = math::Vector3d::Zero;
      this->modeDirty = true;
    }

    //////////////////////////////////////////////////
    template <class T>
    TransformMode BaseGizmoVisual<T>::Mode() const
    {
      return this->mode;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseGizmoVisual<T>::SetActiveAxis(const math::Vector3d &_axis)
    {
      if (this->axis == _axis)
        return;

      this->axis = _axis;
      this->modeDirty = true;
    }

    //////////////////////////////////////////////////
    template <class T>
    math::Vector3d BaseGizmoVisual<T>::ActiveAxis() const
    {
      return this->axis;
    }

    //////////////////////////////////////////////////
    template <class T>
    TransformAxis BaseGizmoVisual<T>::AxisById(unsigned int _id) const
    {
      for (auto v : this->visuals)
      {
        if (v.second->Id() == _id || v.second->ChildById(_id))
          return v.first;
      }
      return TransformAxis::TA_NONE;
    }


    //////////////////////////////////////////////////
    template <class T>
    void BaseGizmoVisual<T>::CreateMaterials()
    {
      MaterialPtr xMat = this->Scene()->Material("Default/TransRed")->Clone();
      MaterialPtr yMat = this->Scene()->Material("Default/TransGreen")->Clone();
      MaterialPtr zMat = this->Scene()->Material("Default/TransBlue")->Clone();
      MaterialPtr activeMat =
          this->Scene()->Material("Default/TransYellow")->Clone();

      this->materials[AM_X] = xMat;
      this->materials[AM_Y] = yMat;
      this->materials[AM_Z] = zMat;
      this->materials[AM_ACTIVE] = activeMat;
    }
    //////////////////////////////////////////////////
    template <class T>
    void BaseGizmoVisual<T>::CreateTranslationVisual()
    {
      VisualPtr transVis = this->Scene()->CreateVisual();

      // trans x
      VisualPtr transXVis = this->Scene()->CreateVisual();
      VisualPtr transShaftXVis = this->Scene()->CreateVisual();
      transShaftXVis->AddGeometry(this->Scene()->CreateCylinder());
      transShaftXVis->SetOrigin(0, 0, 0.5);
      transShaftXVis->SetLocalPosition(0, 0, 0.5);
      transShaftXVis->SetLocalScale(0.025, 0.025, 0.5);
      transXVis->AddChild(transShaftXVis);

      VisualPtr transHeadXVis = this->Scene()->CreateVisual();
      transHeadXVis->AddGeometry(this->Scene()->CreateCone());
      transHeadXVis->SetOrigin(0, 0, -0.5);
      transHeadXVis->SetLocalPosition(0, 0, 0.5);
      transHeadXVis->SetLocalScale(0.07, 0.07, 0.2);
      transXVis->AddChild(transHeadXVis);

      transXVis->SetMaterial("Default/TransRed");
      transXVis->SetLocalRotation(0, IGN_PI * 0.5, 0);
      transVis->AddChild(transXVis);

      // trans y
      VisualPtr transYVis = this->Scene()->CreateVisual();
      VisualPtr transShaftYVis = this->Scene()->CreateVisual();
      transShaftYVis->AddGeometry(this->Scene()->CreateCylinder());
      transShaftYVis->SetOrigin(0, 0, 0.5);
      transShaftYVis->SetLocalPosition(0, 0, 0.5);
      transShaftYVis->SetLocalScale(0.025, 0.025, 0.5);
      transYVis->AddChild(transShaftYVis);

      VisualPtr transHeadYVis = this->Scene()->CreateVisual();
      transHeadYVis->AddGeometry(this->Scene()->CreateCone());
      transHeadYVis->SetOrigin(0, 0, -0.5);
      transHeadYVis->SetLocalPosition(0, 0, 0.5);
      transHeadYVis->SetLocalScale(0.07, 0.07, 0.2);
      transYVis->AddChild(transHeadYVis);

      transYVis->SetMaterial("Default/TransGreen");
      transYVis->SetLocalRotation(-IGN_PI * 0.5, 0, 0);
      transVis->AddChild(transYVis);

      // trans z
      VisualPtr transZVis = this->Scene()->CreateVisual();
      VisualPtr transShaftZVis = this->Scene()->CreateVisual();
      transShaftZVis->AddGeometry(this->Scene()->CreateCylinder());
      transShaftZVis->SetOrigin(0, 0, 0.5);
      transShaftZVis->SetLocalPosition(0, 0, 0.5);
      transShaftZVis->SetLocalScale(0.025, 0.025, 0.5);
      transZVis->AddChild(transShaftZVis);

      VisualPtr transHeadZVis = this->Scene()->CreateVisual();
      transHeadZVis->AddGeometry(this->Scene()->CreateCone());
      transHeadZVis->SetOrigin(0, 0, -0.5);
      transHeadZVis->SetLocalPosition(0, 0, 0.5);
      transHeadZVis->SetLocalScale(0.07, 0.07, 0.2);
      transZVis->AddChild(transHeadZVis);

      transZVis->SetMaterial("Default/TransBlue");
      transVis->AddChild(transZVis);

      this->visuals[TransformAxis::TA_TRANSLATION_X] = transXVis;
      this->visuals[TransformAxis::TA_TRANSLATION_Y] = transYVis;
      this->visuals[TransformAxis::TA_TRANSLATION_Z] = transZVis;

      this->AddChild(transVis);
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseGizmoVisual<T>::CreateRotationVisual()
    {
      // rotation x
      VisualPtr rotVis = this->Scene()->CreateVisual();
      VisualPtr rotXVis = this->Scene()->CreateVisual();
      rotXVis->AddGeometry(this->Scene()->CreateMesh("selection_tube"));
      rotXVis->SetLocalRotation(0, IGN_PI * 0.5, 0);
      rotXVis->SetLocalScale(0.5, 0.5, 0.5);
      rotXVis->SetMaterial("Default/TransRed");
      rotVis->AddChild(rotXVis);

      // rotation y
      VisualPtr rotYVis = this->Scene()->CreateVisual();
      rotYVis->AddGeometry(this->Scene()->CreateMesh("selection_tube"));
      rotYVis->SetLocalRotation(IGN_PI * 0.5, 0, 0);
      rotYVis->SetLocalScale(0.5, 0.5, 0.5);
      rotYVis->SetMaterial("Default/TransGreen");
      rotVis->AddChild(rotYVis);

      // rotation z
      VisualPtr rotZVis = this->Scene()->CreateVisual();
      rotZVis->AddGeometry(this->Scene()->CreateMesh("selection_tube"));
      rotZVis->SetLocalScale(0.5, 0.5, 0.5);
      rotZVis->SetMaterial("Default/TransBlue");
      rotVis->AddChild(rotZVis);

      this->visuals[TransformAxis::TA_ROTATION_X] = rotXVis;
      this->visuals[TransformAxis::TA_ROTATION_Y] = rotYVis;
      this->visuals[TransformAxis::TA_ROTATION_Z] = rotZVis;

      this->AddChild(rotVis);
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseGizmoVisual<T>::CreateScaleVisual()
    {
      VisualPtr scaleVis = this->Scene()->CreateVisual();

      // scale x
      VisualPtr scaleXVis = this->Scene()->CreateVisual();
      VisualPtr scaleShaftXVis = this->Scene()->CreateVisual();
      scaleShaftXVis->AddGeometry(this->Scene()->CreateCylinder());
      scaleShaftXVis->SetOrigin(0, 0, 0.5);
      scaleShaftXVis->SetLocalPosition(0, 0, 0.5);
      scaleShaftXVis->SetLocalScale(0.025, 0.025, 0.5);
      scaleXVis->AddChild(scaleShaftXVis);

      VisualPtr scaleHeadXVis = this->Scene()->CreateVisual();
      scaleHeadXVis->AddGeometry(this->Scene()->CreateBox());
      scaleHeadXVis->SetOrigin(0, 0, -0.5);
      scaleHeadXVis->SetLocalPosition(0, 0, 0.5);
      scaleHeadXVis->SetLocalScale(0.07, 0.07, 0.07);
      scaleXVis->AddChild(scaleHeadXVis);

      scaleXVis->SetMaterial("Default/TransRed");
      scaleXVis->SetLocalRotation(0, IGN_PI * 0.5, 0);
      scaleVis->AddChild(scaleXVis);

      // scale y
      VisualPtr scaleYVis = this->Scene()->CreateVisual();
      VisualPtr scaleShaftYVis = this->Scene()->CreateVisual();
      scaleShaftYVis->AddGeometry(this->Scene()->CreateCylinder());
      scaleShaftYVis->SetOrigin(0, 0, 0.5);
      scaleShaftYVis->SetLocalPosition(0, 0, 0.5);
      scaleShaftYVis->SetLocalScale(0.025, 0.025, 0.5);
      scaleYVis->AddChild(scaleShaftYVis);

      VisualPtr scaleHeadYVis = this->Scene()->CreateVisual();
      scaleHeadYVis->AddGeometry(this->Scene()->CreateBox());
      scaleHeadYVis->SetOrigin(0, 0, -0.5);
      scaleHeadYVis->SetLocalPosition(0, 0, 0.5);
      scaleHeadYVis->SetLocalScale(0.07, 0.07, 0.07);
      scaleYVis->AddChild(scaleHeadYVis);

      scaleYVis->SetMaterial("Default/TransGreen");
      scaleYVis->SetLocalRotation(-IGN_PI * 0.5, 0, 0);
      scaleVis->AddChild(scaleYVis);

      // scale z
      VisualPtr scaleZVis = this->Scene()->CreateVisual();
      VisualPtr scaleShaftZVis = this->Scene()->CreateVisual();
      scaleShaftZVis->AddGeometry(this->Scene()->CreateCylinder());
      scaleShaftZVis->SetOrigin(0, 0, 0.5);
      scaleShaftZVis->SetLocalPosition(0, 0, 0.5);
      scaleShaftZVis->SetLocalScale(0.025, 0.025, 0.5);
      scaleZVis->AddChild(scaleShaftZVis);

      VisualPtr scaleHeadZVis = this->Scene()->CreateVisual();
      scaleHeadZVis->AddGeometry(this->Scene()->CreateBox());
      scaleHeadZVis->SetOrigin(0, 0, -0.5);
      scaleHeadZVis->SetLocalPosition(0, 0, 0.5);
      scaleHeadZVis->SetLocalScale(0.07, 0.07, 0.07);
      scaleZVis->AddChild(scaleHeadZVis);

      scaleZVis->SetMaterial("Default/TransBlue");
      scaleVis->AddChild(scaleZVis);

      this->visuals[TransformAxis::TA_SCALE_X] = scaleXVis;
      this->visuals[TransformAxis::TA_SCALE_Y] = scaleYVis;
      this->visuals[TransformAxis::TA_SCALE_Z] = scaleZVis;

      this->AddChild(scaleVis);
    }
    }
  }
}
#endif
