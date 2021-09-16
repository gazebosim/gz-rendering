#ifndef IGNITION_RENDERING_LUXCOREENGINE_LUXCOREENGINEMESHFACTORY_HH_
#define IGNITION_RENDERING_LUXCOREENGINE_LUXCOREENGINEMESHFACTORY_HH_

#include "LuxCoreEngineRenderTypes.hh"
#include "LuxCoreEngineMesh.hh"

#include "luxcore/luxcore.h"

namespace ignition
{
  namespace rendering
  {
    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
    class LuxCoreEngineMeshFactory
    {
      public: LuxCoreEngineMeshFactory(LuxCoreEngineScenePtr _scene);

      public: LuxCoreEngineMeshPtr Create(const MeshDescriptor &_desc, const std::string &_name);
      
      protected: LuxCoreEngineScenePtr scene;
    };
    }
  }
}

#endif
