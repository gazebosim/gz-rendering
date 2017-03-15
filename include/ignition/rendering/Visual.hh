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
#ifndef _IGNITION_RENDERING_VISUAL_HH_
#define _IGNITION_RENDERING_VISUAL_HH_

#include "ignition/rendering/Node.hh"

namespace ignition
{
  namespace rendering
  {
    /// \class Visual Visual.hh ignition/rendering/Visual.hh
    /// \brief Represents a visual node in a scene graph. A Visual is the only
    /// node that can have Geometry and other Visual children.
    class IGNITION_VISIBLE Visual :
      public virtual Node
    {
      /// \brief Deconstructor
      public: virtual ~Visual() { }

      /// \brief Get number of child nodes
      /// \return The number of child nodes
      public: virtual unsigned int ChildCount() const = 0;

      /// \brief Determine if given node is an attached child
      /// \return True if given node is an attached child
      public: virtual bool HasChild(ConstNodePtr _child) const = 0;

      /// \brief Determine if node with given ID is an attached child
      /// \param[in] _id ID of the node in question
      /// \return True if node with given ID is an attached child
      public: virtual bool HasChildId(unsigned int _id) const = 0;

      /// \brief Determine if node with given name is an attached child
      /// \param[in] _name Name of the node in question
      /// \return True if node with given name is an attached child
      public: virtual bool HasChildName(const std::string &_name) const = 0;

      /// \brief Get node with given ID. If no child exists with given ID, NULL
      /// will be returned.
      /// \param[in] _id ID of the desired node
      /// \return The specified node
      public: virtual NodePtr ChildById(unsigned int _id) const = 0;

      /// \brief Get node with given name. If no child exists with given name,
      /// NULL will be returned.
      /// \param[in] _name Name of the desired node
      /// \return The specified node
      public: virtual NodePtr ChildByName(
                  const std::string &_name) const = 0;

      /// \brief Get node at given index. If no child exists at given index,
      /// NULL will be returned.
      /// \param[in] _index Index of the desired node
      /// \return The specified node
      public: virtual NodePtr ChildByIndex(unsigned int _index) const = 0;

      /// \brief Add the given node to this visual. If the given node is
      /// already a child, no work will be done.
      /// \param[in] _child Child node to be added
      public: virtual void AddChild(NodePtr _child) = 0;

      /// \brief Remove the given node from this visual. If the given node is
      /// not a child of this visual, no work will be done.
      /// \param[in] _child Child node to be removed
      /// \return The removed child node
      public: virtual NodePtr RemoveChild(NodePtr _child) = 0;

      /// \brief Remove the node with the given ID from this visual. If the
      /// specified node is not a child of this visual, no work will be done.
      /// \param[in] _id ID of the child node to be removed
      /// \return The removed child node
      public: virtual NodePtr RemoveChildById(unsigned int _id) = 0;

      /// \brief Remove the node with the given name from this visual. If the
      /// specified node is not a child of this visual, no work will be done.
      /// \param[in] _name Name of the child node to be removed
      /// \return The removed child node
      public: virtual NodePtr RemoveChildByName(const std::string &_name) = 0;

      /// \brief Remove the node at the given index from this visual. If the
      /// specified node is not a child of this visual, no work will be done.
      /// \param[in] _index Index of the child node to be removed
      /// \return The removed child node
      public: virtual NodePtr RemoveChildByIndex(unsigned int _index) = 0;

      /// \brief Remove all child nodes from this visual
      public: virtual void RemoveChildren() = 0;

      /// \brief Get the number of geometries attached to this visual
      /// \return The number of geometries attached to this visual
      public: virtual unsigned int GeometryCount() const = 0;

      /// \brief Determine if given geometry is attached to this visual
      /// \param[in] _geometry Geometry in question
      /// \return True if given node is attached to this visual
      public: virtual bool HasGeometry(ConstGeometryPtr _geometry) const = 0;

      /// \brief Get geometry at given index. If no geometry exists at given
      /// index, NULL will be returned.
      /// \param[in] _index Index of the desired geometry
      /// \return The specified geometry
      public: virtual GeometryPtr GeometryByIndex(
                  unsigned int _index) const = 0;

      /// \brief Add the given geometry to this visual. If the given node is
      /// already attached, no work will be done.
      /// \param[in] _geometry Geometry to be added
      public: virtual void AddGeometry(GeometryPtr _geometry) = 0;

      /// \brief Remove the given geometry from this visual. If the given node
      /// is not a child of this visual, no work will be done.
      /// \param[in] _geometry Geometry to be removed
      /// \return The removed geometry
      public: virtual GeometryPtr RemoveGeometry(GeometryPtr _geometry) = 0;

      /// \brief Remove the geometry at the given index from this visual. If the
      /// specified node is not attached this visual, no work will be done.
      /// \param[in] _index Index of the geometry to be removed
      /// \return The removed geometry
      public: virtual GeometryPtr RemoveGeometryByIndex(
                  unsigned int _index) = 0;

      /// \brief Remove all attached geometries from this visual
      public: virtual void RemoveGeometries() = 0;

      /// \brief Set the material for all attached visuals and geometries. The
      /// specified material will be retrieved from Scene::Material using
      /// the given material name. If no material is registered with the given
      /// name, no work will be done.
      /// \param[in] _name Name of the material to be assigned
      /// \param[in] _unique True if the specified material should be cloned
      public: virtual void SetMaterial(const std::string &_name,
                  bool unique = true) = 0;

      /// \brief Set the material for all attached visuals and geometries
      /// \param[in] _name Name of the material to be assigned
      /// \param[in] _unique True if the specified material should be cloned
      public: virtual void SetMaterial(MaterialPtr _material,
                  bool unique = true) = 0;

      /// \brief Set the material for all attached visuals only
      /// \param[in] _name Name of the material to be assigned
      /// \param[in] _unique True if the specified material should be cloned
      public: virtual void SetChildMaterial(MaterialPtr _material,
                  bool unique = true) = 0;

      /// \brief Set the material for all attached geometries only
      /// \param[in] _name Name of the material to be assigned
      /// \param[in] _unique True if the specified material should be cloned
      public: virtual void SetGeometryMaterial(MaterialPtr _material,
                  bool unique = true) = 0;

      /// \brief Get the local scale
      /// \return The local scale
      public: virtual math::Vector3d LocalScale() const = 0;

      /// \brief Set the local scale. The given scale will be assigned to the
      /// x, y, and z coordinates.
      /// \param[in] _scale New local scale
      public: virtual void SetLocalScale(double _scale) = 0;

      /// \brief Set the local scale
      /// \param[in] _x New x-coordinate scale
      /// \param[in] _y New y-coordinate scale
      /// \param[in] _z New z-coordinate scale
      public: virtual void SetLocalScale(double _x, double _y, double _z) = 0;

      /// \brief Set the local scale
      /// \param[in] _scale New local scale
      public: virtual void SetLocalScale(const math::Vector3d &_scale) = 0;

      /// \brief Get the world scale
      /// \return The world scale
      public: virtual math::Vector3d WorldScale() const = 0;

      /// \brief Set the world scale. The given scale will be assigned to the
      /// x, y, and z coordinates.
      /// \param[in] _scale New world scale
      public: virtual void SetWorldScale(double _scale) = 0;

      /// \brief Set the world scale
      /// \param[in] _x New x-coordinate scale
      /// \param[in] _y New y-coordinate scale
      /// \param[in] _z New z-coordinate scale
      public: virtual void SetWorldScale(double _x, double _y, double _z) = 0;

      /// \brief Set the world scale
      /// \param[in] _scale New world scale
      public: virtual void SetWorldScale(const math::Vector3d &_scale) = 0;

      /// \brief Scale the current scale by the given scalar. The given scalar
      /// will be assigned to the x, y, and z coordinates.
      /// \param[in] _scale Scalar to alter the current scale
      public: virtual void Scale(double _scale) = 0;

      /// \brief Scale the current scale by the given scalars
      /// \param[in] _x Scalar to alter the current x-coordinate scale
      /// \param[in] _y Scalar to alter the current y-coordinate scale
      /// \param[in] _z Scalar to alter the current z-coordinate scale
      public: virtual void Scale(double _x, double _y, double _z) = 0;

      /// \brief Scale the current scale by the given scalars
      /// \param[in] _scale Scalars to alter the current scale
      public: virtual void Scale(const math::Vector3d &_scale) = 0;

      /// \brief Determine if this visual inherits scale from this parent
      /// \return True if this visual inherits scale from this parent
      public: virtual bool InheritScale() const = 0;

      /// \brief Specify if this visual inherits scale from its parent
      /// \param[in] _inherit True if this visual inherits scale from its parent
      public: virtual void SetInheritScale(bool _inherit) = 0;
    };
  }
}
#endif
