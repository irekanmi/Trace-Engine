


#define SAMPLE_NORMAL_MAP(texture_map, tex_Coord, in_normal, in_tangent, out_normal)vec3 _n = texture(texture_map, tex_Coord).rgb;  _n = _n * 2.0f - 1.0f; vec3 obj_norm = normalize(in_normal); vec3 _tangent = normalize( in_tangent.xyz - (dot(in_tangent.xyz, obj_norm) * obj_norm) ); vec3 _bitangent = cross(obj_norm, _tangent) * in_tangent.w; mat3 TBN = mat3(_tangent, _bitangent, obj_norm); out_normal = normalize(TBN * _n);

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