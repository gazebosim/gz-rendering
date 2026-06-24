\page gpucompression GPU image compression (Phase 1)

The ogre2 color camera can deliver frames as GPU-converted NV12 (semi-planar
YUV 4:2:0) instead of a raw CPU readback. The conversion runs on the GPU and
the result is read back asynchronously, without blocking the render thread on a
full-size copy. This cuts the GPU->CPU byte volume to 37.5% of RGBA8; avoiding
the render-thread readback stall is the design goal of this async path (the
`gpu_compress_bench` example reports the measured numbers).

## Usage

\code{.cpp}
camera->SetImageEncoding(gz::rendering::IE_NV12);  // requires the Vulkan backend
if (!camera->IsEncodingSupported(gz::rendering::IE_NV12))
{
  // falls back to the raw Image path
}
camera->Encoding();  // query the currently active ImageEncoding

// Keep 'conn' alive for as long as you want frames; destroying the returned
// gz::common::ConnectionPtr disconnects the callback.
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
