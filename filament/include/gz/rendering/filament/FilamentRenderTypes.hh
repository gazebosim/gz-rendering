/*
 * Copyright (C) 2026 Open Source Robotics Foundation
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
#ifndef GZ_RENDERING_FILAMENT_FILAMENTRENDERTYPES_HH_
#define GZ_RENDERING_FILAMENT_FILAMENTRENDERTYPES_HH_

#include <memory>

#include "gz/rendering/config.hh"
#include "gz/rendering/base/BaseRenderTypes.hh"
#include "gz/rendering/base/BaseStorage.hh"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    //
    class FilamentCamera;
    class FilamentDepthCamera;
    class FilamentGpuRays;
    class FilamentBoundingBoxCamera;
    class FilamentSegmentationCamera;
    class FilamentThermalCamera;
    class FilamentDirectionalLight;
    class FilamentGeometry;
    class FilamentHeightmap;
    class FilamentLight;
    class FilamentMaterial;
    class FilamentMesh;
    class FilamentMeshFactory;
    class FilamentNode;
    class FilamentObject;
    class FilamentParticleEmitter;
    class FilamentPointLight;
    class FilamentRenderEngine;
    class FilamentRenderTarget;
    class FilamentRenderTexture;
    class FilamentScene;
    class FilamentSensor;
    class FilamentSpotLight;
    class FilamentSubMesh;
    class FilamentVisual;
    class FilamentWideAngleCamera;

    typedef BaseGeometryStore<FilamentGeometry>    FilamentGeometryStore;
    typedef BaseLightStore<FilamentLight>          FilamentLightStore;
    typedef BaseNodeStore<FilamentNode>            FilamentNodeStore;
    typedef BaseSceneStore<FilamentScene>          FilamentSceneStore;
    typedef BaseSensorStore<FilamentSensor>        FilamentSensorStore;
    typedef BaseSubMeshStore<FilamentSubMesh>      FilamentSubMeshStore;
    typedef BaseVisualStore<FilamentVisual>        FilamentVisualStore;
    typedef BaseMaterialMap<FilamentMaterial>      FilamentMaterialMap;

    typedef shared_ptr<FilamentCamera>             FilamentCameraPtr;
    typedef shared_ptr<FilamentDepthCamera>        FilamentDepthCameraPtr;
    typedef shared_ptr<FilamentGpuRays>            FilamentGpuRaysPtr;
    typedef shared_ptr<FilamentBoundingBoxCamera>  FilamentBoundingBoxCameraPtr;
    typedef shared_ptr<FilamentSegmentationCamera>
        FilamentSegmentationCameraPtr;
    typedef shared_ptr<FilamentThermalCamera>      FilamentThermalCameraPtr;
    typedef shared_ptr<FilamentDirectionalLight>   FilamentDirectionalLightPtr;
    typedef shared_ptr<FilamentGeometry>           FilamentGeometryPtr;
    typedef shared_ptr<FilamentHeightmap>          FilamentHeightmapPtr;
    typedef shared_ptr<FilamentLight>              FilamentLightPtr;
    typedef shared_ptr<FilamentMaterial>           FilamentMaterialPtr;
    typedef shared_ptr<FilamentMesh>               FilamentMeshPtr;
    typedef shared_ptr<FilamentParticleEmitter>  FilamentParticleEmitterPtr;
    typedef shared_ptr<FilamentMeshFactory>        FilamentMeshFactoryPtr;
    typedef shared_ptr<FilamentNode>               FilamentNodePtr;
    typedef shared_ptr<FilamentObject>             FilamentObjectPtr;
    typedef shared_ptr<FilamentPointLight>         FilamentPointLightPtr;
    typedef shared_ptr<FilamentRenderTarget>       FilamentRenderTargetPtr;
    typedef shared_ptr<FilamentRenderTexture>      FilamentRenderTexturePtr;
    typedef shared_ptr<FilamentScene>              FilamentScenePtr;
    typedef shared_ptr<FilamentSensor>             FilamentSensorPtr;
    typedef shared_ptr<FilamentSpotLight>          FilamentSpotLightPtr;
    typedef shared_ptr<FilamentSubMesh>            FilamentSubMeshPtr;
    typedef shared_ptr<FilamentVisual>             FilamentVisualPtr;
    typedef shared_ptr<FilamentWideAngleCamera>    FilamentWideAngleCameraPtr;

    typedef shared_ptr<FilamentGeometryStore>      FilamentGeometryStorePtr;
    typedef shared_ptr<FilamentLightStore>         FilamentLightStorePtr;
    typedef shared_ptr<FilamentNodeStore>          FilamentNodeStorePtr;
    typedef shared_ptr<FilamentSceneStore>         FilamentSceneStorePtr;
    typedef shared_ptr<FilamentSensorStore>        FilamentSensorStorePtr;
    typedef shared_ptr<FilamentSubMeshStore>       FilamentSubMeshStorePtr;
    typedef shared_ptr<FilamentVisualStore>        FilamentVisualStorePtr;
    typedef shared_ptr<FilamentMaterialMap>        FilamentMaterialMapPtr;
    }
  }
}
#endif
