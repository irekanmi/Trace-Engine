

#define OUT_OIT_DATA layout (location = 0) out vec4 accum; layout (location = 1) out float reveal;

#define PROCESS_COLOR(color) float weight = clamp(pow(min(1.0, color.a * 10.0) + 0.01, 3.0) * 1e8 *  pow(1.0 - gl_FragCoord.z * 0.9, 3.0), 1e-2, 3e3); accum = vec4(color.rgb * color.a, color.a) * weight;    reveal = color.a;
