#ifndef IGNITION_RENDERING_LUXCOREENGINE_LUXCOREENGINECAMERA_HH_
#define IGNITION_RENDERING_LUXCOREENGINE_LUXCOREENGINECAMERA_HH_

#include "ignition/rendering/base/BaseCamera.hh"

#include "LuxCoreEngineRenderTypes.hh"
#include "LuxCoreEngineSensor.hh"
#include "LuxCoreEngineRenderTarget.hh"

#include "luxcore/luxcore.h"

namespace ignition
{
  namespace rendering
  {
    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
    class LuxCoreEngineCamera :
      public BaseCamera<LuxCoreEngineSensor>
    {
      protected: LuxCoreEngineCamera();

      public: virtual ~LuxCoreEngineCamera();

      public: virtual void Render();

      public: virtual void Update();

      protected: virtual RenderTargetPtr RenderTarget() const;

      protected: void SetRenderSession(luxcore::RenderSession *renderSessionPtr);

      protected: LuxCoreEngineRenderTargetPtr renderTarget;

      private: luxcore::RenderSession *renderSessionPtr;

      private: friend class LuxCoreEngineScene;
    };
    }
  }
}

#endif
