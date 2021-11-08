#include <ignition/common/Console.hh>

#include "LuxCoreEngineRenderTarget.hh"

using namespace ignition;
using namespace rendering;

LuxCoreEngineRenderTarget::LuxCoreEngineRenderTarget() : hostDataBuffer(0) {}

LuxCoreEngineRenderTarget::~LuxCoreEngineRenderTarget() {}

void LuxCoreEngineRenderTarget::Copy(Image &_image) const {
  if (_image.Width() != this->width || _image.Height() != this->height) {
    ignerr << "Invalid image dimensions" << std::endl;
    return;
  }

  if (this->hostDataBuffer == NULL) {
    ignerr << "Host buffer is NULL" << std::endl;
    return;
  }

  void *imageData = _image.Data<void>();
  memcpy(imageData, this->hostDataBuffer, this->width * this->height * 3);
}

void *LuxCoreEngineRenderTarget::HostDataBuffer() {
  return this->hostDataBuffer;
}

void LuxCoreEngineRenderTarget::ResizeHostDataBuffer(unsigned int size) {
  this->hostDataBuffer = malloc(size);
}

unsigned int LuxCoreEngineRenderTarget::MemorySize() const {}

void LuxCoreEngineRenderTarget::RebuildImpl() {}
