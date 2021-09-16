#ifndef IGNITION_RENDERING_LUXCOREENGINE_LUXCOREENGINERENDERTARGET_HH_
#define IGNITION_RENDERING_LUXCOREENGINE_LUXCOREENGINERENDERTARGET_HH_

#include "ignition/rendering/base/BaseRenderTarget.hh"

#include "LuxCoreEngineRenderTypes.hh"
#include "LuxCoreEngineObject.hh"

namespace ignition
{
  namespace rendering
  {
    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
    class LuxCoreEngineRenderTarget :
        public virtual BaseRenderTarget<LuxCoreEngineObject>
    {
      protected: LuxCoreEngineRenderTarget();

      public: virtual ~LuxCoreEngineRenderTarget();

      public: virtual void Copy(Image &_image) const;

      public: void *HostDataBuffer();

      public: void ResizeHostDataBuffer(unsigned int size);

      protected: unsigned int MemorySize() const;

      protected: virtual void RebuildImpl();

      protected: void *hostDataBuffer;

      private: friend class LuxCoreEngineCamera;
    };
    }
  }
}
#endif
