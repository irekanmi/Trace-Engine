


#define SAMPLE_NORMAL_MAP(texture_map, tex_Coord, in_normal, in_tangent, out_normal)vec3 _n = texture(texture_map, tex_Coord).rgb;  _n = _n * 2.0f - 1.0f; vec3 obj_norm = normalize(in_normal); vec3 _tangent = normalize( in_tangent.xyz - (dot(in_tangent.xyz, obj_norm) * obj_norm) ); vec3 _bitangent = cross(obj_norm, _tangent) * in_tangent.w; mat3 TBN = mat3(_tangent, _bitangent, obj_norm); out_normal = normalize(TBN * _n);


/* Color palette */
    #define BLACK           vec3(0.0, 0.0, 0.0)
    #define WHITE           vec3(1.0, 1.0, 1.0)
    #define RED             vec3(1.0, 0.0, 0.0)
    #define GREEN           vec3(0.0, 1.0, 0.0)
    #define BLUE            vec3(0.0, 0.0, 1.0)
    #define YELLOW          vec3(1.0, 1.0, 0.0)
    #define CYAN            vec3(0.0, 1.0, 1.0)
    #define MAGENTA         vec3(1.0, 0.0, 1.0)
    #define ORANGE          vec3(1.0, 0.5, 0.0)
    #define PURPLE          vec3(1.0, 0.0, 0.5)
    #define LIME            vec3(0.5, 1.0, 0.0)
    #define ACQUA           vec3(0.0, 1.0, 0.5)
    #define VIOLET          vec3(0.5, 0.0, 1.0)
    #define AZUR            vec3(0.0, 0.5, 1.0)

vec4 SampleTextureMap(sampler2D texture_map, vec2 tex_Coord)
{
    return texture(texture_map, tex_Coord);
}

vec3 SampleTextureMap_RGB(sampler2D texture_map, vec2 tex_Coord)
{
    return texture(texture_map, tex_Coord).rgb;
}

vec2 SampleTextureMap_RG(sampler2D texture_map, vec2 tex_Coord)
{
    return texture(texture_map, tex_Coord).rg;
}

float SampleTextureMap_R(sampler2D texture_map, vec2 tex_Coord)
{
    return texture(texture_map, tex_Coord).r;
}