#if 0
//
// Generated by Microsoft (R) HLSL Shader Compiler 9.27.952.3022
//
//   fxc LayerManagerD3D9Shaders.hlsl /ELayerQuadVS /nologo
//    /FhLayerManagerD3D9Shaders.h /VnLayerQuadVS
//
//
// Parameters:
//
//   float4x4 mLayerQuadTransform;
//   float4x4 mLayerTransform;
//   float4x4 mProjection;
//   float4 vRenderTargetOffset;
//
//
// Registers:
//
//   Name                Reg   Size
//   ------------------- ----- ----
//   mLayerQuadTransform c0       4
//   mLayerTransform     c4       4
//   mProjection         c8       4
//   vRenderTargetOffset c12      1
//

    vs_2_0
    dcl_position v0
    mul r0, v0.y, c1
    mad r0, c0, v0.x, r0
    mad r0, c2, v0.z, r0
    mad r0, c3, v0.w, r0
    mul r1, r0.y, c5
    mad r1, c4, r0.x, r1
    mad r1, c6, r0.z, r1
    mad r0, c7, r0.w, r1
    add r0, r0, -c12
    mul r1, r0.y, c9
    mad r1, c8, r0.x, r1
    mad r1, c10, r0.z, r1
    mad oPos, c11, r0.w, r1
    mov oT0.xy, v0

// approximately 14 instruction slots used
#endif

const BYTE LayerQuadVS[] =
{
      0,   2, 254, 255, 254, 255, 
     67,   0,  67,  84,  65,  66, 
     28,   0,   0,   0, 215,   0, 
      0,   0,   0,   2, 254, 255, 
      4,   0,   0,   0,  28,   0, 
      0,   0,   0,   1,   0,   0, 
    208,   0,   0,   0, 108,   0, 
      0,   0,   2,   0,   0,   0, 
      4,   0,   0,   0, 128,   0, 
      0,   0,   0,   0,   0,   0, 
    144,   0,   0,   0,   2,   0, 
      4,   0,   4,   0,   0,   0, 
    128,   0,   0,   0,   0,   0, 
      0,   0, 160,   0,   0,   0, 
      2,   0,   8,   0,   4,   0, 
      0,   0, 128,   0,   0,   0, 
      0,   0,   0,   0, 172,   0, 
      0,   0,   2,   0,  12,   0, 
      1,   0,   0,   0, 192,   0, 
      0,   0,   0,   0,   0,   0, 
    109,  76,  97, 121, 101, 114, 
     81, 117,  97, 100,  84, 114, 
     97, 110, 115, 102, 111, 114, 
    109,   0,   3,   0,   3,   0, 
      4,   0,   4,   0,   1,   0, 
      0,   0,   0,   0,   0,   0, 
    109,  76,  97, 121, 101, 114, 
     84, 114,  97, 110, 115, 102, 
    111, 114, 109,   0, 109,  80, 
    114, 111, 106, 101,  99, 116, 
    105, 111, 110,   0, 118,  82, 
    101, 110, 100, 101, 114,  84, 
     97, 114, 103, 101, 116,  79, 
    102, 102, 115, 101, 116,   0, 
      1,   0,   3,   0,   1,   0, 
      4,   0,   1,   0,   0,   0, 
      0,   0,   0,   0, 118, 115, 
     95,  50,  95,  48,   0,  77, 
    105,  99, 114, 111, 115, 111, 
    102, 116,  32,  40,  82,  41, 
     32,  72,  76,  83,  76,  32, 
     83, 104,  97, 100, 101, 114, 
     32,  67, 111, 109, 112, 105, 
    108, 101, 114,  32,  57,  46, 
     50,  55,  46,  57,  53,  50, 
     46,  51,  48,  50,  50,   0, 
     31,   0,   0,   2,   0,   0, 
      0, 128,   0,   0,  15, 144, 
      5,   0,   0,   3,   0,   0, 
     15, 128,   0,   0,  85, 144, 
      1,   0, 228, 160,   4,   0, 
      0,   4,   0,   0,  15, 128, 
      0,   0, 228, 160,   0,   0, 
      0, 144,   0,   0, 228, 128, 
      4,   0,   0,   4,   0,   0, 
     15, 128,   2,   0, 228, 160, 
      0,   0, 170, 144,   0,   0, 
    228, 128,   4,   0,   0,   4, 
      0,   0,  15, 128,   3,   0, 
    228, 160,   0,   0, 255, 144, 
      0,   0, 228, 128,   5,   0, 
      0,   3,   1,   0,  15, 128, 
      0,   0,  85, 128,   5,   0, 
    228, 160,   4,   0,   0,   4, 
      1,   0,  15, 128,   4,   0, 
    228, 160,   0,   0,   0, 128, 
      1,   0, 228, 128,   4,   0, 
      0,   4,   1,   0,  15, 128, 
      6,   0, 228, 160,   0,   0, 
    170, 128,   1,   0, 228, 128, 
      4,   0,   0,   4,   0,   0, 
     15, 128,   7,   0, 228, 160, 
      0,   0, 255, 128,   1,   0, 
    228, 128,   2,   0,   0,   3, 
      0,   0,  15, 128,   0,   0, 
    228, 128,  12,   0, 228, 161, 
      5,   0,   0,   3,   1,   0, 
     15, 128,   0,   0,  85, 128, 
      9,   0, 228, 160,   4,   0, 
      0,   4,   1,   0,  15, 128, 
      8,   0, 228, 160,   0,   0, 
      0, 128,   1,   0, 228, 128, 
      4,   0,   0,   4,   1,   0, 
     15, 128,  10,   0, 228, 160, 
      0,   0, 170, 128,   1,   0, 
    228, 128,   4,   0,   0,   4, 
      0,   0,  15, 192,  11,   0, 
    228, 160,   0,   0, 255, 128, 
      1,   0, 228, 128,   1,   0, 
      0,   2,   0,   0,   3, 224, 
      0,   0, 228, 144, 255, 255, 
      0,   0
};

