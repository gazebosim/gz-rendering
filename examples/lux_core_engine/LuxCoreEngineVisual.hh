#ifndef IGNITION_RENDERING_LUXCOREENGINE_LUXCOREENGINEVISUAL_HH_
#define IGNITION_RENDERING_LUXCOREENGINE_LUXCOREENGINEVISUAL_HH_

#include "ignition/rendering/base/BaseVisual.hh"

#include "LuxCoreEngineRenderTypes.hh"
#include "LuxCoreEngineNode.hh"
#include "LuxCoreEngineGeometry.hh"

namespace ignition
{
  namespace rendering
  {
    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
    class LuxCoreEngineVisual :
      public BaseVisual<LuxCoreEngineNode>
    {
      public: void SetLocalPosition(double _x, double _y, double _z);

      public: void SetLocalScale(double _x, double _y, double _z);
      
      public: void SetLocalRotation(double _r, double _p, double _y);

      public: void SetMaterial(MaterialPtr _material, bool _unique = true);

      protected: GeometryStorePtr Geometries() const;

      protected: bool AttachGeometry(GeometryPtr _geometry);

      protected: bool DetachGeometry(GeometryPtr _geometry);

      protected: void Init();

      private: void CreateStorage();

      private: LuxCoreEngineVisualPtr SharedThis();

      protected: LuxCoreEngineGeometryStorePtr geometries;

      private: friend class LuxCoreEngineScene;
    };
    }
  }
}

#endif
