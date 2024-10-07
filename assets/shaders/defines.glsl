
#ifndef DEFINES_HEADER_
#define DEFINES_HEADER_

#define COL32_R_SHIFT    0
#define COL32_G_SHIFT    8
#define COL32_B_SHIFT    16
#define COL32_A_SHIFT    24
#define COL32_A_MASK     0xFF000000
#define COL32(R,G,B,A)    (((uint)(A)<<COL32_A_SHIFT) | ((uint)(B)<<COL32_B_SHIFT) | ((uint)(G)<<COL32_G_SHIFT) | ((uint)(R)<<COL32_R_SHIFT))
#define COL32_WHITE       COL32(255,255,255,255)  // Opaque white = 0xFFFFFFFF
#define COL32_BLACK       COL32(0,0,0,255)        // Opaque black
#define COL32_BLACK_TRANS COL32(0,0,0,0)          // Transparent black = 0x00000000

const float PI = 3.14159265359;

#define MAX_SHADOW_SUN_LIGHTS 2
#define MAX_SHADOW_SPOT_LIGHTS 8
#define MAX_SHADOW_POINT_LIGHTS 5
#define SHADOW_BIAS 0.0005f

#define MAX_BONE_PER_VERTEX 4
#define MAX_BONES_PER_MESH 256

#endif