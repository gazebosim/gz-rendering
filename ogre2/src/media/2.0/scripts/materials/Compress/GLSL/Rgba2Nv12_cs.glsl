// RGBA -> NV12 compute shader (BT.709 limited range).
// Strategy: PACKED-byte buffer — the UAV holds w*h*3/8 uint elements, each
// holding 4 NV12 output bytes (little-endian: lowest NV12 address in the low 8
// bits). The host reads it back with a plain memcpy (no per-byte compaction).
//
// Race-freedom: one invocation owns a 4x2 source tile and writes exactly three
// disjoint uint words (two Y rows + one interleaved CbCr word), so no two
// invocations touch the same uint. This requires imgWidth % 4 == 0 and
// imgHeight even (enforced by Ogre2GpuCompression::Configure).
//
// Dispatch: one invocation per 4x2 luma tile.
// Grid = ceil(imgWidth/4) x ceil(imgHeight/2) x 1, auto-computed by ogre-next
// via thread_groups_based_on_texture divisor [4,2,1].

@property( syntax != glslvk )
    #version 430
@else
    #version 450
@end

// Input: the rendered colour texture (RGBA8). We read RGB values as they come
// (treatment of sRGB-vs-linear is a tuning detail arbitrated by Task 6).
@property( syntax != glslvk )
uniform sampler2D srcTex;
@else
layout( vulkan( ogre_t0 ) ) uniform texture2D srcTex;
@end

// Output: NV12 byte stream packed 4 bytes per uint32 (little-endian).
// Buffer size = imgWidth * imgHeight * 3 / 8  uint elements.
@property( syntax != glslvk )
layout(std430, binding = 0) buffer Nv12Buffer
@else
layout(std430, ogre_U0) buffer Nv12Buffer
@end
{
    writeonly uint dstWords[];
};

vulkan( layout( ogre_P0 ) uniform Params { )
    uniform uint imgWidth;
    uniform uint imgHeight;
vulkan( }; )

layout( local_size_x = @value( threads_per_group_x ),
        local_size_y = @value( threads_per_group_y ),
        local_size_z = 1 ) in;

// BT.709 limited range. Input RGB assumed to be in [0,1].
float rgb2y(vec3 c)
{
    return 16.0 + (0.1826 * c.r + 0.6142 * c.g + 0.0620 * c.b) * 255.0;
}
float rgb2cb(vec3 c)
{
    return 128.0 + (-0.1006 * c.r - 0.3386 * c.g + 0.4392 * c.b) * 255.0;
}
float rgb2cr(vec3 c)
{
    return 128.0 + (0.4392 * c.r - 0.3989 * c.g - 0.0403 * c.b) * 255.0;
}

// Luma byte for a pixel (clamp+truncate identical to the per-byte shader).
uint y8(vec3 c)  { return uint(clamp(rgb2y(c), 0.0, 255.0)); }
uint c8(float v) { return uint(clamp(v,       0.0, 255.0)); }

void main()
{
    uint w = imgWidth;
    uint h = imgHeight;

    // One invocation per 4x2 tile = two horizontally-adjacent 2x2 chroma blocks.
    uint tileX = gl_GlobalInvocationID.x;
    uint tileY = gl_GlobalInvocationID.y;

    uint x0 = tileX * 4u;
    uint y0 = tileY * 2u;
    if (x0 >= w || y0 >= h)
        return;

    // Fetch the 4x2 texels (cols x0..x0+3, rows y0..y0+1).
    vec3 p00 = texelFetch(srcTex, ivec2(x0,      y0     ), 0).rgb;
    vec3 p10 = texelFetch(srcTex, ivec2(x0 + 1u, y0     ), 0).rgb;
    vec3 p20 = texelFetch(srcTex, ivec2(x0 + 2u, y0     ), 0).rgb;
    vec3 p30 = texelFetch(srcTex, ivec2(x0 + 3u, y0     ), 0).rgb;
    vec3 p01 = texelFetch(srcTex, ivec2(x0,      y0 + 1u), 0).rgb;
    vec3 p11 = texelFetch(srcTex, ivec2(x0 + 1u, y0 + 1u), 0).rgb;
    vec3 p21 = texelFetch(srcTex, ivec2(x0 + 2u, y0 + 1u), 0).rgb;
    vec3 p31 = texelFetch(srcTex, ivec2(x0 + 3u, y0 + 1u), 0).rgb;

    uint wq = w / 4u;  // uint words per Y-plane row

    // --- Y plane: two packed words, one per row (bytes y*w + x0 .. +3) ---
    dstWords[y0 * wq + tileX] =
        y8(p00) | (y8(p10) << 8) | (y8(p20) << 16) | (y8(p30) << 24);
    dstWords[(y0 + 1u) * wq + tileX] =
        y8(p01) | (y8(p11) << 8) | (y8(p21) << 16) | (y8(p31) << 24);

    // --- Interleaved CbCr plane: one packed word (Cb_l Cr_l Cb_r Cr_r) ---
    // 4:2:0 subsampling: average chroma over each 2x2 block.
    vec3 avgL = (p00 + p10 + p01 + p11) * 0.25;
    vec3 avgR = (p20 + p30 + p21 + p31) * 0.25;
    uint uvWord = (w * h) / 4u + tileY * wq + tileX;
    dstWords[uvWord] =
        c8(rgb2cb(avgL)) | (c8(rgb2cr(avgL)) << 8) |
        (c8(rgb2cb(avgR)) << 16) | (c8(rgb2cr(avgR)) << 24);
}
