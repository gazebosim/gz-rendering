#ifndef IGNITION_RENDERING_LUXCOREENGINE_LUXCOREENGINEMESH_HH_
#define IGNITION_RENDERING_LUXCOREENGINE_LUXCOREENGINEMESH_HH_

#include "ignition/common/Console.hh"

#include "ignition/rendering/base/BaseMesh.hh"

#include "LuxCoreEngineRenderTypes.hh"
#include "LuxCoreEngineGeometry.hh"

#include "luxcore/luxcore.h"

namespace ignition
{
  namespace rendering
  {
    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
    class LuxCoreEngineMesh :
      public BaseMesh<LuxCoreEngineGeometry>
    {
      public: LuxCoreEngineMesh();

      public: bool HasParent() const;

      public: VisualPtr Parent() const;
      
      public: void SetMaterial(MaterialPtr _material, bool _unique = true);
      
      protected: SubMeshStorePtr SubMeshes() const;
    };
    }
  }
}

#endif
