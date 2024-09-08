
#ifndef FUNCTIONS_HEADER_
#define FUNCTIONS_HEADER_

#include "defines.glsl"

vec4 prefilter_color(vec4 color, float falloff)
{
    float brightness = max(max(color.r, color.g), color.b);
    float numerator = max(brightness - falloff, 0.0001f);
    float denominator = max(brightness, 0.0001f);
    float result = numerator / denominator;
    return color * result;
}

vec4 sampleTexture13Tap(sampler2D tex, vec2 inUV)
{
    //a - b - c
    //- d - e -
    //f - g - h
    //- i - j -
    //k - l - m
    // g => being the current texel
    vec4 result;
    ivec2 tex_size = textureSize(tex, 0);
    float texel_size_X = tex_size.x;
    float texel_size_Y = tex_size.y;
    texel_size_X = 1.0f / texel_size_X;
    texel_size_Y = 1.0f / texel_size_Y;

    vec4 a = texture(tex, inUV + vec2(-2 * texel_size_X, 2 * texel_size_Y)); 
    vec4 b = texture(tex, inUV + vec2(texel_size_X, 2 * texel_size_Y)); 
    vec4 c = texture(tex, inUV + vec2(2 * texel_size_X, 2 * texel_size_Y)); 
    vec4 d = texture(tex, inUV + vec2(-1 * texel_size_X, 1 * texel_size_Y)); 
    vec4 e = texture(tex, inUV + vec2(1 * texel_size_X, 1 * texel_size_Y)); 
    vec4 f = texture(tex, inUV + vec2(-1 * texel_size_X, 0.0f)); 
    vec4 g = texture(tex, inUV + vec2(0.0f, 0.0f));
    vec4 h = texture(tex, inUV + vec2(1 * texel_size_X, 0.0f));
    vec4 i = texture(tex, inUV + vec2(-1 * texel_size_X, -1 * texel_size_Y));
    vec4 j = texture(tex, inUV + vec2(1 * texel_size_X, -1 * texel_size_Y));
    vec4 k = texture(tex, inUV + vec2(-2 * texel_size_X, -2 * texel_size_Y));
    vec4 l = texture(tex, inUV + vec2(0.0f, -2 * texel_size_Y));
    vec4 m = texture(tex, inUV + vec2(2 * texel_size_X, -2 * texel_size_Y));
    
    // Weight to be appiled to ensure energy preservation
    // 0.5 + 0.125 + 0.125 + 0.125 + 0.125 = 1;
    result = g * 0.125f;
    result += (a + c + k + m) * 0.03125f;
    result += (b + f + h + l) * 0.0625f;
    result += (d + e + i + j) * 0.125f;

    return result;
}

vec4 sampleBox_4x4(sampler2D tex, vec2 inUV, float radius)
{
    //a - b
    //c - d

    vec4 result;
    ivec2 tex_size = textureSize(tex, 0);
    float texel_size_X = tex_size.x;
    float texel_size_Y = tex_size.y;
    texel_size_X = 1.0f / texel_size_X;
    texel_size_Y = 1.0f / texel_size_Y;


    vec4 a = texture(tex, inUV + vec2(-1 * texel_size_X, 1 * texel_size_Y));
    vec4 b = texture(tex, inUV + vec2(1 * texel_size_X, 1 * texel_size_Y));
    vec4 c = texture(tex, inUV + vec2(-1 * texel_size_X, -1 * texel_size_Y));
    vec4 d = texture(tex, inUV + vec2(1 * texel_size_X, -1 * texel_size_Y));

    result = a + b + c + d;
    result *= 0.25f;
    return result;
}

vec4 sampleTent_3x3(sampler2D tex, vec2 inUV, float radius)
{
    // Apply a 3x3 tent filter to upsample using
    // 1| 1 2 1 |
    // -| 2 4 2 |
    //16| 1 2 1 |

    // a - b - c
    // d - e - f
    // g - h - i
    //  e => begin the current texel

    ivec2 tex_size = textureSize(tex, 0);
    float texel_size_X = tex_size.x;
    float texel_size_Y = tex_size.y;
    texel_size_X = 1.0f / texel_size_X;
    texel_size_Y = 1.0f / texel_size_Y;
    texel_size_X *= radius;
    texel_size_Y *= radius;

    vec4 a = texture(tex, inUV + vec2(-1 * texel_size_X, 1 * texel_size_Y));
    vec4 b = texture(tex, inUV + vec2(0.0f, 1 * texel_size_Y));
    vec4 c = texture(tex, inUV + vec2(1 * texel_size_X, 1 * texel_size_Y));
    vec4 d = texture(tex, inUV + vec2(-1 * texel_size_X, 0.0f));
    vec4 e = texture(tex, inUV + vec2(0.0f, 0.0f));
    vec4 f = texture(tex, inUV + vec2(1 * texel_size_X, 0.0f));
    vec4 g = texture(tex, inUV + vec2(-1 * texel_size_X, -1 * texel_size_Y));
    vec4 h = texture(tex, inUV + vec2(0.0f, -1 * texel_size_Y));
    vec4 i = texture(tex, inUV + vec2(1 * texel_size_X, -1 * texel_size_Y));

    vec4 result;
    result = e * 4;
    result += (b + d + f + h) * 2;
    result += (a + c + g + i);
    result *= 1.0 / 16.0;

    return result;
}

