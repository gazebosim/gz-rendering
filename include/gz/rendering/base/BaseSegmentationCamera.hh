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
#ifndef GZ_RENDERING_BASE_BASESEGMENTATIONCAMERA_HH_
#define GZ_RENDERING_BASE_BASESEGMENTATIONCAMERA_HH_

#include <random>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>

#include <gz/common/Event.hh>

#include "gz/rendering/base/BaseCamera.hh"
#include "gz/rendering/Scene.hh"
#include "gz/rendering/SegmentationCamera.hh"
#include "gz/rendering/Visual.hh"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {

    /// \brief Base implementation of the segmentation camera class: the
    /// segmentation type, background and label maps, and the assignment of
    /// the label colors that engines share
    template <class T>
    class BaseSegmentationCamera :
      public virtual SegmentationCamera,
      public virtual BaseCamera<T>,
      public virtual T
    {
      /// \brief Constructor
      protected: BaseSegmentationCamera();

      /// \brief Destructor
      public: virtual ~BaseSegmentationCamera();

      // Documentation inherited
      public: virtual void CreateSegmentationTexture() override = 0;

      // Documentation inherited
      public: virtual uint8_t *SegmentationData() const override;

      // Documentation inherited
      public: virtual gz::common::ConnectionPtr
        ConnectNewSegmentationFrame(
          std::function<void(const uint8_t *, unsigned int, unsigned int,
          unsigned int, const std::string &)>  _subscriber) override;

      // Documentation inherited
      public: virtual void SetSegmentationType(
        SegmentationType _type) override;

      // Documentation inherited
      public: virtual SegmentationType Type() const override;

      // Documentation inherited
      public: virtual void EnableColoredMap(bool _enable) override;

      // Documentation inherited
      public: virtual bool IsColoredMap() const override;

      // Documentation inherited
      public: virtual void SetBackgroundColor(
        const math::Color &_color) override;

      // Documentation inherited
      public: virtual void SetBackgroundLabel(int _label) override;

      // Documentation inherited
      public: virtual const math::Color &BackgroundColor() const override;

      // Documentation inherited
      public: virtual int BackgroundLabel() const override;

      // Documentation inherited
      public: void LabelMapFromColoredBuffer(
                  uint8_t *_labelBuffer) const override = 0;

      /// \brief Forget the colors assigned by ColorForVisual. Call it before
      /// assigning the colors of a new frame.
      protected: void ClearLabelColors();

      /// \brief Get the color a visual is rendered with, according to its
      /// "label" user data, the segmentation type and whether the map is
      /// colored. Call it once per renderable, sorted so that all the
      /// renderables of a model are consecutive.
      /// \param[in] _visual Visual the color is applied to
      /// \param[in,out] _prevParentName Name of the top level visual of the
      /// previous call, so that all links of a model get the same color
      /// \return The color to apply to the visual
      protected: math::Color ColorForVisual(const VisualPtr &_visual,
                                            std::string &_prevParentName);

      /// \brief Convert a label to a unique color for the colored map and
      /// remember the color as taken
      /// \param[in] _label id of the semantic map or encoded id of panoptic map
      /// \param[in] _isMultiLink True to skip the taken color check: all the
      /// links of a model have the same color
      /// \return Unique color in the colored map for that label
      protected: math::Color LabelToColor(int64_t _label,
                                          bool _isMultiLink = false);

      /// \brief Convert a colored map to a label map, see
      /// SegmentationCamera::LabelMapFromColoredBuffer
      /// \param[in] _coloredBuffer Colored map of the last frame, RGB8
      /// \param[out] _labelBuffer Label map, RGB8
      protected: void LabelMapFromColoredBuffer(const uint8_t *_coloredBuffer,
                                                uint8_t *_labelBuffer) const;

      /// \brief The buffer that contains segmentation data
      protected: uint8_t *segmentationData {nullptr};

      /// \brief The segmentation type
      protected: SegmentationType type {SegmentationType::ST_SEMANTIC};

      /// \brief Whether a colored map is being generated (true) or label ID map
      /// is being generated (false)
      protected: bool isColoredMap {false};

      /// \brief The color of objects that are considered background (i.e.,
      /// objects that have no label)
      protected: math::Color backgroundColor {0, 0, 0};

      /// \brief The label of background objects
      protected: int backgroundLabel {0};

      /// \brief Number of instances of each label. Key: label id
      private: std::unordered_map<int, unsigned int> instancesCount;

      /// \brief Colors in use (encoded id of r,g,b)
      private: std::unordered_set<int64_t> takenColors;

      /// \brief Labels that already have a color, for semantic segmentation
      private: std::unordered_set<int64_t> coloredLabel;

      /// \brief Mapping from the color id to the label in semantic
      /// segmentation, or to the composite id (8 bit label + 16 bit instance
      /// count) in panoptic segmentation
      private: std::unordered_map<int64_t, int64_t> colorToLabel;

      /// \brief Pseudo random generator of the color of a label
      private: std::default_random_engine generator;
    };

    //////////////////////////////////////////////////
    template <class T>
    BaseSegmentationCamera<T>::BaseSegmentationCamera()
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    BaseSegmentationCamera<T>::~BaseSegmentationCamera()
    {
      if (this->segmentationData)
      {
        delete [] this->segmentationData;
        this->segmentationData = nullptr;
      }
    }

    //////////////////////////////////////////////////
    template <class T>
    uint8_t *BaseSegmentationCamera<T>::SegmentationData() const
    {
      return this->segmentationData;
    }

    //////////////////////////////////////////////////
    template <class T>
    gz::common::ConnectionPtr BaseSegmentationCamera<T>::
      ConnectNewSegmentationFrame(
          std::function<void(const uint8_t *, unsigned int, unsigned int,
          unsigned int, const std::string &)>)
    {
      return nullptr;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseSegmentationCamera<T>::SetSegmentationType(SegmentationType _type)
    {
      this->type = _type;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseSegmentationCamera<T>::EnableColoredMap(bool _enable)
    {
      this->isColoredMap = _enable;
    }

    //////////////////////////////////////////////////
    template <class T>
    SegmentationType BaseSegmentationCamera<T>::Type() const
    {
      return this->type;
    }

    //////////////////////////////////////////////////
    template <class T>
    bool BaseSegmentationCamera<T>::IsColoredMap() const
    {
      return this->isColoredMap;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseSegmentationCamera<T>::SetBackgroundColor(
        const math::Color &_color)
    {
      this->backgroundColor = _color;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseSegmentationCamera<T>::SetBackgroundLabel(int _label)
    {
      this->backgroundLabel = _label;
    }

    //////////////////////////////////////////////////
    template <class T>
    const math::Color &BaseSegmentationCamera<T>::BackgroundColor() const
    {
      return this->backgroundColor;
    }

    //////////////////////////////////////////////////
    template <class T>
    int BaseSegmentationCamera<T>::BackgroundLabel() const
    {
      return this->backgroundLabel;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseSegmentationCamera<T>::ClearLabelColors()
    {
      this->instancesCount.clear();
      this->takenColors.clear();
      this->coloredLabel.clear();
      this->colorToLabel.clear();
    }

    //////////////////////////////////////////////////
    template <class T>
    math::Color BaseSegmentationCamera<T>::ColorForVisual(
        const VisualPtr &_visual, std::string &_prevParentName)
    {
      // items with no label are considered background
      int label = this->BackgroundLabel();
      Variant labelAny = _visual->UserData("label");
      if (const int *labelPtr = std::get_if<int>(&labelAny))
        label = *labelPtr;

      if (this->Type() == SegmentationType::ST_SEMANTIC)
      {
        // each pixel has the item's color or the item's label
        if (this->IsColoredMap())
          return this->LabelToColor(label);
        return math::Color(label / 255.0f, label / 255.0f, label / 255.0f);
      }

      // panoptic: instances are counted per top level model
      VisualPtr top = _visual;
      while (top->Parent() && top->Parent() != _visual->Scene()->RootVisual())
        top = std::dynamic_pointer_cast<Visual>(top->Parent());
      const std::string parentName = top->Name();

      // A multi link model has many links with the same parent, which
      // should all have the same color
      unsigned int &count = this->instancesCount[label];
      bool isMultiLink = false;
      if (parentName == _prevParentName)
      {
        isMultiLink = true;
      }
      else
      {
        ++count;
        _prevParentName = parentName;
      }
      const int instanceCount = count;

      if (this->IsColoredMap())
      {
        if (label == this->BackgroundLabel())
          return this->LabelToColor(label, isMultiLink);
        // composite id: 8 bit label and 16 bit instance count
        return this->LabelToColor(
            static_cast<int64_t>(label) * 256 * 256 + instanceCount,
            isMultiLink);
      }
      return math::Color((instanceCount % 256) / 255.0f,
          (instanceCount / 256) / 255.0f, label / 255.0f);
    }

    //////////////////////////////////////////////////
    template <class T>
    math::Color BaseSegmentationCamera<T>::LabelToColor(int64_t _label,
        bool _isMultiLink)
    {
      if (_label == this->BackgroundLabel())
        return this->BackgroundColor();

      // use the label as seed to generate the same color for the label
      this->generator.seed(_label);
      std::uniform_int_distribution<int> distribution(0, 255);
      const int r = distribution(this->generator);
      const int g = distribution(this->generator);
      const int b = distribution(this->generator);
      const math::Color color(r / 255.0f, g / 255.0f, b / 255.0f);

      // all items with the same label have the same color, and so do all
      // links of a model: no need to check for taken colors
      if (this->Type() == SegmentationType::ST_SEMANTIC &&
          this->coloredLabel.count(_label))
        return color;
      if (_isMultiLink)
        return color;

      // loop recursively till finding a unique color
      const int64_t colorId = r * 256 * 256 + g * 256 + b;
      if (!this->takenColors.insert(colorId).second)
        return this->LabelToColor(_label);

      this->coloredLabel.insert(_label);
      this->colorToLabel[colorId] = _label;
      return color;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseSegmentationCamera<T>::LabelMapFromColoredBuffer(
        const uint8_t *_coloredBuffer, uint8_t *_labelBuffer) const
    {
      if (!this->isColoredMap || !_coloredBuffer)
        return;

      const size_t pixels =
          static_cast<size_t>(this->ImageWidth()) * this->ImageHeight();
      for (size_t index = 0; index < pixels * 3; index += 3)
      {
        const int64_t colorId = _coloredBuffer[index] * 256 * 256 +
            _coloredBuffer[index + 1] * 256 + _coloredBuffer[index + 2];

        // initialize the pixel with the background label value
        _labelBuffer[index] = this->backgroundLabel;
        _labelBuffer[index + 1] = this->backgroundLabel;
        _labelBuffer[index + 2] = this->backgroundLabel;

        auto it = this->colorToLabel.find(colorId);
        if (it == this->colorToLabel.end())
          continue;
        const int64_t label = it->second;

        if (this->type == SegmentationType::ST_SEMANTIC)
        {
          const uint8_t label8bit = label % 256;
          _labelBuffer[index] = label8bit;
          _labelBuffer[index + 1] = label8bit;
          _labelBuffer[index + 2] = label8bit;
        }
        else if (this->type == SegmentationType::ST_PANOPTIC)
        {
          // composite id: 8 bit label, then the 16 bit instance count
          // split in two 8 bit channels
          const uint16_t instanceCount = label % (256 * 256);
          _labelBuffer[index + 2] = label / (256 * 256);
          _labelBuffer[index + 1] = instanceCount / 256;
          _labelBuffer[index] = instanceCount % 256;
        }
      }
    }
  }
  }
}
#endif
