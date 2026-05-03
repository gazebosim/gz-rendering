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
#ifndef GZ_RENDERING_OGRE_NEXT_OGRE_NEXTRENDERTYPES_HH_
#define GZ_RENDERING_OGRE_NEXT_OGRE_NEXTRENDERTYPES_HH_

#include <memory>

#include "gz/rendering/config.hh"
#include "gz/rendering/base/BaseRenderTypes.hh"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    //
    class OgreNextArrowVisual;
    class OgreNextAxisVisual;
    class OgreNextBoundingBoxCamera;
    class OgreNextCamera;
    class OgreNextCapsule;
    class OgreNextCOMVisual;
    class OgreNextDepthCamera;
    class OgreNextDirectionalLight;
    class OgreNextFrustumVisual;
    class OgreNextGeometry;
    class OgreNextGizmoVisual;
    class OgreNextGlobalIlluminationCiVct;
    class OgreNextGlobalIlluminationVct;
    class OgreNextGpuRays;
    class OgreNextGrid;
    class OgreNextHeightmap;
    class OgreNextInertiaVisual;
    class OgreNextJointVisual;
    class OgreNextLight;
    class OgreNextLightVisual;
    class OgreNextLidarVisual;
    class OgreNextMarker;
    class OgreNextMaterial;
    class OgreNextMesh;
    class OgreNextMeshFactory;
    class OgreNextNode;
    class OgreNextObject;
    class OgreNextObjectInterface;
    class OgreNextParticleEmitter;
    class OgreNextProjector;
    class OgreNextPointLight;
    class OgreNextRayQuery;
    class OgreNextRenderEngine;
    class OgreNextRenderTarget;
    class OgreNextRenderTargetMaterial;
    class OgreNextRenderTexture;
    class OgreNextRenderWindow;
    class OgreNextScene;
    class OgreNextSegmentationCamera;
    class OgreNextSensor;
    class OgreNextSpotLight;
    class OgreNextSubMesh;
    class OgreNextThermalCamera;
    class OgreNextVisual;
    class OgreNextWideAngleCamera;
    class OgreNextWireBox;

    typedef BaseGeometryStore<OgreNextGeometry>      OgreNextGeometryStore;
    typedef BaseLightStore<OgreNextLight>            OgreNextLightStore;
    typedef BaseNodeStore<OgreNextNode>              OgreNextNodeStore;
    typedef BaseSceneStore<OgreNextScene>            OgreNextSceneStore;
    typedef BaseSensorStore<OgreNextSensor>          OgreNextSensorStore;
    typedef BaseSubMeshStore<OgreNextSubMesh>        OgreNextSubMeshStore;
    typedef BaseVisualStore<OgreNextVisual>          OgreNextVisualStore;

    typedef BaseMaterialMap<OgreNextMaterial>        OgreNextMaterialMap;

    typedef shared_ptr<OgreNextArrowVisual>          OgreNextArrowVisualPtr;
    typedef shared_ptr<OgreNextAxisVisual>           OgreNextAxisVisualPtr;
    typedef shared_ptr<OgreNextBoundingBoxCamera>
        OgreNextBoundingBoxCameraPtr;
    typedef shared_ptr<OgreNextCamera>               OgreNextCameraPtr;
    typedef shared_ptr<OgreNextCapsule>              OgreNextCapsulePtr;
    typedef shared_ptr<OgreNextCOMVisual>            OgreNextCOMVisualPtr;
    typedef shared_ptr<OgreNextDepthCamera>          OgreNextDepthCameraPtr;
    typedef shared_ptr<OgreNextDirectionalLight>
        OgreNextDirectionalLightPtr;
    typedef shared_ptr<OgreNextFrustumVisual>        OgreNextFrustumVisualPtr;
    typedef shared_ptr<OgreNextGeometry>             OgreNextGeometryPtr;
    typedef shared_ptr<OgreNextGizmoVisual>          OgreNextGizmoVisualPtr;
    typedef shared_ptr<OgreNextGpuRays>              OgreNextGpuRaysPtr;
    typedef shared_ptr<OgreNextGrid>                 OgreNextGridPtr;
    typedef shared_ptr<OgreNextHeightmap>            OgreNextHeightmapPtr;
    typedef shared_ptr<OgreNextInertiaVisual>        OgreNextInertiaVisualPtr;
    typedef shared_ptr<OgreNextJointVisual>          OgreNextJointVisualPtr;
    typedef shared_ptr<OgreNextLight>                OgreNextLightPtr;
    typedef shared_ptr<OgreNextLightVisual>          OgreNextLightVisualPtr;
    typedef shared_ptr<OgreNextLidarVisual>          OgreNextLidarVisualPtr;
    typedef shared_ptr<OgreNextMarker>               OgreNextMarkerPtr;
    typedef shared_ptr<OgreNextMaterial>             OgreNextMaterialPtr;
    typedef shared_ptr<OgreNextMesh>                 OgreNextMeshPtr;
    typedef shared_ptr<OgreNextMeshFactory>          OgreNextMeshFactoryPtr;
    typedef shared_ptr<OgreNextNode>                 OgreNextNodePtr;
    typedef shared_ptr<OgreNextObject>               OgreNextObjectPtr;
    typedef shared_ptr<OgreNextObjectInterface>      OgreNextObjectInterfacePtr;
    typedef shared_ptr<OgreNextParticleEmitter>      OgreNextParticleEmitterPtr;
    typedef shared_ptr<OgreNextGlobalIlluminationCiVct>
      OgreNextGlobalIlluminationCiVctPtr;
    typedef shared_ptr<OgreNextGlobalIlluminationVct>
      OgreNextGlobalIlluminationVctPtr;
    typedef shared_ptr<OgreNextPointLight>           OgreNextPointLightPtr;
    typedef shared_ptr<OgreNextProjector>            OgreNextProjectorPtr;
    typedef shared_ptr<OgreNextRayQuery>             OgreNextRayQueryPtr;
    typedef shared_ptr<OgreNextRenderEngine>         OgreNextRenderEnginePtr;
    typedef shared_ptr<OgreNextRenderTarget>         OgreNextRenderTargetPtr;
    typedef shared_ptr<OgreNextRenderTexture>        OgreNextRenderTexturePtr;
    typedef shared_ptr<OgreNextRenderWindow>         OgreNextRenderWindowPtr;
    typedef shared_ptr<OgreNextScene>                OgreNextScenePtr;
    typedef shared_ptr<OgreNextSegmentationCamera>
      OgreNextSegmentationCameraPtr;
    typedef shared_ptr<OgreNextSensor>               OgreNextSensorPtr;
    typedef shared_ptr<OgreNextSpotLight>            OgreNextSpotLightPtr;
    typedef shared_ptr<OgreNextSubMesh>              OgreNextSubMeshPtr;
    typedef shared_ptr<OgreNextThermalCamera>        OgreNextThermalCameraPtr;
    typedef shared_ptr<OgreNextVisual>               OgreNextVisualPtr;
    typedef shared_ptr<OgreNextWideAngleCamera>      OgreNextWideAngleCameraPtr;
    typedef shared_ptr<OgreNextWireBox>              OgreNextWireBoxPtr;

    typedef shared_ptr<OgreNextGeometryStore>        OgreNextGeometryStorePtr;
    typedef shared_ptr<OgreNextLightStore>           OgreNextLightStorePtr;
    typedef shared_ptr<OgreNextNodeStore>            OgreNextNodeStorePtr;
    typedef shared_ptr<OgreNextRenderTargetMaterial>
        OgreNextRenderTargetMaterialPtr;
    typedef shared_ptr<OgreNextSceneStore>           OgreNextSceneStorePtr;
    typedef shared_ptr<OgreNextSensorStore>          OgreNextSensorStorePtr;
    typedef shared_ptr<OgreNextSubMeshStore>         OgreNextSubMeshStorePtr;
    typedef shared_ptr<OgreNextVisualStore>          OgreNextVisualStorePtr;

    typedef shared_ptr<OgreNextMaterialMap>          OgreNextMaterialMapPtr;
    }
  }
}
#endif