#if 0
//
// Generated by Microsoft (R) HLSL Shader Compiler 9.27.952.3022
//
//   fxc LayerManagerD3D9Shaders.hlsl /ERGBShader /nologo /Tps_2_0
//    /FhLayerManagerD3D9Shaders.h /VnRGBShaderPS
//
//
// Parameters:
//
//   float fLayerOpacity;
//   sampler2D s2D;
//
//
// Registers:
//
//   Name          Reg   Size
//   ------------- ----- ----
//   fLayerOpacity c0       1
//   s2D           s0       1
//

    ps_2_0
    dcl t0.xy
    dcl_2d s0
    texld r0, t0, s0
    mul r0, r0, c0.x
    mov oC0, r0

// approximately 3 instruction slots used (1 texture, 2 arithmetic)
#endif

const BYTE RGBShaderPS[] =
{
      0,   2, 255, 255, 254, 255, 
     45,   0,  67,  84,  65,  66, 
     28,   0,   0,   0, 127,   0, 
      0,   0,   0,   2, 255, 255, 
      2,   0,   0,   0,  28,   0, 
      0,   0,   0,   1,   0,   0, 
    120,   0,   0,   0,  68,   0, 
      0,   0,   2,   0,   0,   0, 
      1,   0,   0,   0,  84,   0, 
      0,   0,   0,   0,   0,   0, 
    100,   0,   0,   0,   3,   0, 
      0,   0,   1,   0,   0,   0, 
    104,   0,   0,   0,   0,   0, 
      0,   0, 102,  76,  97, 121, 
    101, 114,  79, 112,  97,  99, 
    105, 116, 121,   0, 171, 171, 
      0,   0,   3,   0,   1,   0, 
      1,   0,   1,   0,   0,   0, 
      0,   0,   0,   0, 115,  50, 
     68,   0,   4,   0,  12,   0, 
      1,   0,   1,   0,   1,   0, 
      0,   0,   0,   0,   0,   0, 
    112, 115,  95,  50,  95,  48, 
      0,  77, 105,  99, 114, 111, 
    115, 111, 102, 116,  32,  40, 
     82,  41,  32,  72,  76,  83, 
     76,  32,  83, 104,  97, 100, 
    101, 114,  32,  67, 111, 109, 
    112, 105, 108, 101, 114,  32, 
     57,  46,  50,  55,  46,  57, 
     53,  50,  46,  51,  48,  50, 
     50,   0,  31,   0,   0,   2, 
      0,   0,   0, 128,   0,   0, 
      3, 176,  31,   0,   0,   2, 
      0,   0,   0, 144,   0,   8, 
     15, 160,  66,   0,   0,   3, 
      0,   0,  15, 128,   0,   0, 
    228, 176,   0,   8, 228, 160, 
      5,   0,   0,   3,   0,   0, 
     15, 128,   0,   0, 228, 128, 
      0,   0,   0, 160,   1,   0, 
      0,   2,   0,   8,  15, 128, 
      0,   0, 228, 128, 255, 255, 
      0,   0
};

