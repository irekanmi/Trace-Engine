

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

#define OUT_VERTEX_DATA layout(location = 0)out data_object { vec3 _normal_; vec3 _fragPos; vec2 _texCoord; vec4 _tangent_; vec3 _view_position_; vec3 world_position; };
#define IN_VERTEX_DATA layout(location = 0)in data_object { vec3 _normal_; vec3 _fragPos; vec2 _texCoord; vec4 _tangent_; vec3 _view_position_; vec3 world_position; };



#define OUT_FRAG_DATA layout(location = 0)out vec4 g_Position; layout(location = 1)out vec4 g_Normal; layout(location = 2)out uvec4 g_ColorSpecular;
#define FRAG_POS g_Position.xyz
#define FRAG_POS_X g_Position.x
#define FRAG_POS_Y g_Position.y
#define FRAG_POS_Z g_Position.z
#define FRAG_POS_W g_Position.w
#define FRAG_NORMAL g_Normal.xyz
#define FRAG_NORMAL_X g_Normal.x
#define FRAG_NORMAL_Y g_Normal.y
#define FRAG_NORMAL_Z g_Normal.z
#define FRAG_NORMAL_W g_Normal.w
#define FRAG_COLOR g_ColorSpecular.rgb
#define FRAG_COLOR_R g_ColorSpecular.r
#define FRAG_COLOR_G g_ColorSpecular.g
#define FRAG_COLOR_B g_ColorSpecular.b
#define FRAG_COLOR_A g_ColorSpecular.a
#define FRAG_SPECULAR g_ColorSpecular.a
#define FRAG_SHININESS g_Normal.a