vec4 colorFromUint32(uint color)
{
    vec4 result;

    float a = float(color >> 24) / 255.0f;
    float b = float( (color >> 16) & 0x000000FF) / 255.0f;
    float g = float( (color >> 8) & 0x000000FF) / 255.0f;
    float r = float( (color >> 0) & 0x000000FF) / 255.0f;

    result.r = r;
    result.g = g;
    result.b = b;
    result.a = a;

    return result;
}

uint vec4ToUint32(vec4 value)
{
    uint result = 0;
    
    int a = int(value.a * 255.0f);
    int b = int(value.b * 255.0f);
    int g = int(value.g * 255.0f);
    int r = int(value.r * 255.0f);

    result |= (a << 24);
    result |= (b << 16);
    result |= (g << 8);
    result |= (r << 0);

    return result;
}

float map(float value, float in_min, float in_max, float out_min, float out_max)
{
    return out_min + ( (out_max - out_min) * ( (value - in_min) / (in_max - in_min) ));
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float DistributionGGX(float NdotH, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}

float GeometrySmith(float NdotV, float NdotL, float roughness)
{
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

vec3 CookTorrance_BRDF(vec3 normal, vec3 light_direction, vec3 view_direction, vec3 radiance, vec3 albedo, float metallic, float roughness)
{
    vec3 final_color = vec3(0.0f);

    vec3 half_direction = normalize(light_direction + view_direction);

    float NdotV = max(dot(normal, view_direction), 0.0f);
    float NdotL = max(dot(normal, light_direction), 0.0f);
    float NdotH = max(dot(normal, half_direction), 0.0f);

    float cosTheta = max(dot(half_direction, view_direction), 0.0f);

    vec3 F0 = vec3(0.04f);
    F0 = mix(F0, albedo, metallic);

    vec3 Fresnel = fresnelSchlick(cosTheta, F0);
    float NDF = DistributionGGX(NdotH, roughness);
    float G = GeometrySmith(NdotV, NdotL, roughness);

    vec3 numerator    = NDF * G * Fresnel;
    float denominator = (4.0 * NdotV * NdotL)  + 0.0001;
    vec3 specular     = numerator / denominator; 

    vec3 kS = Fresnel;
    vec3 kD = vec3(1.0f) - kS;

    vec3 diffuse = kD * (albedo / PI);

    final_color = ( diffuse + specular ) * (radiance * NdotL);

    return final_color;
}

vec2 ParallaxMapping(sampler2D height_map, float height_scale ,vec2 tex_coords, vec3 tangent_view_dir)
{
    vec2 result;

    float min_layers = 8.0f;
    float max_layers = 24.0f;
    float num_layers = mix(max_layers, min_layers, max(dot(vec3(0.0f, 0.0f, 1.0f), tangent_view_dir), 0.0f) );

    float layer_depth = 1.0f / num_layers;

    vec2 P = (tangent_view_dir.xy /*/ tangent_view_dir.z*/) * height_scale;
    vec2 delta_tex_coords = ( P / num_layers);

    float current_layer_depth = 0.0f;
    vec2 current_tex_coords = tex_coords;
    float current_depth_value = (1.0f - texture(height_map, current_tex_coords).r);

    while(current_layer_depth < current_depth_value)
    {
        current_tex_coords -= delta_tex_coords;
        current_depth_value = (1.0f - texture(height_map, current_tex_coords).r);
        current_layer_depth += layer_depth;
    }

    vec2 previous_tex_coord = current_tex_coords + delta_tex_coords;

    // get depth after and before collision for linear interpolation
    float after_depth  = current_depth_value - current_layer_depth;
    float before_depth = (1.0f - texture(height_map, previous_tex_coord).r) - current_layer_depth + layer_depth;

    // interpolation of texture coordinates
    float weight = after_depth / (after_depth - before_depth);
    result = previous_tex_coord * weight + current_tex_coords * (1.0 - weight);

    return result;
}

vec2 SimpleParallaxMapping(sampler2D height_map, float height_scale ,vec2 tex_coords, vec3 tangent_view_dir)
{
    float height =  1.0f - texture(height_map, tex_coords).r;    
    vec2 p = tangent_view_dir.xy * (height * height_scale);
    return tex_coords - p;
}

float ShadowPCF(sampler2D shadow_map, vec2 tex_coords, int num_samples, float frag_depth, float bias)
{
    vec2 map_size = 1.0f / textureSize(shadow_map, 0);
    float total_samples_depth = 0.0f;
    float samples_count = 0.0f;

    float pixel_depth = clamp(frag_depth, 0.0f, 1.0f);
    for(int x = -num_samples; x <= num_samples; x++)
    {
        for(int y = -num_samples; y <= num_samples; y++)
        {
            vec2 offset = vec2(x, y) * map_size;
            vec2 actual_tex_coord = tex_coords + offset;
            float depth = texture(shadow_map, actual_tex_coord).r;
            if((depth + bias)< pixel_depth)
            {
                total_samples_depth += 1.0f;
            }
            samples_count++;
        }
    }

    return 1.0f - ( total_samples_depth / samples_count /*pow(num_samples * 2 + 1, 2)*/ );
}


#endif