#if 0
//
// Generated by Microsoft (R) HLSL Shader Compiler 9.27.952.3022
//
//   fxc LayerManagerD3D9Shaders.hlsl /EYCbCrShader /nologo /Tps_2_0
//    /FhLayerManagerD3D9Shaders.h /VnYCbCrShaderPS
//
//
// Parameters:
//
//   float fLayerOpacity;
//   sampler2D s2DCb;
//   sampler2D s2DCr;
//   sampler2D s2DY;
//
//
// Registers:
//
//   Name          Reg   Size
//   ------------- ----- ----
//   fLayerOpacity c0       1
//   s2DY          s0       1
//   s2DCb         s1       1
//   s2DCr         s2       1
//

    ps_2_0
    def c1, -0.5, -0.0625, 1.16400003, 1.59599996
    def c2, 0.813000023, 0.391000003, 2.01799989, 1
    dcl t0.xy
    dcl_2d s0
    dcl_2d s1
    dcl_2d s2
    texld r0, t0, s2
    texld r1, t0, s0
    texld r2, t0, s1
    add r0.x, r0.x, c1.x
    add r0.y, r1.x, c1.y
    mul r0.y, r0.y, c1.z
    mad r0.z, r0.x, -c2.x, r0.y
    mad r1.x, r0.x, c1.w, r0.y
    add r0.x, r2.x, c1.x
    mad r1.y, r0.x, -c2.y, r0.z
    mad r1.z, r0.x, c2.z, r0.y
    mov r1.w, c2.w
    mul r0, r1, c0.x
    mov oC0, r0

// approximately 14 instruction slots used (3 texture, 11 arithmetic)
#endif

const BYTE YCbCrShaderPS[] =
{
      0,   2, 255, 255, 254, 255, 
     68,   0,  67,  84,  65,  66, 
     28,   0,   0,   0, 219,   0, 
      0,   0,   0,   2, 255, 255, 
      4,   0,   0,   0,  28,   0, 
      0,   0,   0,   1,   0,   0, 
    212,   0,   0,   0, 108,   0, 
      0,   0,   2,   0,   0,   0, 
      1,   0,   0,   0, 124,   0, 
      0,   0,   0,   0,   0,   0, 
    140,   0,   0,   0,   3,   0, 
      1,   0,   1,   0,   0,   0, 
    148,   0,   0,   0,   0,   0, 
      0,   0, 164,   0,   0,   0, 
      3,   0,   2,   0,   1,   0, 
      0,   0, 172,   0,   0,   0, 
      0,   0,   0,   0, 188,   0, 
      0,   0,   3,   0,   0,   0, 
      1,   0,   0,   0, 196,   0, 
      0,   0,   0,   0,   0,   0, 
    102,  76,  97, 121, 101, 114, 
     79, 112,  97,  99, 105, 116, 
    121,   0, 171, 171,   0,   0, 
      3,   0,   1,   0,   1,   0, 
      1,   0,   0,   0,   0,   0, 
      0,   0, 115,  50,  68,  67, 
     98,   0, 171, 171,   4,   0, 
     12,   0,   1,   0,   1,   0, 
      1,   0,   0,   0,   0,   0, 
      0,   0, 115,  50,  68,  67, 
    114,   0, 171, 171,   4,   0, 
     12,   0,   1,   0,   1,   0, 
      1,   0,   0,   0,   0,   0, 
      0,   0, 115,  50,  68,  89, 
      0, 171, 171, 171,   4,   0, 
     12,   0,   1,   0,   1,   0, 
      1,   0,   0,   0,   0,   0, 
      0,   0, 112, 115,  95,  50, 
     95,  48,   0,  77, 105,  99, 
    114, 111, 115, 111, 102, 116, 
     32,  40,  82,  41,  32,  72, 
     76,  83,  76,  32,  83, 104, 
     97, 100, 101, 114,  32,  67, 
    111, 109, 112, 105, 108, 101, 
    114,  32,  57,  46,  50,  55, 
     46,  57,  53,  50,  46,  51, 
     48,  50,  50,   0,  81,   0, 
      0,   5,   1,   0,  15, 160, 
      0,   0,   0, 191,   0,   0, 
    128, 189, 244, 253, 148,  63, 
    186,  73, 204,  63,  81,   0, 
      0,   5,   2,   0,  15, 160, 
    197,  32,  80,  63,  39,  49, 
    200,  62, 233,  38,   1,  64, 
      0,   0, 128,  63,  31,   0, 
      0,   2,   0,   0,   0, 128, 
      0,   0,   3, 176,  31,   0, 
      0,   2,   0,   0,   0, 144, 
      0,   8,  15, 160,  31,   0, 
      0,   2,   0,   0,   0, 144, 
      1,   8,  15, 160,  31,   0, 
      0,   2,   0,   0,   0, 144, 
      2,   8,  15, 160,  66,   0, 
      0,   3,   0,   0,  15, 128, 
      0,   0, 228, 176,   2,   8, 
    228, 160,  66,   0,   0,   3, 
      1,   0,  15, 128,   0,   0, 
    228, 176,   0,   8, 228, 160, 
     66,   0,   0,   3,   2,   0, 
     15, 128,   0,   0, 228, 176, 
      1,   8, 228, 160,   2,   0, 
      0,   3,   0,   0,   1, 128, 
      0,   0,   0, 128,   1,   0, 
      0, 160,   2,   0,   0,   3, 
      0,   0,   2, 128,   1,   0, 
      0, 128,   1,   0,  85, 160, 
      5,   0,   0,   3,   0,   0, 
      2, 128,   0,   0,  85, 128, 
      1,   0, 170, 160,   4,   0, 
      0,   4,   0,   0,   4, 128, 
      0,   0,   0, 128,   2,   0, 
      0, 161,   0,   0,  85, 128, 
      4,   0,   0,   4,   1,   0, 
      1, 128,   0,   0,   0, 128, 
      1,   0, 255, 160,   0,   0, 
     85, 128,   2,   0,   0,   3, 
      0,   0,   1, 128,   2,   0, 
      0, 128,   1,   0,   0, 160, 
      4,   0,   0,   4,   1,   0, 
      2, 128,   0,   0,   0, 128, 
      2,   0,  85, 161,   0,   0, 
    170, 128,   4,   0,   0,   4, 
      1,   0,   4, 128,   0,   0, 
      0, 128,   2,   0, 170, 160, 
      0,   0,  85, 128,   1,   0, 
      0,   2,   1,   0,   8, 128, 
      2,   0, 255, 160,   5,   0, 
      0,   3,   0,   0,  15, 128, 
      1,   0, 228, 128,   0,   0, 
      0, 160,   1,   0,   0,   2, 
      0,   8,  15, 128,   0,   0, 
    228, 128, 255, 255,   0,   0
};

