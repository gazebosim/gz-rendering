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
#ifndef _IGNITION_RENDERING_RENDERTYPES_HH_
#define _IGNITION_RENDERING_RENDERTYPES_HH_

#include <boost/shared_ptr.hpp>

namespace ignition
{
  namespace rendering
  {
    template <class T>
    using shared_ptr = boost::shared_ptr<T>;

    class ArrowVisual;
    class AxisVisual;
    class Camera;
    class DirectionalLight;
    class Geometry;
    class JointVisual;
    class Image;
    class Light;
    class Material;
    class Mesh;
    class Node;
    class Object;
    class ObjectFactory;
    class PointLight;
    class RenderEngine;
    class Scene;
    class Sensor;
    class SpotLight;
    class SubMesh;
    class Visual;
    class RenderTarget;
    class RenderTexture;

    typedef shared_ptr<ArrowVisual>      ArrowVisualPtr;
    typedef shared_ptr<AxisVisual>       AxisVisualPtr;
    typedef shared_ptr<Camera>           CameraPtr;
    typedef shared_ptr<DirectionalLight> DirectionalLightPtr;
    typedef shared_ptr<Geometry>         GeometryPtr;
    typedef shared_ptr<JointVisual>      JointVisualPtr;
    typedef shared_ptr<Image>            ImagePtr;
    typedef shared_ptr<Light>            LightPtr;
    typedef shared_ptr<Material>         MaterialPtr;
    typedef shared_ptr<Mesh>             MeshPtr;
    typedef shared_ptr<Node>             NodePtr;
    typedef shared_ptr<Object>           ObjectPtr;
    typedef shared_ptr<ObjectFactory>    ObjectFactoryPtr;
    typedef shared_ptr<PointLight>       PointLightPtr;
    typedef shared_ptr<Scene>            ScenePtr;
    typedef shared_ptr<Sensor>           SensorPtr;
    typedef shared_ptr<SpotLight>        SpotLightPtr;
    typedef shared_ptr<SubMesh>          SubMeshPtr;
    typedef shared_ptr<Visual>           VisualPtr;
    typedef shared_ptr<RenderTarget>     RenderTargetPtr;
    typedef shared_ptr<RenderTexture>    RenderTexturePtr;

    typedef shared_ptr<const ArrowVisual>      ConstArrowVisualPtr;
    typedef shared_ptr<const AxisVisual>       ConstAxisVisualPtr;
    typedef shared_ptr<const Camera>           ConstCameraPtr;
    typedef shared_ptr<const DirectionalLight> ConstDirectionalLightPtr;
    typedef shared_ptr<const Geometry>         ConstGeometryPtr;
    typedef shared_ptr<const JointVisual>      ConstJointVisualPtr;
    typedef shared_ptr<const Image>            ConstImagePtr;
    typedef shared_ptr<const Light>            ConstLightPtr;
    typedef shared_ptr<const Material>         ConstMaterialPtr;
    typedef shared_ptr<const Mesh>             ConstMeshPtr;
    typedef shared_ptr<const Node>             ConstNodePtr;
    typedef shared_ptr<const Object>           ConstObjectPtr;
    typedef shared_ptr<const ObjectFactory>    ConstObjectFactoryPtr;
    typedef shared_ptr<const PointLight>       ConstPointLightPtr;
    typedef shared_ptr<const Scene>            ConstScenePtr;
    typedef shared_ptr<const Sensor>           ConstSensorPtr;
    typedef shared_ptr<const SpotLight>        ConstSpotLightPtr;
    typedef shared_ptr<const SubMesh>          ConstSubMeshPtr;
    typedef shared_ptr<const Visual>           ConstVisualPtr;
    typedef shared_ptr<const RenderTarget>     ConstRenderTargetPtr;
    typedef shared_ptr<const RenderTexture>    ConstRenderTexturePtr;

    /*
    template <class T> Map;
    template <class T> Store;
    template <class T> CompositeStore;

    typedef Map<Material>        MaterialMap;
    typedef Map<SubMesh>         SubMeshMap;
    typedef Store<Scene>         SceneStore;
    typedef Store<Node>          NodeStore;
    typedef Store<Light>         LightStore;
    typedef Store<Sensor>        SensorStore;
    typedef Store<Visual>        VisualStore;
    typedef Store<Geometry>      GeometryStore;
    typedef CompositeStore<Node> NodeCompositeStore;

    typedef shared_ptr<SceneStore>         SceneStorePtr;
    typedef shared_ptr<NodeStore>          NodeStorePtr;
    typedef shared_ptr<LightStore>         LightStorePtr;
    typedef shared_ptr<SensorStore>        SensorStorePtr;
    typedef shared_ptr<VisualStore>        VisualStorePtr;
    typedef shared_ptr<GeometryStore>      GeometryStorePtr;
    typedef shared_ptr<MaterialMap>        MaterialMapPtr;
    typedef shared_ptr<SubMeshMap>         SubMeshMapPtr;
    typedef shared_ptr<NodeCompositeStore> NodeCompositeStorePtr;
    */
  }
}
#endif
