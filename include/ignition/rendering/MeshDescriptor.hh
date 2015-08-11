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
#ifndef _IGNITION_RENDERING_MESHDESCRIPTOR_HH_
#define _IGNITION_RENDERING_MESHDESCRIPTOR_HH_

#include <string>
#include "ignition/rendering/Util.hh"

namespace gazebo
{
  namespace common
  {
    class Mesh;
  }
}

namespace ignition
{

  namespace rendering
  {
    struct IGNITION_VISIBLE MeshDescriptor
    {
      public: MeshDescriptor();

      public: MeshDescriptor(const std::string &_meshName);

      public: MeshDescriptor(const gazebo::common::Mesh *_mesh);

      public: MeshDescriptor Normalize() const;

      public: const gazebo::common::Mesh *mesh;

      public: std::string meshName;

      public: std::string subMeshName;

      public: bool centerSubMesh;
    };
  }
}
#endif
