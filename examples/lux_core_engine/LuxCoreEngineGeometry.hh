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

      public: virtual void SetLocalPosition(double _x, double _y, double _z) = 0;

      public: virtual void SetLocalScale(double _x, double _y, double _z) = 0;

      public: virtual void SetLocalRotation(double _r, double _p, double _y) = 0;
    };
    }
  }
}

#endif
