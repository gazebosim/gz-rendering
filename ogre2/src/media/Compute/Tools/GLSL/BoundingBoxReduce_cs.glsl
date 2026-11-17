// Copyright (C) 2026 Open Source Robotics Foundation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Bounding box camera: finds the pixel extent of every object index in the
// index image rendered by Ogre2BoundingBoxMaterialSwitcher, so only the
// extents (16 bytes per object) have to be read back instead of the image.
//
// Each pixel stores an object index in 24 bits (R: bits 0-7, G: 8-15,
// B: 16-23); 0 is the background. Each thread scans a horizontal strip of
// one row and, for every run of pixels with the same index, updates that
// index's extent with atomics. For index i, boxes[4i .. 4i+3] hold:
//   0xFFFFFFFF - minX, 0xFFFFFFFF - minY, maxX, maxY
// Minimums are stored inverted so every field is updated with atomicMax and
// a buffer cleared to zero means "not seen".
//
// OpenGL only for now (GL 4.3); other APIs use the CPU scan.

#version 430

// Read as a plain texture, not an image, so the render target doesn't need
// to be created as a UAV (which can slow down rendering into it).
uniform sampler2D indexImage;

layout( std430, binding = 0 ) restrict buffer BoxBuffer
{
  uint boxes[];
};

// x: image width, y: image height, z: number of indices (entries),
// w: strip length in pixels
uniform uvec4 params;

layout( local_size_x = @value( threads_per_group_x ),
        local_size_y = @value( threads_per_group_y ),
        local_size_z = @value( threads_per_group_z ) ) in;

uint decodeIndex( vec4 _c )
{
  uvec3 b = uvec3( round( _c.rgb * 255.0 ) );
  return b.r | ( b.g << 8u ) | ( b.b << 16u );
}

void addRun( uint _index, uint _x0, uint _x1, uint _y )
{
  if( _index == 0u || _index >= params.z )
    return;
  uint base = _index * 4u;
  atomicMax( boxes[base + 0u], 0xFFFFFFFFu - _x0 );
  atomicMax( boxes[base + 1u], 0xFFFFFFFFu - _y );
  atomicMax( boxes[base + 2u], _x1 );
  atomicMax( boxes[base + 3u], _y );
}

void main()
{
  uint y = gl_GlobalInvocationID.y;
  uint x0 = gl_GlobalInvocationID.x * params.w;
  if( y >= params.y || x0 >= params.x )
    return;
  uint x1 = min( x0 + params.w, params.x );

  uint current = decodeIndex( texelFetch( indexImage, ivec2( x0, y ), 0 ) );
  uint start = x0;
  for( uint x = x0 + 1u; x < x1; ++x )
  {
    uint index = decodeIndex( texelFetch( indexImage, ivec2( x, y ), 0 ) );
    if( index != current )
    {
      addRun( current, start, x - 1u, y );
      current = index;
      start = x;
    }
  }
  addRun( current, start, x1 - 1u, y );
}
