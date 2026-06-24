/*
 * Copyright (C) 2026 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#ifndef GZ_RENDERING_COMPRESSEDIMAGE_HH_
#define GZ_RENDERING_COMPRESSEDIMAGE_HH_

#include <cstdint>

#include <gz/utils/ImplPtr.hh>

#include "gz/rendering/config.hh"
#include "gz/rendering/Export.hh"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    //
    /// \enum ImageEncoding
    /// \brief Wire encoding of a CompressedImage. Distinct from PixelFormat,
    /// which describes a raw pixel layout.
    enum GZ_RENDERING_VISIBLE ImageEncoding : uint16_t
    {
      /// \brief Raw path (the plain Image); compressed output disabled.
      IE_NONE  = 0,
      /// \brief Semi-planar YUV 4:2:0 (Y plane + interleaved CbCr); size w*h*3/2.
      IE_NV12  = 1,
      /// \brief H.264 bitstream (Phase 2; unsupported in Phase 1).
      IE_H264  = 2,
      /// \brief H.265 bitstream (Phase 2; unsupported in Phase 1).
      IE_H265  = 3,
      /// \brief JPEG (optional future).
      IE_JPEG  = 4,
      /// \brief Number of encodings.
      IE_COUNT = 5
    };

    /// \struct ColorimetryInfo
    /// \brief Colour metadata carried with every compressed/NV12 frame so a
    /// decoder reconstructs correct colours. Defaults: BT.709, limited range,
    /// sRGB transfer preserved.
    struct GZ_RENDERING_VISIBLE ColorimetryInfo
    {
      /// \brief H.264 matrix_coefficients (1 = BT.709).
      public: uint8_t matrixCoefficients = 1u;
      /// \brief H.264 colour_primaries (1 = BT.709).
      public: uint8_t colourPrimaries = 1u;
      /// \brief H.264 transfer_characteristics (13 = IEC 61966-2-1 sRGB).
      public: uint8_t transferCharacteristics = 13u;
      /// \brief video_full_range_flag; false = limited/studio swing.
      public: bool fullRange = false;
    };

    /// \class CompressedImage CompressedImage.hh gz/rendering/CompressedImage.hh
    /// \brief Encapsulates a variable-length, GPU-encoded image buffer.
    /// The internal buffer is reused/grown across frames; Data() is valid only
    /// for the duration of the delivery callback (copy to retain).
    class GZ_RENDERING_VISIBLE CompressedImage
    {
      /// \brief Default constructor.
      public: CompressedImage();

      /// \brief Constructor.
      /// \param[in] _width Source image width in pixels.
      /// \param[in] _height Source image height in pixels.
      /// \param[in] _encoding Wire encoding.
      public: CompressedImage(unsigned int _width, unsigned int _height,
                  ImageEncoding _encoding);

      /// \brief Destructor.
      public: ~CompressedImage();

      /// \brief Get a const pointer to the encoded bytes (or nullptr if empty).
      public: const void *Data() const;

      /// \brief Get the number of valid encoded bytes this frame.
      public: unsigned int Size() const;

      /// \brief Get the source image width in pixels.
      public: unsigned int Width() const;

      /// \brief Get the source image height in pixels.
      public: unsigned int Height() const;

      /// \brief Get the wire encoding.
      public: ImageEncoding Encoding() const;

      /// \brief Get the colour metadata.
      public: ColorimetryInfo Colorimetry() const;

      /// \brief True for non-bitstream encodings (NV12/JPEG/NONE) and for
      /// IDR/I frames of a bitstream encoding; false for P frames.
      public: bool KeyFrame() const;

      /// \brief Set the source dimensions.
      public: void SetDimensions(unsigned int _width, unsigned int _height);

      /// \brief Set the wire encoding.
      public: void SetEncoding(ImageEncoding _encoding);

      /// \brief Set the colour metadata.
      public: void SetColorimetry(const ColorimetryInfo &_info);

      /// \brief Set the key-frame flag.
      public: void SetKeyFrame(bool _keyFrame);

      /// \brief Ensure the internal buffer holds at least _size bytes, set the
      /// logical Size() to _size, and return a writable pointer to it. Capacity
      /// only grows; smaller requests reuse the buffer.
      /// \param[in] _size Number of bytes to make available.
      /// \return Writable pointer to at least _size bytes.
      public: unsigned char *Reserve(unsigned int _size);

      GZ_UTILS_IMPL_PTR(dataPtr)
    };
    }
  }
}
#endif
