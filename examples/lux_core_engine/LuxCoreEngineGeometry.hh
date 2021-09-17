#ifndef IGNITION_RENDERING_LUXCOREENGINE_LUXCOREENGINEGEOMETRY_HH_
#define IGNITION_RENDERING_LUXCOREENGINE_LUXCOREENGINEGEOMETRY_HH_

#include "ignition/rendering/base/BaseGeometry.hh"

#include "LuxCoreEngineRenderTypes.hh"
#include "LuxCoreEngineObject.hh"

namespace ignition
{
  namespace rendering
  {
    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
    class LuxCoreEngineGeometry :
      public BaseGeometry<LuxCoreEngineObject>
    {
      public: LuxCoreEngineGeometry();

      public: void SetLocalPosition(double _x, double _y, double _z);

      public: void SetLocalScale(double _x, double _y, double _z);

      private: float transformationMatrix[16];
    };
    }
  }
}

#endif
