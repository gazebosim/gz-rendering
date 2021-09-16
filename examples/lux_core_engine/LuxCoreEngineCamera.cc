#include "LuxCoreEngineCamera.hh"

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
  this->renderSessionPtr->Start();
  usleep(120000);
  this->renderSessionPtr->Stop();
  luxcore::Film& film = this->renderSessionPtr->GetFilm();     
  
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

void LuxCoreEngineCamera::SetRenderSession(luxcore::RenderSession *renderSessionPtr)
{
  this->renderSessionPtr = renderSessionPtr;
};
