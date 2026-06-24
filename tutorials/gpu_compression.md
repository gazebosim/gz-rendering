\page gpucompression GPU image compression (Phase 1)

The ogre2 color camera can deliver frames as GPU-converted NV12 (semi-planar
YUV 4:2:0) instead of a raw CPU readback, via an asynchronous, non-blocking
path. This reduces both the render-thread stall and the GPU->CPU byte volume.

## Usage

\code{.cpp}
camera->SetImageEncoding(gz::rendering::IE_NV12);  // requires the Vulkan backend
if (!camera->IsEncodingSupported(gz::rendering::IE_NV12))
{
  // falls back to the raw Image path
}
auto conn = camera->ConnectNewCompressedImageFrame(
    [](const gz::rendering::CompressedImage &img)
    {
      // img.Data() is valid only during this callback; copy to retain.
      // img.Size() == width*height*3/2 for NV12.
      // img.Colorimetry(): BT.709, limited range, sRGB transfer.
    });
\endcode

NV12 is the input format for the (future) Phase 2 Vulkan Video H.264/H.265
encoder, which will plug in behind the same `IE_H264`/`IE_H265` encodings.
