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

#ifndef IGNITION_RENDERING_BASECAPSULE_HH_
#define IGNITION_RENDERING_BASECAPSULE_HH_

#include <vector>

#include <ignition/math/Vector3.hh>
#include <ignition/math/Vector2.hh>

#include "ignition/rendering/Capsule.hh"
#include "ignition/rendering/base/BaseObject.hh"

namespace ignition
{
  namespace rendering
  {
    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {

    /// \brief Base implementation of a Capsule Visual
    template <class T>
    class BaseCapsule :
      public virtual Capsule,
      public virtual T
    {
      // Documentation inherited
      protected: BaseCapsule();

      // Documentation inherited
      public: virtual ~BaseCapsule();

      // Documentation inherited
      public: virtual void PreRender() override;

      // Documentation inherited.
      public: virtual void Destroy() override;

      // Documentation inherited
      public: virtual void SetRadius(const double _radius) override;

      // Documentation inherited
      public: virtual void SetLength(const  double _length) override;

      // Documentation inherited
      public: virtual double Radius() override;

      // Documentation inherited
      public: virtual double Length() override;

      /// \brief return the list of triangles, _indexes and _uv coordinates
      /// \param[inout] _positions list with the triangles
      /// \param[inout] _indexes indexes of the position list
      /// \param[inout] _uv UV coordinates
      protected: void CapsuleMesh(
        std::vector<ignition::math::Vector3d> &_positions,
        std::vector<int> &_indexes,
        std::vector<ignition::math::Vector2d> &_uv);

      /// \brief Radius of the capsule
      protected: double radius = 0.5;

      /// \brief Length of the capsule
      protected: double length = 0.5;

      /// \brief Flag to indicate grid properties have changed
      protected: bool capsuleDirty = false;
    };

    /////////////////////////////////////////////////
    // BaseCapsule
    /////////////////////////////////////////////////
    template <class T>
    BaseCapsule<T>::BaseCapsule()
    {
    }

    /////////////////////////////////////////////////
    template <class T>
    BaseCapsule<T>::~BaseCapsule()
    {
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseCapsule<T>::PreRender()
    {
      T::PreRender();
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseCapsule<T>::Destroy()
    {
      T::Destroy();
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseCapsule<T>::SetRadius(const double _radius)
    {
      this->radius = _radius;
      this->capsuleDirty = true;
    }

    /////////////////////////////////////////////////
    template <class T>
    double BaseCapsule<T>::Radius()
    {
      return this->radius;
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseCapsule<T>::SetLength(const double _length)
    {
      this->length = _length;
      this->capsuleDirty = true;
    }

    /////////////////////////////////////////////////
    template <class T>
    double BaseCapsule<T>::Length()
    {
      return this->length;
    }
    /////////////////////////////////////////////////
    template <class T>
    void BaseCapsule<T>::CapsuleMesh(
      std::vector<ignition::math::Vector3d> &_positions,
      std::vector<int> &_indexes,
      std::vector<ignition::math::Vector2d> &_uv)
    {
      // Based on https://github.com/godotengine/godot primitive_meshes.cpp
      int prevRow, thisRow, point;
    	float x, y, z, u, v, w;
      float oneThird = 1.0 / 3.0;
    	float twoThirds = 2.0 / 3.0;
      unsigned int rings = 12;
      unsigned int radialSegments = 32;

    	point = 0;

      /* top hemisphere */
    	thisRow = 0;
    	prevRow = 0;
    	for (unsigned int j = 0; j <= (rings + 1); j++) {
    		v = j;

    		v /= (rings + 1);
    		w = sin(0.5 * M_PI * v);
    		y = this->radius * cos(0.5 * M_PI * v);

    		for (unsigned int i = 0; i <= radialSegments; i++) {
    			u = i;
    			u /= radialSegments;

    			x = -sin(u * (M_PI * 2.0));
    			z = cos(u * (M_PI * 2.0));

    			ignition::math::Vector3d p(
            x * this->radius * w, y, -z * this->radius * w);
    			_positions.push_back(
            p + ignition::math::Vector3d(0.0, 0.5 * this->length, 0.0));
          _uv.emplace_back(ignition::math::Vector2d(u, v * oneThird));
    			point++;

    			if (i > 0 && j > 0) {
            _indexes.emplace_back(thisRow + i - 1);
            _indexes.emplace_back(prevRow + i);
            _indexes.emplace_back(prevRow + i - 1);

            _indexes.emplace_back(thisRow + i - 1);
            _indexes.emplace_back(thisRow + i);
            _indexes.emplace_back(prevRow + i);
    			}
    		}
    		prevRow = thisRow;
    		thisRow = point;
    	}

      /* cylinder */
    	thisRow = point;
    	prevRow = 0;
    	for (unsigned int j = 0; j <= (rings + 1); j++) {
    		v = j;
    		v /= (rings + 1);

    		y = this->length * v;
    		y = (this->length * 0.5) - y;

    		for (unsigned int i = 0; i <= radialSegments; i++) {
    			u = i;
    			u /= radialSegments;

    			x = -sin(u * (M_PI * 2.0));
    			z = cos(u * (M_PI * 2.0));

    			ignition::math::Vector3d p(
            x * this->radius, y, -z * this->radius);
    			_positions.emplace_back(p);
          _uv.emplace_back(
            ignition::math::Vector2d(u, oneThird + (v * oneThird)));
    			point++;

    			if (i > 0 && j > 0) {
            _indexes.emplace_back(thisRow + i - 1);
            _indexes.emplace_back(prevRow + i);
            _indexes.emplace_back(prevRow + i - 1);

            _indexes.emplace_back(thisRow + i - 1);
            _indexes.emplace_back(thisRow + i);
            _indexes.emplace_back(prevRow + i);
    			}
    		}
    		prevRow = thisRow;
    		thisRow = point;
    	}

    	/* bottom hemisphere */
    	thisRow = point;
    	prevRow = 0;
    	for (unsigned int j = 0; j <= (rings + 1); j++) {
    		v = j;

    		v /= (rings + 1);
    		v += 1.0;
    		w = sin(0.5 * M_PI * v);
    		y = this->radius * cos(0.5 * M_PI * v);

    		for (unsigned int i = 0; i <= radialSegments; i++) {
    			float u2 = i;
    			u2 /= radialSegments;

    			x = -sin(u2 * (M_PI * 2.0));
    			z = cos(u2 * (M_PI * 2.0));

    			ignition::math::Vector3d p(
            x * this->radius * w, y, -z * this->radius * w);
    			_positions.emplace_back(
            p + ignition::math::Vector3d(0.0, -0.5 * this->length, 0.0));
          _uv.emplace_back(
            ignition::math::Vector2d(u2, twoThirds + ((v - 1.0) * oneThird)));
    			point++;

    			if (i > 0 && j > 0) {
            _indexes.emplace_back(thisRow + i - 1);
            _indexes.emplace_back(prevRow + i);
            _indexes.emplace_back(prevRow + i - 1);

            _indexes.emplace_back(thisRow + i - 1);
            _indexes.emplace_back(thisRow + i);
            _indexes.emplace_back(prevRow + i);
    			}
    		}

    		prevRow = thisRow;
    		thisRow = point;
    	}
    }
    }
  }
}
#endif
