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
#ifndef IGNITION_RENDERING_BASE_BASEGRID_HH_
#define IGNITION_RENDERING_BASE_BASEGRID_HH_

#include <string>
#include "ignition/rendering/Grid.hh"
#include "ignition/rendering/base/BaseObject.hh"

namespace ignition
{
  namespace rendering
  {
    //////////////////////////////////////////////////
    template <class T>
    class IGNITION_VISIBLE BaseGrid :
      public virtual Mesh,
      public virtual T
    {
      protected: BaseGrid();

      public: virtual ~BaseGrid();

      public: virtual void PreRender();

      public: virtual void Destroy();

    };

    //////////////////////////////////////////////////
    // BaseGrid
    //////////////////////////////////////////////////
    template <class T>
    BaseGrid<T>::BaseGrid()
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    BaseGrid<T>::~BaseGrid()
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    unsigned int BaseGrid<T>::CellCount() const
    {
      return this->cellCount;
    }

    //////////////////////////////////////////////////
    template <class T>
    unsigned int BaseGrid<T>::CellLength() const
    {
      return this->cellLength;
    }

    //////////////////////////////////////////////////
    template <class T>
    unsigned int BaseGrid<T>::VerticalCellCount() const
    {
      return this->verticalCellCount;
    }


    //////////////////////////////////////////////////
    template <class T>
    void BaseGrid<T>::PreRender()
    {
      T::PreRender();
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseGrid<T>::Destroy()
    {
      T::Destroy();
    }

  }
}
#endif