#if 0
//
// Generated by Microsoft (R) HLSL Shader Compiler 9.27.952.3022
//
//   fxc LayerManagerD3D9Shaders.hlsl /ESolidColorShader /nologo /Tps_2_0
//    /FhLayerManagerD3D9Shaders.h /VnSolidColorShaderPS
//
//
// Parameters:
//
//   float4 fLayerColor;
//
//
// Registers:
//
//   Name         Reg   Size
//   ------------ ----- ----
//   fLayerColor  c0       1
//

    ps_2_0
    mov oC0, c0

// approximately 1 instruction slot used
#endif

const BYTE SolidColorShaderPS[] =
{
      0,   2, 255, 255, 254, 255, 
     34,   0,  67,  84,  65,  66, 
     28,   0,   0,   0,  83,   0, 
      0,   0,   0,   2, 255, 255, 
      1,   0,   0,   0,  28,   0, 
      0,   0,   0,   1,   0,   0, 
     76,   0,   0,   0,  48,   0, 
      0,   0,   2,   0,   0,   0, 
      1,   0,   0,   0,  60,   0, 
      0,   0,   0,   0,   0,   0, 
    102,  76,  97, 121, 101, 114, 
     67, 111, 108, 111, 114,   0, 
      1,   0,   3,   0,   1,   0, 
      4,   0,   1,   0,   0,   0, 
      0,   0,   0,   0, 112, 115, 
     95,  50,  95,  48,   0,  77, 
    105,  99, 114, 111, 115, 111, 
    102, 116,  32,  40,  82,  41, 
     32,  72,  76,  83,  76,  32, 
     83, 104,  97, 100, 101, 114, 
     32,  67, 111, 109, 112, 105, 
    108, 101, 114,  32,  57,  46, 
     50,  55,  46,  57,  53,  50, 
     46,  51,  48,  50,  50,   0, 
      1,   0,   0,   2,   0,   8, 
     15, 128,   0,   0, 228, 160, 
    255, 255,   0,   0
};