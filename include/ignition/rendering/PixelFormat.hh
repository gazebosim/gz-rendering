/*
 * Copyright (C) 2015 Open Source Robotics Foundation
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
#ifndef _IGNITION_RENDERING_PIXELFORMAT_HH_
#define _IGNITION_RENDERING_PIXELFORMAT_HH_

#include <string>
#include "ignition/rendering/Util.hh"

namespace ignition
{
  namespace rendering
  {
    enum IGNITION_VISIBLE PixelFormat
    {
      PF_UNKNOWN      = 0,
      PF_L8           = 1,
      PF_R8G8B8       = 2,
      PF_B8G8R8       = 3,
      PF_BAYER_RGGB8  = 4,
      PF_BAYER_BGGR8  = 5,
      PF_BAYER_GBGR8  = 6,
      PF_BAYER_GRGB8  = 7,
      PF_COUNT        = 8,
    };

    class IGNITION_VISIBLE PixelUtil
    {
      public: static bool IsValid(PixelFormat _format);

      public: static PixelFormat Sanitize(PixelFormat _format);

      public: static std::string GetName(PixelFormat _format);

      public: static unsigned int GetChannelCount(PixelFormat _format);

      public: static unsigned int GetBytesPerChannel(PixelFormat _format);

      public: static unsigned int GetBytesPerPixel(PixelFormat _format);

      public: static unsigned int GetMemorySize(PixelFormat _format,
                  unsigned int _width, unsigned int _height);

      public: static PixelFormat GetEnum(const std::string &_name);

      private: static const char *names[PF_COUNT];

      private: static const unsigned char channelCounts[PF_COUNT];

      private: static const unsigned char channelByteCounts[PF_COUNT];
    };
  }
}
#endif
