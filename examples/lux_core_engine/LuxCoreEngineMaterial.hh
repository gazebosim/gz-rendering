#ifndef IGNITION_RENDERING_LUXCOREENGINE_LUXCOREENGINEMATERIAL_HH_
#define IGNITION_RENDERING_LUXCOREENGINE_LUXCOREENGINEMATERIAL_HH_

#include "ignition/rendering/base/BaseMaterial.hh"

#include "LuxCoreEngineRenderTypes.hh"
#include "LuxCoreEngineObject.hh"

namespace ignition
{
  namespace rendering
  {
    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
    class LuxCoreEngineMaterial :
      public BaseMaterial<LuxCoreEngineObject>
    {
      protected: LuxCoreEngineMaterial();

      public: virtual ~LuxCoreEngineMaterial();

      public: void UpdateLuxSDL();

      public: void SetDiffuse(const double _r, const double _g,
                  const double _b, const double _a = 1.0);

      public: void SetEmissive(const double _r, const double _g,
                  const double _b, const double _a = 1.0);

      public: void SetShaderType(enum ShaderType _type);

      protected: float diffuseR, diffuseG, diffuseB;

      protected: float emissiveR, emissiveG, emissiveB;

      protected: std::string materialType;

      private: friend class LuxCoreEngineScene;
    };
    }
  }
}

#endif
