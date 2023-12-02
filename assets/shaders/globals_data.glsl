

#define IN_POSITION(index) layout(location = index)in vec3 in_pos;
#define IN_NORMAL(index) layout(location = index)in vec3 in_normal;
#define IN_TEX_COORD(index) layout(location = index)in vec2 in_texCoord;
#define IN_TANGENT(index) layout(location = index)in vec4 in_tangent;

#define DEFAULT_VERTEX_INPUT IN_POSITION(0) IN_NORMAL(1) IN_TEX_COORD(2) IN_TANGENT(3)

#ifdef DEFAULT_VERTEX_INPUT

#define VERT_POSITION in_pos;
#define VERT_NORMAL in_normal;
#define VERT_TEX_COORD in_texCoord;
#define VERT_TANGENT in_tangent;

#endif

#define OUT_VERTEX_DATA layout(location = 0)out data_object { vec3 _normal_; vec3 _fragPos; vec2 _texCoord; vec4 _tangent_; };
#define IN_VERTEX_DATA layout(location = 0)in data_object { vec3 _normal_; vec3 _fragPos; vec2 _texCoord; vec4 _tangent_; };



#define OUT_FRAG_DATA layout(location = 0)out vec4 g_Position; layout(location = 1)out vec4 g_Normal; layout(location = 2)out vec4 g_ColorSpecular;
#define FRAG_POS g_Position.xyz
#define FRAG_NORMAL g_Normal.xyz
#define FRAG_SPECULAR g_ColorSpecular.a
#define FRAG_COLOR g_ColorSpecular.rgb
#define FRAG_SHININESS g_Normal.a

