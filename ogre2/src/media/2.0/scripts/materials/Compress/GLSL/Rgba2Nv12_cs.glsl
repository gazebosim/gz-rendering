// RGBA -> NV12 compute shader (BT.709 limited range).
// Strategy: uint-per-byte buffer — the UAV buffer has w*h*3/2 uint elements,
// each holding one NV12 output byte in the low 8 bits. Indices are disjoint
// across invocations, so no atomics are needed.
//
// Dispatch: one invocation per 2x2 luma block.
// Grid = ceil(imgWidth/2) x ceil(imgHeight/2) x 1, auto-computed by ogre-next via thread_groups_based_on_texture divisor [2,2,1].

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

// Output: NV12 byte stream packed as one uint32 per byte (low 8 bits used).
// Buffer size = imgWidth * imgHeight * 3 / 2  uint elements.
@property( syntax != glslvk )
layout(std430, binding = 0) buffer Nv12Buffer
@else
layout(std430, ogre_U0) buffer Nv12Buffer
@end
{
    writeonly uint dstBytes[];
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

void main()
{
    uint w = imgWidth;
    uint h = imgHeight;

    // One invocation per 2x2 luma block.
    uint bx = gl_GlobalInvocationID.x;   // block x index
    uint by = gl_GlobalInvocationID.y;   // block y index

    // Guard against over-dispatch (in case dims are odd, though w/h must be even
    // for NV12).
    if (bx * 2u >= w || by * 2u >= h)
        return;

    uint x0 = bx * 2u;
    uint y0 = by * 2u;

    // Fetch the four texels of this 2x2 block.
    vec3 c00 = texelFetch(srcTex, ivec2(x0,      y0     ), 0).rgb;
    vec3 c10 = texelFetch(srcTex, ivec2(x0 + 1u, y0     ), 0).rgb;
    vec3 c01 = texelFetch(srcTex, ivec2(x0,      y0 + 1u), 0).rgb;
    vec3 c11 = texelFetch(srcTex, ivec2(x0 + 1u, y0 + 1u), 0).rgb;

    // --- Y plane (bytes [0, w*h)) ---
    // Each pixel maps to byte index: y * w + x
    dstBytes[y0 * w + x0]             = uint(clamp(rgb2y(c00), 0.0, 255.0));
    dstBytes[y0 * w + x0 + 1u]        = uint(clamp(rgb2y(c10), 0.0, 255.0));
    dstBytes[(y0 + 1u) * w + x0]      = uint(clamp(rgb2y(c01), 0.0, 255.0));
    dstBytes[(y0 + 1u) * w + x0 + 1u] = uint(clamp(rgb2y(c11), 0.0, 255.0));

    // --- Interleaved CbCr plane (bytes [w*h, w*h*3/2)) ---
    // Average chroma over the 2x2 block (NV12 4:2:0 subsampling).
    vec3 avg = (c00 + c10 + c01 + c11) * 0.25;
    uint cb = uint(clamp(rgb2cb(avg), 0.0, 255.0));
    uint cr = uint(clamp(rgb2cr(avg), 0.0, 255.0));

    // Each 2x2 block produces one (Cb, Cr) pair at:
    //   base = w*h + (by * (w/2) + bx) * 2
    uint cBase = w * h + (by * (w / 2u) + bx) * 2u;
    dstBytes[cBase]      = cb;
    dstBytes[cBase + 1u] = cr;
}
