/*
 * Copyright (C) 2018 Open Source Robotics Foundation
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
#ifndef IGNITION_RENDERING_BASE_BASETEXT_HH_
#define IGNITION_RENDERING_BASE_BASETEXT_HH_

#include <string>
#include "ignition/rendering/Text.hh"
#include "ignition/rendering/base/BaseObject.hh"

namespace ignition
{
  namespace rendering
  {
    /// \brief Base implementation of a text geometry
    template <class T>
    class IGNITION_RENDERING_VISIBLE BaseText :
      public virtual Text,
      public virtual T
    {
      /// \brief Constructor
      protected: BaseText();

      /// \brief Destructor
      public: virtual ~BaseText();

      // Documentation inherited.
      public: virtual void PreRender();

      // Documentation inherited.
      public: virtual void Destroy();

      // Documentation inherited.
      public: void SetFontName(const std::string &_font);

      // Documentation inherited.
      public: std::string FontName() const;

      // Documentation inherited.
      public: void SetTextString(const std::string &_text);

      // Documentation inherited.
      public: std::string TextString() const;

      // Documentation inherited.
      public: void SetColor(const ignition::math::Color &_color);

      // Documentation inherited.
      public: ignition::math::Color Color() const;

      // Documentation inherited.
      public: void SetCharHeight(const float _height);

      // Documentation inherited.
      public: float CharHeight() const;

      // Documentation inherited.
      public: void SetSpaceWidth(const float _width);

      // Documentation inherited.
      public: float SpaceWidth() const;

      // Documentation inherited.
      public: void SetTextAlignment(const HorizontalAlign &_horzAlign,
                                    const VerticalAlign &_vertAlign);
      // Documentation inherited.
      public: void SetBaseline(const float _baseline);

      // Documentation inherited.
      public: float Baseline() const;

      // Documentation inherited.
      public: void SetShowOnTop(const bool _onTop);

      // Documentation inherited.
      public: bool ShowOnTop() const;

      // Documentation inherited.
      public: ignition::math::Box AABB();

      /// \brief Font name, such as "Arial"
      protected: std::string fontName = "Arial";

      /// \brief Text being displayed
      protected: std::string text;

      /// \brief Text color
      protected: ignition::math::Color color = ignition::math::Color::White;

      /// \brief Character height in meters
      protected: float charHeight= 1.0;

      /// \brief Width of space between letters
      protected: float spaceWidth = 0;

      /// \brief Horizontal alignment
      protected: HorizontalAlign horizontalAlign = Text::H_LEFT;

      /// \brief Vertical alignment
      protected: VerticalAlign verticalAlign = Text::V_BOTTOM;

      /// \brief Baseline height in meters.
      protected: float baseline = 0;

      /// \brief True for text to be displayed on top of other objects in the scene.
      protected: bool onTop = false;

      /// \brief Flag to indicate text properties have changed
      protected: bool textDirty = false;
    };

    //////////////////////////////////////////////////
    // BaseText
    //////////////////////////////////////////////////
    template <class T>
    BaseText<T>::BaseText()
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    BaseText<T>::~BaseText()
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    std::string BaseText<T>::FontName() const
    {
      return this->fontName;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseText<T>::SetFontName(const std::string &_font)
    {
      this->fontName = _font;
      this->textDirty = true;
    }

    //////////////////////////////////////////////////
    template <class T>
    std::string BaseText<T>::TextString() const
    {
      return this->text;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseText<T>::SetTextString(const std::string &_text)
    {
      this->text = _text;
      this->textDirty = true;
    }

    //////////////////////////////////////////////////
    template <class T>
    ignition::math::Color BaseText<T>::Color() const
    {
      return this->color;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseText<T>::SetColor(const ignition::math::Color &_color)
    {
      this->color = _color;
      this->textDirty = true;
    }

    //////////////////////////////////////////////////
    template <class T>
    float BaseText<T>::CharHeight() const
    {
      return this->charHeight;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseText<T>::SetCharHeight(const float _height)
    {
      this->charHeight = _height;
      this->textDirty = true;
    }

    //////////////////////////////////////////////////
    template <class T>
    float BaseText<T>::SpaceWidth() const
    {
      return this->spaceWidth;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseText<T>::SetSpaceWidth(const float _width)
    {
      this->spaceWidth = _width;
      this->textDirty = true;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseText<T>::SetTextAlignment(const HorizontalAlign &_horzAlign,
                                       const VerticalAlign &_vertAlign)
    {
      this->horizontalAlign = _horzAlign;
      this->verticalAlign = _vertAlign;
      this->textDirty = true;
    }

    //////////////////////////////////////////////////
    template <class T>
    float BaseText<T>::Baseline() const
    {
      return this->baseline;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseText<T>::SetBaseline(const float _baseline)
    {
      this->baseline = _baseline;
      this->textDirty = true;
    }

    //////////////////////////////////////////////////
    template <class T>
    bool BaseText<T>::ShowOnTop() const
    {
      return this->onTop;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseText<T>::SetShowOnTop(const bool _onTop)
    {
      this->onTop = _onTop;
      this->textDirty = true;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseText<T>::PreRender()
    {
      T::PreRender();
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseText<T>::Destroy()
    {
      T::Destroy();
    }
  }
}
#endif
