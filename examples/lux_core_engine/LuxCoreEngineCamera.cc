#include "LuxCoreEngineCamera.hh"

#include "LuxCoreEngineScene.hh"

using namespace ignition;
using namespace rendering;

LuxCoreEngineCamera::LuxCoreEngineCamera()
{
  this->renderTarget = LuxCoreEngineRenderTargetPtr(new LuxCoreEngineRenderTarget);
  this->renderTarget->SetFormat(PF_R8G8B8);
};

LuxCoreEngineCamera::~LuxCoreEngineCamera() {};

void LuxCoreEngineCamera::Render() 
{
  if (!this->renderSessionLux)
  { 
    luxrays::Properties props;
    props.Set(luxrays::Property("renderengine.type")("PATHCPU")); 
    props.Set(luxrays::Property("film.width")("640")); 
    props.Set(luxrays::Property("film.height")("480")); 
    props.Set(luxrays::Property("film.imagepipeline.0.type")("TONEMAP_LINEAR")); 
    props.Set(luxrays::Property("film.imagepipeline.1.type")("GAMMA_CORRECTION")); 
    props.Set(luxrays::Property("film.imagepipeline.1.value")("2.2")); 
    luxcore::RenderConfig *config = luxcore::RenderConfig::Create(props, scene->SceneLux());
    this->renderSessionLux = luxcore::RenderSession::Create(config);
  }

  this->renderSessionLux->Start();
  usleep(120000);
  this->renderSessionLux->Stop();
  this->renderSessionLux->WaitForDone();

  luxcore::Film& film = this->renderSessionLux->GetFilm();     
  
  float *luxcoreBuffer = (float*)malloc(film.GetWidth() * film.GetHeight() * 3 * sizeof(float));
  film.GetOutput(luxcore::Film::OUTPUT_RGB_IMAGEPIPELINE, luxcoreBuffer); 

  if (this->renderTarget->HostDataBuffer() == NULL) 
  {
    this->renderTarget->ResizeHostDataBuffer(this->ImageWidth() * this->ImageHeight() * 3);
  }

  unsigned char* buffer = (unsigned char*)this->renderTarget->HostDataBuffer();
  for (unsigned int x = 0; x < this->ImageHeight() * this->ImageWidth() * 3; x++) {
    buffer[x] = luxcoreBuffer[x] * 255;
  }

  free(luxcoreBuffer);
};

void LuxCoreEngineCamera::Update() 
{
  this->Render();
};

RenderTargetPtr LuxCoreEngineCamera::RenderTarget() const 
{
  return this->renderTarget;
};
