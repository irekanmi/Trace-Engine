#include "pch.h"

#include "shader_graph/ShaderGraphNode.h"
#include "shader_graph/ShaderGraph.h"
#include "spdlog/fmt/fmt.h"
#include "core/Application.h"



namespace trace {

#define GET_NODE_OUTPUT(node, out_result, input_index, shader_graph_instance) if (node->GetInputs()[input_index].node_id != 0)                     \
	{                                                                                                                                              \
		ShaderGraphNode* in_node = (ShaderGraphNode*)shader_graph_instance->GetShaderGraph()->GetNode(node->GetInputs()[input_index].node_id);     \
		in_node->Run(shader_graph_instance);																									   \
		out_result = *(std::string*)in_node->GetValueInternal(instance, node->GetInputs()[input_index].value_index);													   \
	}

	static std::unordered_map<ShaderNodeType, ShaderNodeMetaData> node_meta_data =
	{
		{
			ShaderNodeType::Float_Constant,
			{
				{},
				{{GenericValueType::Float, 0}},
				[](std::string* out_vars, GenericNode* node, GenericGraphInstance* graph_instance) -> std::string
				{
					ShaderGraphInstance* instance = (ShaderGraphInstance*)graph_instance;
					ConstantNode* _node = (ConstantNode*)node;
					std::string out_code = "";
					GenericParameterData& param = _node->GetDefaultParameterData()[0];
					std::string in_0 = GenericHelper::GetParameterValueString(param, GenericValueType::Float);
					
					out_vars[0] = "var_" + std::to_string(instance->GetNextVarIndex());
					out_code = "float {} = {};";

					out_code = fmt::format(out_code, out_vars[0], in_0);
					

					return out_code;
				}
			}
		},
		{
			ShaderNodeType::Int_Constant,
			{
				{},
				{{GenericValueType::Int, 0}},
				[](std::string* out_vars, GenericNode* node, GenericGraphInstance* graph_instance) -> std::string
				{
					ShaderGraphInstance* instance = (ShaderGraphInstance*)graph_instance;
					ConstantNode* _node = (ConstantNode*)node;
					std::string out_code = "";
					GenericParameterData& param = _node->GetDefaultParameterData()[0];
					std::string in_0 = GenericHelper::GetParameterValueString(param, GenericValueType::Int);

					out_vars[0] = "var_" + std::to_string(instance->GetNextVarIndex());
					out_code = "int {} = {};";

					out_code = fmt::format(out_code, out_vars[0], in_0);


					return out_code;
				}
			}
		},
		{
			ShaderNodeType::Vec2_Constant,
			{
				{GenericNodeInput{GenericValueType::Float, 0, 0}, GenericNodeInput{GenericValueType::Float, 0, 0} },
				{{GenericValueType::Vec2, 0}},
				[](std::string* out_vars, GenericNode* node, GenericGraphInstance* graph_instance) -> std::string
				{
					ShaderGraphInstance* instance = (ShaderGraphInstance*)graph_instance;
					ConstantNode* _node = (ConstantNode*)node;
					std::string out_code = "";
					GenericParameterData& param0 = _node->GetDefaultParameterData()[0];
					std::string in_0 = GenericHelper::GetParameterValueString(param0, GenericValueType::Float);
					GenericParameterData& param1 = _node->GetDefaultParameterData()[1];
					std::string in_1 = GenericHelper::GetParameterValueString(param1, GenericValueType::Float);

					
					GET_NODE_OUTPUT(_node, in_0, 0, instance);
					GET_NODE_OUTPUT(_node, in_1, 1, instance);
					

					out_vars[0] = "var_" + std::to_string(instance->GetNextVarIndex());
					out_code = "vec2 {} = vec2({}, {});";

					out_code = fmt::format(out_code, out_vars[0], in_0, in_1);


					return out_code;
				}
			}
		},
		{
			ShaderNodeType::Vec3_Constant,
			{
				{GenericNodeInput{GenericValueType::Float, 0, 0}, GenericNodeInput{GenericValueType::Float, 0, 0}, GenericNodeInput{GenericValueType::Float, 0, 0} },
				{{GenericValueType::Vec3, 0}},
				[](std::string* out_vars, GenericNode* node, GenericGraphInstance* graph_instance) -> std::string
				{
					ShaderGraphInstance* instance = (ShaderGraphInstance*)graph_instance;
					ConstantNode* _node = (ConstantNode*)node;
					std::string out_code = "";
					GenericParameterData& param0 = _node->GetDefaultParameterData()[0];
					std::string in_0 = GenericHelper::GetParameterValueString(param0, GenericValueType::Float);
					GenericParameterData& param1 = _node->GetDefaultParameterData()[1];
					std::string in_1 = GenericHelper::GetParameterValueString(param1, GenericValueType::Float);
					GenericParameterData& param2 = _node->GetDefaultParameterData()[2];
					std::string in_2 = GenericHelper::GetParameterValueString(param2, GenericValueType::Float);

					GET_NODE_OUTPUT(_node, in_0, 0, instance);
					GET_NODE_OUTPUT(_node, in_1, 1, instance);
					GET_NODE_OUTPUT(_node, in_2, 2, instance);

					out_vars[0] = "var_" + std::to_string(instance->GetNextVarIndex());
					out_code = "vec3 {} = vec3({}, {}, {});";

					out_code = fmt::format(out_code, out_vars[0], in_0, in_1, in_2);


					return out_code;
				}
			}
		},
		{
			ShaderNodeType::Vec4_Constant,
			{
				{GenericNodeInput{GenericValueType::Float, 0, 0}, GenericNodeInput{GenericValueType::Float, 0, 0}, GenericNodeInput{GenericValueType::Float, 0, 0}, GenericNodeInput{GenericValueType::Float, 0, 0} },
				{{GenericValueType::Vec4, 0}},
				[](std::string* out_vars, GenericNode* node, GenericGraphInstance* graph_instance) -> std::string
				{
					ShaderGraphInstance* instance = (ShaderGraphInstance*)graph_instance;
					ConstantNode* _node = (ConstantNode*)node;
					std::string out_code = "";
					GenericParameterData& param0 = _node->GetDefaultParameterData()[0];
					std::string in_0 = GenericHelper::GetParameterValueString(param0, GenericValueType::Float);
					GenericParameterData& param1 = _node->GetDefaultParameterData()[1];
					std::string in_1 = GenericHelper::GetParameterValueString(param1, GenericValueType::Float);
					GenericParameterData& param2 = _node->GetDefaultParameterData()[2];
					std::string in_2 = GenericHelper::GetParameterValueString(param2, GenericValueType::Float);
					GenericParameterData& param3 = _node->GetDefaultParameterData()[3];
					std::string in_3 = GenericHelper::GetParameterValueString(param3, GenericValueType::Float);

					GET_NODE_OUTPUT(_node, in_0, 0, instance);
					GET_NODE_OUTPUT(_node, in_1, 1, instance);
					GET_NODE_OUTPUT(_node, in_2, 2, instance);
					GET_NODE_OUTPUT(_node, in_3, 3, instance);

					out_vars[0] = "var_" + std::to_string(instance->GetNextVarIndex());
					out_code = "vec4 {} = vec4({}, {}, {}, {});";

					out_code = fmt::format(out_code, out_vars[0], in_0, in_1, in_2, in_3);


					return out_code;
				}
			}
		},
		{
			ShaderNodeType::Sample_Texture_Function,
			{
				{GenericNodeInput{GenericValueType::Sampler2D, 0, 0}, GenericNodeInput{GenericValueType::Vec2, 0, 0}},
				{{GenericValueType::Vec4, 0}, {GenericValueType::Vec3, 1}, {GenericValueType::Float, 2}, {GenericValueType::Float, 3}, {GenericValueType::Float, 4}, {GenericValueType::Float, 5}},
				[](std::string* out_vars, GenericNode* node, GenericGraphInstance* graph_instance) -> std::string
				{
					ShaderGraphInstance* instance = (ShaderGraphInstance*)graph_instance;
					ConstantNode* _node = (ConstantNode*)node;
					std::string out_code = "";
					GenericParameterData& param0 = _node->GetDefaultParameterData()[0];
					std::string in_0 = GenericHelper::GetParameterValueString(param0, GenericValueType::Float);
					GenericParameterData& param1 = _node->GetDefaultParameterData()[1];
					std::string in_1 = GenericHelper::GetParameterValueString(param1, GenericValueType::Vec2);
					

					GET_NODE_OUTPUT(_node, in_0, 0, instance);
					GET_NODE_OUTPUT(_node, in_1, 1, instance);

					out_vars[0] = "var_" + std::to_string(instance->GetNextVarIndex());
					out_vars[1] = out_vars[0] + ".rgb";
					out_vars[2] = out_vars[0] + ".r";
					out_vars[3] = out_vars[0] + ".g";
					out_vars[4] = out_vars[0] + ".b";
					out_vars[5] = out_vars[0] + ".a";
					out_code = R"(vec4 {} = texture({}, {});)";

					out_code = fmt::format(out_code, out_vars[0], in_0, in_1);


					return out_code;
				}
			}
		},
		{
			ShaderNodeType::Normal_Texture_To_World_Function,
			{
				{GenericNodeInput{GenericValueType::Vec3, 0, 0}, GenericNodeInput{GenericValueType::Vec4, 0, 0}, GenericNodeInput{GenericValueType::Vec3, 0, 0}},
				{{GenericValueType::Vec3, 0}},
				[](std::string* out_vars, GenericNode* node, GenericGraphInstance* graph_instance) -> std::string
				{
					ShaderGraphInstance* instance = (ShaderGraphInstance*)graph_instance;
					ConstantNode* _node = (ConstantNode*)node;
					std::string out_code = "";
					GenericParameterData& param0 = _node->GetDefaultParameterData()[0];
					std::string in_0 = "_normal_";
					GenericParameterData& param1 = _node->GetDefaultParameterData()[1];
					std::string in_1 = "_tangent_";
					GenericParameterData& param2 = _node->GetDefaultParameterData()[2];
					std::string in_2 = "vec3(0.0f, 0.0f, 1.0f)";
					

					GET_NODE_OUTPUT(_node, in_0, 0, instance);
					GET_NODE_OUTPUT(_node, in_1, 1, instance);
					GET_NODE_OUTPUT(_node, in_2, 2, instance);

					out_vars[0] = "var_" + std::to_string(instance->GetNextVarIndex());
					out_code = R"(vec3 {} = normal_texuture_to_world_space({}, {}, {} );)";

					out_code = fmt::format(out_code, out_vars[0], in_0, in_1, in_2);


					return out_code;
				}
			}
		},
		{
			ShaderNodeType::Frensel_Function,
			{
				{GenericNodeInput{GenericValueType::Vec3, 0, 0}, GenericNodeInput{GenericValueType::Vec3, 0, 0}, GenericNodeInput{GenericValueType::Float, 0, 0}},
				{{GenericValueType::Float, 0}},
				[](std::string* out_vars, GenericNode* node, GenericGraphInstance* graph_instance) -> std::string
				{
					ShaderGraphInstance* instance = (ShaderGraphInstance*)graph_instance;
					ConstantNode* _node = (ConstantNode*)node;
					std::string out_code = "";
					GenericParameterData& param0 = _node->GetDefaultParameterData()[0];
					std::string in_0 = "vec3(0.0f, 0.0f, 1.0f)";
					GenericParameterData& param1 = _node->GetDefaultParameterData()[1];
					std::string in_1 = "vec3(0.0f, 0.0f, -1.0f)";
					GenericParameterData& param2 = _node->GetDefaultParameterData()[2];
					std::string in_2 = GenericHelper::GetParameterValueString(param2, GenericValueType::Float);
					

					GET_NODE_OUTPUT(_node, in_0, 0, instance);
					GET_NODE_OUTPUT(_node, in_1, 1, instance);
					GET_NODE_OUTPUT(_node, in_2, 2, instance);

					out_vars[0] = "var_" + std::to_string(instance->GetNextVarIndex());
					out_code = R"(float {} = frenselEffect({}, {}, {} );)";

					out_code = fmt::format(out_code, out_vars[0], in_0, in_1, in_2);


					return out_code;
				}
			}
		},
		{
			ShaderNodeType::Multiply_Vec4_Function,
			{
				{GenericNodeInput{GenericValueType::Vec4, 0, 0}, GenericNodeInput{GenericValueType::Vec4, 0, 0}},
				{{GenericValueType::Vec4, 0}},
				[](std::string* out_vars, GenericNode* node, GenericGraphInstance* graph_instance) -> std::string
				{
					ShaderGraphInstance* instance = (ShaderGraphInstance*)graph_instance;
					ConstantNode* _node = (ConstantNode*)node;
					std::string out_code = "";
					GenericParameterData& param0 = _node->GetDefaultParameterData()[0];
					std::string in_0 = GenericHelper::GetParameterValueString(param0, GenericValueType::Vec4);
					GenericParameterData& param1 = _node->GetDefaultParameterData()[1];
					std::string in_1 = GenericHelper::GetParameterValueString(param1, GenericValueType::Vec4);
					

					GET_NODE_OUTPUT(_node, in_0, 0, instance);
					GET_NODE_OUTPUT(_node, in_1, 1, instance);

					out_vars[0] = "var_" + std::to_string(instance->GetNextVarIndex());
					out_code = R"(vec4 {} = {} * {};)";

					out_code = fmt::format(out_code, out_vars[0], in_0, in_1);


					return out_code;
				}
			}
		},
		{
			ShaderNodeType::Multiply_Vec4_Float_Function,
			{
				{GenericNodeInput{GenericValueType::Vec4, 0, 0}, GenericNodeInput{GenericValueType::Float, 0, 0}},
				{{GenericValueType::Vec4, 0}},
				[](std::string* out_vars, GenericNode* node, GenericGraphInstance* graph_instance) -> std::string
				{
					ShaderGraphInstance* instance = (ShaderGraphInstance*)graph_instance;
					ConstantNode* _node = (ConstantNode*)node;
					std::string out_code = "";
					GenericParameterData& param0 = _node->GetDefaultParameterData()[0];
					std::string in_0 = GenericHelper::GetParameterValueString(param0, GenericValueType::Vec4);
					GenericParameterData& param1 = _node->GetDefaultParameterData()[1];
					std::string in_1 = GenericHelper::GetParameterValueString(param1, GenericValueType::Float);
					

					GET_NODE_OUTPUT(_node, in_0, 0, instance);
					GET_NODE_OUTPUT(_node, in_1, 1, instance);

					out_vars[0] = "var_" + std::to_string(instance->GetNextVarIndex());
					out_code = R"(vec4 {} = {} * vec4({});)";

					out_code = fmt::format(out_code, out_vars[0], in_0, in_1);


					return out_code;
				}
			}
		},
		{
			ShaderNodeType::Multiply_Vec3_Function,
			{
				{GenericNodeInput{GenericValueType::Vec3, 0, 0}, GenericNodeInput{GenericValueType::Vec3, 0, 0}},
				{{GenericValueType::Vec3, 0}},
				[](std::string* out_vars, GenericNode* node, GenericGraphInstance* graph_instance) -> std::string
				{
					ShaderGraphInstance* instance = (ShaderGraphInstance*)graph_instance;
					ConstantNode* _node = (ConstantNode*)node;
					std::string out_code = "";
					GenericParameterData& param0 = _node->GetDefaultParameterData()[0];
					std::string in_0 = GenericHelper::GetParameterValueString(param0, GenericValueType::Vec3);
					GenericParameterData& param1 = _node->GetDefaultParameterData()[1];
					std::string in_1 = GenericHelper::GetParameterValueString(param1, GenericValueType::Vec3);
					

					GET_NODE_OUTPUT(_node, in_0, 0, instance);
					GET_NODE_OUTPUT(_node, in_1, 1, instance);

					out_vars[0] = "var_" + std::to_string(instance->GetNextVarIndex());
					out_code = R"(vec3 {} = {} * {};)";

					out_code = fmt::format(out_code, out_vars[0], in_0, in_1);


					return out_code;
				}
			}
		},
		{
			ShaderNodeType::Multiply_Vec3_Float_Function,
			{
				{GenericNodeInput{GenericValueType::Vec3, 0, 0}, GenericNodeInput{GenericValueType::Float, 0, 0}},
				{{GenericValueType::Vec3, 0}},
				[](std::string* out_vars, GenericNode* node, GenericGraphInstance* graph_instance) -> std::string
				{
					ShaderGraphInstance* instance = (ShaderGraphInstance*)graph_instance;
					ConstantNode* _node = (ConstantNode*)node;
					std::string out_code = "";
					GenericParameterData& param0 = _node->GetDefaultParameterData()[0];
					std::string in_0 = GenericHelper::GetParameterValueString(param0, GenericValueType::Vec3);
					GenericParameterData& param1 = _node->GetDefaultParameterData()[1];
					std::string in_1 = GenericHelper::GetParameterValueString(param1, GenericValueType::Float);
					

					GET_NODE_OUTPUT(_node, in_0, 0, instance);
					GET_NODE_OUTPUT(_node, in_1, 1, instance);

					out_vars[0] = "var_" + std::to_string(instance->GetNextVarIndex());
					out_code = R"(vec3 {} = {} * vec3({});)";

					out_code = fmt::format(out_code, out_vars[0], in_0, in_1);


					return out_code;
				}
			}
		},
		{
			ShaderNodeType::Multiply_Vec2_Function,
			{
				{GenericNodeInput{GenericValueType::Vec2, 0, 0}, GenericNodeInput{GenericValueType::Vec2, 0, 0}},
				{{GenericValueType::Vec2, 0}},
				[](std::string* out_vars, GenericNode* node, GenericGraphInstance* graph_instance) -> std::string
				{
					ShaderGraphInstance* instance = (ShaderGraphInstance*)graph_instance;
					ConstantNode* _node = (ConstantNode*)node;
					std::string out_code = "";
					GenericParameterData& param0 = _node->GetDefaultParameterData()[0];
					std::string in_0 = GenericHelper::GetParameterValueString(param0, GenericValueType::Vec2);
					GenericParameterData& param1 = _node->GetDefaultParameterData()[1];
					std::string in_1 = GenericHelper::GetParameterValueString(param1, GenericValueType::Vec2);
					

					GET_NODE_OUTPUT(_node, in_0, 0, instance);
					GET_NODE_OUTPUT(_node, in_1, 1, instance);

					out_vars[0] = "var_" + std::to_string(instance->GetNextVarIndex());
					out_code = R"(vec2 {} = {} * {};)";

					out_code = fmt::format(out_code, out_vars[0], in_0, in_1);


					return out_code;
				}
			}
		},
		{
			ShaderNodeType::Multiply_Vec2_Float_Function,
			{
				{GenericNodeInput{GenericValueType::Vec2, 0, 0}, GenericNodeInput{GenericValueType::Float, 0, 0}},
				{{GenericValueType::Vec2, 0}},
				[](std::string* out_vars, GenericNode* node, GenericGraphInstance* graph_instance) -> std::string
				{
					ShaderGraphInstance* instance = (ShaderGraphInstance*)graph_instance;
					ConstantNode* _node = (ConstantNode*)node;
					std::string out_code = "";
					GenericParameterData& param0 = _node->GetDefaultParameterData()[0];
					std::string in_0 = GenericHelper::GetParameterValueString(param0, GenericValueType::Vec2);
					GenericParameterData& param1 = _node->GetDefaultParameterData()[1];
					std::string in_1 = GenericHelper::GetParameterValueString(param1, GenericValueType::Float);
					

					GET_NODE_OUTPUT(_node, in_0, 0, instance);
					GET_NODE_OUTPUT(_node, in_1, 1, instance);

					out_vars[0] = "var_" + std::to_string(instance->GetNextVarIndex());
					out_code = R"(vec2 {} = {} * vec2({});)";

					out_code = fmt::format(out_code, out_vars[0], in_0, in_1);


					return out_code;
				}
			}
		},
		{
			ShaderNodeType::Multiply_Float_Function,
			{
				{GenericNodeInput{GenericValueType::Float, 0, 0}, GenericNodeInput{GenericValueType::Float, 0, 0}},
				{{GenericValueType::Float, 0}},
				[](std::string* out_vars, GenericNode* node, GenericGraphInstance* graph_instance) -> std::string
				{
					ShaderGraphInstance* instance = (ShaderGraphInstance*)graph_instance;
					ConstantNode* _node = (ConstantNode*)node;
					std::string out_code = "";
					GenericParameterData& param0 = _node->GetDefaultParameterData()[0];
					std::string in_0 = GenericHelper::GetParameterValueString(param0, GenericValueType::Float);
					GenericParameterData& param1 = _node->GetDefaultParameterData()[1];
					std::string in_1 = GenericHelper::GetParameterValueString(param1, GenericValueType::Float);
					

					GET_NODE_OUTPUT(_node, in_0, 0, instance);
					GET_NODE_OUTPUT(_node, in_1, 1, instance);

					out_vars[0] = "var_" + std::to_string(instance->GetNextVarIndex());
					out_code = R"(float {} = {} * {};)";

					out_code = fmt::format(out_code, out_vars[0], in_0, in_1);


					return out_code;
				}
			}
		},
		{
			ShaderNodeType::Add_Vec4_Function,
			{
				{GenericNodeInput{GenericValueType::Vec4, 0, 0}, GenericNodeInput{GenericValueType::Vec4, 0, 0}},
				{{GenericValueType::Vec4, 0}},
				[](std::string* out_vars, GenericNode* node, GenericGraphInstance* graph_instance) -> std::string
				{
					ShaderGraphInstance* instance = (ShaderGraphInstance*)graph_instance;
					ConstantNode* _node = (ConstantNode*)node;
					std::string out_code = "";
					GenericParameterData& param0 = _node->GetDefaultParameterData()[0];
					std::string in_0 = GenericHelper::GetParameterValueString(param0, GenericValueType::Vec4);
					GenericParameterData& param1 = _node->GetDefaultParameterData()[1];
					std::string in_1 = GenericHelper::GetParameterValueString(param1, GenericValueType::Vec4);
					

					GET_NODE_OUTPUT(_node, in_0, 0, instance);
					GET_NODE_OUTPUT(_node, in_1, 1, instance);

					out_vars[0] = "var_" + std::to_string(instance->GetNextVarIndex());
					out_code = R"(vec4 {} = {} + {};)";

					out_code = fmt::format(out_code, out_vars[0], in_0, in_1);


					return out_code;
				}
			}
		},
		{
			ShaderNodeType::Add_Vec4_Float_Function,
			{
				{GenericNodeInput{GenericValueType::Vec4, 0, 0}, GenericNodeInput{GenericValueType::Float, 0, 0}},
				{{GenericValueType::Vec4, 0}},
				[](std::string* out_vars, GenericNode* node, GenericGraphInstance* graph_instance) -> std::string
				{
					ShaderGraphInstance* instance = (ShaderGraphInstance*)graph_instance;
					ConstantNode* _node = (ConstantNode*)node;
					std::string out_code = "";
					GenericParameterData& param0 = _node->GetDefaultParameterData()[0];
					std::string in_0 = GenericHelper::GetParameterValueString(param0, GenericValueType::Vec4);
					GenericParameterData& param1 = _node->GetDefaultParameterData()[1];
					std::string in_1 = GenericHelper::GetParameterValueString(param1, GenericValueType::Float);
					

					GET_NODE_OUTPUT(_node, in_0, 0, instance);
					GET_NODE_OUTPUT(_node, in_1, 1, instance);

					out_vars[0] = "var_" + std::to_string(instance->GetNextVarIndex());
					out_code = R"(vec4 {} = {} + vec4({});)";

					out_code = fmt::format(out_code, out_vars[0], in_0, in_1);


					return out_code;
				}
			}
		},
		{
			ShaderNodeType::Add_Vec3_Function,
			{
				{GenericNodeInput{GenericValueType::Vec3, 0, 0}, GenericNodeInput{GenericValueType::Vec3, 0, 0}},
				{{GenericValueType::Vec3, 0}},
				[](std::string* out_vars, GenericNode* node, GenericGraphInstance* graph_instance) -> std::string
				{
					ShaderGraphInstance* instance = (ShaderGraphInstance*)graph_instance;
					ConstantNode* _node = (ConstantNode*)node;
					std::string out_code = "";
					GenericParameterData& param0 = _node->GetDefaultParameterData()[0];
					std::string in_0 = GenericHelper::GetParameterValueString(param0, GenericValueType::Vec3);
					GenericParameterData& param1 = _node->GetDefaultParameterData()[1];
					std::string in_1 = GenericHelper::GetParameterValueString(param1, GenericValueType::Vec3);
					

					GET_NODE_OUTPUT(_node, in_0, 0, instance);
					GET_NODE_OUTPUT(_node, in_1, 1, instance);

					out_vars[0] = "var_" + std::to_string(instance->GetNextVarIndex());
					out_code = R"(vec3 {} = {} + {};)";

					out_code = fmt::format(out_code, out_vars[0], in_0, in_1);


					return out_code;
				}
			}
		},
		{
			ShaderNodeType::Add_Vec3_Float_Function,
			{
				{GenericNodeInput{GenericValueType::Vec3, 0, 0}, GenericNodeInput{GenericValueType::Float, 0, 0}},
				{{GenericValueType::Vec3, 0}},
				[](std::string* out_vars, GenericNode* node, GenericGraphInstance* graph_instance) -> std::string
				{
					ShaderGraphInstance* instance = (ShaderGraphInstance*)graph_instance;
					ConstantNode* _node = (ConstantNode*)node;
					std::string out_code = "";
					GenericParameterData& param0 = _node->GetDefaultParameterData()[0];
					std::string in_0 = GenericHelper::GetParameterValueString(param0, GenericValueType::Vec3);
					GenericParameterData& param1 = _node->GetDefaultParameterData()[1];
					std::string in_1 = GenericHelper::GetParameterValueString(param1, GenericValueType::Float);
					

					GET_NODE_OUTPUT(_node, in_0, 0, instance);
					GET_NODE_OUTPUT(_node, in_1, 1, instance);

					out_vars[0] = "var_" + std::to_string(instance->GetNextVarIndex());
					out_code = R"(vec3 {} = {} + vec3({});)";

					out_code = fmt::format(out_code, out_vars[0], in_0, in_1);


					return out_code;
				}
			}
		},
		{
			ShaderNodeType::Add_Vec2_Function,
			{
				{GenericNodeInput{GenericValueType::Vec2, 0, 0}, GenericNodeInput{GenericValueType::Vec2, 0, 0}},
				{{GenericValueType::Vec2, 0}},
				[](std::string* out_vars, GenericNode* node, GenericGraphInstance* graph_instance) -> std::string
				{
					ShaderGraphInstance* instance = (ShaderGraphInstance*)graph_instance;
					ConstantNode* _node = (ConstantNode*)node;
					std::string out_code = "";
					GenericParameterData& param0 = _node->GetDefaultParameterData()[0];
					std::string in_0 = GenericHelper::GetParameterValueString(param0, GenericValueType::Vec2);
					GenericParameterData& param1 = _node->GetDefaultParameterData()[1];
					std::string in_1 = GenericHelper::GetParameterValueString(param1, GenericValueType::Vec2);
					

					GET_NODE_OUTPUT(_node, in_0, 0, instance);
					GET_NODE_OUTPUT(_node, in_1, 1, instance);

					out_vars[0] = "var_" + std::to_string(instance->GetNextVarIndex());
					out_code = R"(vec2 {} = {} + {};)";

					out_code = fmt::format(out_code, out_vars[0], in_0, in_1);


					return out_code;
				}
			}
		},
		{
			ShaderNodeType::Add_Vec2_Float_Function,
			{
				{GenericNodeInput{GenericValueType::Vec2, 0, 0}, GenericNodeInput{GenericValueType::Float, 0, 0}},
				{{GenericValueType::Vec2, 0}},
				[](std::string* out_vars, GenericNode* node, GenericGraphInstance* graph_instance) -> std::string
				{
					ShaderGraphInstance* instance = (ShaderGraphInstance*)graph_instance;
					ConstantNode* _node = (ConstantNode*)node;
					std::string out_code = "";
					GenericParameterData& param0 = _node->GetDefaultParameterData()[0];
					std::string in_0 = GenericHelper::GetParameterValueString(param0, GenericValueType::Vec2);
					GenericParameterData& param1 = _node->GetDefaultParameterData()[1];
					std::string in_1 = GenericHelper::GetParameterValueString(param1, GenericValueType::Float);
					

					GET_NODE_OUTPUT(_node, in_0, 0, instance);
					GET_NODE_OUTPUT(_node, in_1, 1, instance);

					out_vars[0] = "var_" + std::to_string(instance->GetNextVarIndex());
					out_code = R"(vec2 {} = {} + vec2({});)";

					out_code = fmt::format(out_code, out_vars[0], in_0, in_1);


					return out_code;
				}
			}
		},
		{
			ShaderNodeType::Add_Float_Function,
			{
				{GenericNodeInput{GenericValueType::Float, 0, 0}, GenericNodeInput{GenericValueType::Float, 0, 0}},
				{{GenericValueType::Float, 0}},
				[](std::string* out_vars, GenericNode* node, GenericGraphInstance* graph_instance) -> std::string
				{
					ShaderGraphInstance* instance = (ShaderGraphInstance*)graph_instance;
					ConstantNode* _node = (ConstantNode*)node;
					std::string out_code = "";
					GenericParameterData& param0 = _node->GetDefaultParameterData()[0];
					std::string in_0 = GenericHelper::GetParameterValueString(param0, GenericValueType::Float);
					GenericParameterData& param1 = _node->GetDefaultParameterData()[1];
					std::string in_1 = GenericHelper::GetParameterValueString(param1, GenericValueType::Float);
					

					GET_NODE_OUTPUT(_node, in_0, 0, instance);
					GET_NODE_OUTPUT(_node, in_1, 1, instance);

					out_vars[0] = "var_" + std::to_string(instance->GetNextVarIndex());
					out_code = R"(float {} = {} + {};)";

					out_code = fmt::format(out_code, out_vars[0], in_0, in_1);


					return out_code;
				}
			}
		},
		{
			ShaderNodeType::Object_UV_Variable,
			{
				{},
				{{GenericValueType::Vec2, 0}},
				[](std::string* out_vars, GenericNode* node, GenericGraphInstance* graph_instance) -> std::string
				{
					ShaderGraphInstance* instance = (ShaderGraphInstance*)graph_instance;
					ConstantNode* _node = (ConstantNode*)node;
					std::string out_code = "";

					out_vars[0] = "_texCoord";

					return out_code;
				}
			}
		},
		{
			ShaderNodeType::Object_Normal_Variable,
			{
				{},
				{{GenericValueType::Vec3, 0}},
				[](std::string* out_vars, GenericNode* node, GenericGraphInstance* graph_instance) -> std::string
				{
					ShaderGraphInstance* instance = (ShaderGraphInstance*)graph_instance;
					ConstantNode* _node = (ConstantNode*)node;
					std::string out_code = "";

					out_vars[0] = "_normal_";

					return out_code;
				}
			}
		},
		{
			ShaderNodeType::Object_Tangent_Variable,
			{
				{},
				{{GenericValueType::Vec4, 0}},
				[](std::string* out_vars, GenericNode* node, GenericGraphInstance* graph_instance) -> std::string
				{
					ShaderGraphInstance* instance = (ShaderGraphInstance*)graph_instance;
					ConstantNode* _node = (ConstantNode*)node;
					std::string out_code = "";

					out_vars[0] = "_tangent_";

					return out_code;
				}
			}
		},
		{
			ShaderNodeType::View_Direction_Variable,
			{
				{},
				{{GenericValueType::Vec3, 0}},
				[](std::string* out_vars, GenericNode* node, GenericGraphInstance* graph_instance) -> std::string
				{
					ShaderGraphInstance* instance = (ShaderGraphInstance*)graph_instance;
					ConstantNode* _node = (ConstantNode*)node;
					std::string out_code = "";

					out_vars[0] = "view_direction";

					return out_code;
				}
			}
		},
		{
			ShaderNodeType::Split_Vec4_Helper,
			{
				{{GenericValueType::Vec4, 0, 0}},
				{{GenericValueType::Vec3, 0}, {GenericValueType::Vec2, 1}, {GenericValueType::Float, 2}, {GenericValueType::Float, 3}, {GenericValueType::Float, 4}, {GenericValueType::Float, 5}},
				[](std::string* out_vars, GenericNode* node, GenericGraphInstance* graph_instance) -> std::string
				{
					ShaderGraphInstance* instance = (ShaderGraphInstance*)graph_instance;
					ConstantNode* _node = (ConstantNode*)node;
					std::string out_code = "";

					GenericParameterData& param0 = _node->GetDefaultParameterData()[0];
					std::string in_0 = GenericHelper::GetParameterValueString(param0, GenericValueType::Vec4);

					GET_NODE_OUTPUT(_node, in_0, 0, instance);

					out_vars[0] = in_0 + ".xyz";
					out_vars[1] = in_0 + ".xy";
					out_vars[2] = in_0 + ".x";
					out_vars[3] = in_0 + ".y";
					out_vars[4] = in_0 + ".z";
					out_vars[5] = in_0 + ".w";

					return out_code;
				}
			}
		},
		{
			ShaderNodeType::Split_Vec3_Helper,
			{
				{{GenericValueType::Vec3, 0, 0}},
				{{GenericValueType::Vec2, 0}, {GenericValueType::Float, 1}, {GenericValueType::Float, 2}, {GenericValueType::Float, 3}},
				[](std::string* out_vars, GenericNode* node, GenericGraphInstance* graph_instance) -> std::string
				{
					ShaderGraphInstance* instance = (ShaderGraphInstance*)graph_instance;
					ConstantNode* _node = (ConstantNode*)node;
					std::string out_code = "";

					GenericParameterData& param0 = _node->GetDefaultParameterData()[0];
					std::string in_0 = GenericHelper::GetParameterValueString(param0, GenericValueType::Vec3);

					GET_NODE_OUTPUT(_node, in_0, 0, instance);

					out_vars[0] = in_0 + ".xy";
					out_vars[1] = in_0 + ".x";
					out_vars[2] = in_0 + ".y";
					out_vars[3] = in_0 + ".z";

					return out_code;
				}
			}
		},
		{
			ShaderNodeType::Split_Vec2_Helper,
			{
				{{GenericValueType::Vec2, 0, 0}},
				{{GenericValueType::Float, 0}, {GenericValueType::Float, 1}},
				[](std::string* out_vars, GenericNode* node, GenericGraphInstance* graph_instance) -> std::string
				{
					ShaderGraphInstance* instance = (ShaderGraphInstance*)graph_instance;
					ConstantNode* _node = (ConstantNode*)node;
					std::string out_code = "";

					GenericParameterData& param0 = _node->GetDefaultParameterData()[0];
					std::string in_0 = GenericHelper::GetParameterValueString(param0, GenericValueType::Vec2);

					GET_NODE_OUTPUT(_node, in_0, 0, instance);

					out_vars[0] = in_0 + ".x";
					out_vars[1] = in_0 + ".y";

					return out_code;
				}
			}
		},
		{
			ShaderNodeType::Transparent_UnLit_Node,
			{
				{{GenericValueType::Vec3, 0, 0}, {GenericValueType::Float, 0, 0}},
				{},
				[](std::string* out_vars, GenericNode* node, GenericGraphInstance* graph_instance) -> std::string
				{
					ShaderGraphInstance* instance = (ShaderGraphInstance*)graph_instance;
					ConstantNode* _node = (ConstantNode*)node;
					std::string out_code = "";

					GenericParameterData& param0 = _node->GetDefaultParameterData()[0];
					std::string in_0 = GenericHelper::GetParameterValueString(param0, GenericValueType::Vec3);
					GenericParameterData& param1 = _node->GetDefaultParameterData()[1];
					std::string in_1 = GenericHelper::GetParameterValueString(param1, GenericValueType::Float);

					GET_NODE_OUTPUT(_node, in_0, 0, instance);
					GET_NODE_OUTPUT(_node, in_1, 1, instance);

					std::string code = R"(
	vec4 out_color;
    out_color.rgb = {};
    out_color.a = {};
    PROCESS_COLOR(out_color);
			)";

					out_code = fmt::format(code, in_0, in_1);

					return out_code;
				}
			}
		},
		{
			ShaderNodeType::Screen_Color_Variable,
			{
				{},
				{{GenericValueType::Sampler2D, 0}},
				[](std::string* out_vars, GenericNode* node, GenericGraphInstance* graph_instance) -> std::string
				{
					ShaderGraphInstance* instance = (ShaderGraphInstance*)graph_instance;
					ConstantNode* _node = (ConstantNode*)node;
					std::string out_code = "";

					out_vars[0] = "_screen_color";

					return out_code;
				}
			}
		},
		{
			ShaderNodeType::Simple_Noise_Function,
			{
				{{GenericValueType::Vec2, 0, 0}, {GenericValueType::Float, 0, 0}},
				{{GenericValueType::Float, 0}},
				[](std::string* out_vars, GenericNode* node, GenericGraphInstance* graph_instance) -> std::string
				{
					ShaderGraphInstance* instance = (ShaderGraphInstance*)graph_instance;
					ConstantNode* _node = (ConstantNode*)node;
					std::string out_code = "";


					GenericParameterData& param0 = _node->GetDefaultParameterData()[0];
					std::string in_0 = GenericHelper::GetParameterValueString(param0, GenericValueType::Vec2);
					GenericParameterData& param1 = _node->GetDefaultParameterData()[1];
					std::string in_1 = GenericHelper::GetParameterValueString(param1, GenericValueType::Float);

					GET_NODE_OUTPUT(_node, in_0, 0, instance);
					GET_NODE_OUTPUT(_node, in_1, 1, instance);

					out_vars[0] = "var_" + std::to_string(instance->GetNextVarIndex());

					out_code = "float {} = simple_noise({}, {});";

					out_code = fmt::format(out_code, out_vars[0], in_0, in_1);
					

					return out_code;
				}
			}
		},
		{
			ShaderNodeType::Gradient_Noise_Function,
			{
				{{GenericValueType::Vec2, 0, 0}, {GenericValueType::Float, 0, 0}},
				{{GenericValueType::Float, 0}},
				[](std::string* out_vars, GenericNode* node, GenericGraphInstance* graph_instance) -> std::string
				{
					ShaderGraphInstance* instance = (ShaderGraphInstance*)graph_instance;
					ConstantNode* _node = (ConstantNode*)node;
					std::string out_code = "";


					GenericParameterData& param0 = _node->GetDefaultParameterData()[0];
					std::string in_0 = GenericHelper::GetParameterValueString(param0, GenericValueType::Vec2);
					GenericParameterData& param1 = _node->GetDefaultParameterData()[1];
					std::string in_1 = GenericHelper::GetParameterValueString(param1, GenericValueType::Float);

					GET_NODE_OUTPUT(_node, in_0, 0, instance);
					GET_NODE_OUTPUT(_node, in_1, 1, instance);

					out_vars[0] = "var_" + std::to_string(instance->GetNextVarIndex());

					out_code = "float {} = gradientNoise({}, {});";

					out_code = fmt::format(out_code, out_vars[0], in_0, in_1);
					

					return out_code;
				}
			}
		},
		{
			ShaderNodeType::Twist_Function,
			{
				{{GenericValueType::Vec2, 0, 0}, {GenericValueType::Vec2, 0, 0}, {GenericValueType::Float, 0, 0}, {GenericValueType::Vec2, 0, 0}},
				{{GenericValueType::Vec2, 0}},
				[](std::string* out_vars, GenericNode* node, GenericGraphInstance* graph_instance) -> std::string
				{
					ShaderGraphInstance* instance = (ShaderGraphInstance*)graph_instance;
					ConstantNode* _node = (ConstantNode*)node;
					std::string out_code = "";


					GenericParameterData& param0 = _node->GetDefaultParameterData()[0];
					std::string in_0 = GenericHelper::GetParameterValueString(param0, GenericValueType::Vec2);
					GenericParameterData& param1 = _node->GetDefaultParameterData()[1];
					std::string in_1 = GenericHelper::GetParameterValueString(param1, GenericValueType::Vec2);
					GenericParameterData& param2 = _node->GetDefaultParameterData()[2];
					std::string in_2 = GenericHelper::GetParameterValueString(param2, GenericValueType::Float);
					GenericParameterData& param3 = _node->GetDefaultParameterData()[3];
					std::string in_3 = GenericHelper::GetParameterValueString(param3, GenericValueType::Vec2);

					GET_NODE_OUTPUT(_node, in_0, 0, instance);
					GET_NODE_OUTPUT(_node, in_1, 1, instance);
					GET_NODE_OUTPUT(_node, in_2, 2, instance);
					GET_NODE_OUTPUT(_node, in_3, 3, instance);

					out_vars[0] = "var_" + std::to_string(instance->GetNextVarIndex());

					out_code = "vec2 {} = twist({}, {}, {}, {});";

					out_code = fmt::format(out_code, out_vars[0], in_0, in_1, in_2, in_3);
					

					return out_code;
				}
			}
		},
		{
			ShaderNodeType::Normal_From_Gradient_Noise,
			{
				{{GenericValueType::Vec2, 0, 0}, {GenericValueType::Float, 0, 0}, {GenericValueType::Float, 0, 0}, {GenericValueType::Float, 0, 0}},
				{{GenericValueType::Vec3, 0}},
				[](std::string* out_vars, GenericNode* node, GenericGraphInstance* graph_instance) -> std::string
				{
					ShaderGraphInstance* instance = (ShaderGraphInstance*)graph_instance;
					ConstantNode* _node = (ConstantNode*)node;
					std::string out_code = "";


					GenericParameterData& param0 = _node->GetDefaultParameterData()[0];
					std::string in_0 = GenericHelper::GetParameterValueString(param0, GenericValueType::Vec2);
					GenericParameterData& param1 = _node->GetDefaultParameterData()[1];
					std::string in_1 = GenericHelper::GetParameterValueString(param1, GenericValueType::Float);
					GenericParameterData& param2 = _node->GetDefaultParameterData()[2];
					std::string in_2 = GenericHelper::GetParameterValueString(param2, GenericValueType::Float);
					GenericParameterData& param3 = _node->GetDefaultParameterData()[3];
					std::string in_3 = GenericHelper::GetParameterValueString(param3, GenericValueType::Float);

					GET_NODE_OUTPUT(_node, in_0, 0, instance);
					GET_NODE_OUTPUT(_node, in_1, 1, instance);
					GET_NODE_OUTPUT(_node, in_2, 2, instance);
					GET_NODE_OUTPUT(_node, in_3, 3, instance);

					out_vars[0] = "var_" + std::to_string(instance->GetNextVarIndex());

					out_code = "vec3 {} = normalFromGradientNoise({}, {}, {}, {});";

					out_code = fmt::format(out_code, out_vars[0], in_0, in_1, in_2, in_3);
					

					return out_code;
				}
			}
		},
		{
			ShaderNodeType::Time_Variable,
			{
				{},
				{{GenericValueType::Float, 0}},
				[](std::string* out_vars, GenericNode* node, GenericGraphInstance* graph_instance) -> std::string
				{
					ShaderGraphInstance* instance = (ShaderGraphInstance*)graph_instance;
					ConstantNode* _node = (ConstantNode*)node;
					std::string out_code = "";

					out_vars[0] = "_time_values.x";

					return out_code;
				}
			}
		},
		{
			ShaderNodeType::Sin_Float,
			{
				{{GenericValueType::Float, 0, 0}},
				{{GenericValueType::Float, 0}},
				[](std::string* out_vars, GenericNode* node, GenericGraphInstance* graph_instance) -> std::string
				{
					ShaderGraphInstance* instance = (ShaderGraphInstance*)graph_instance;
					ConstantNode* _node = (ConstantNode*)node;
					std::string out_code = "";


					GenericParameterData& param0 = _node->GetDefaultParameterData()[0];
					std::string in_0 = GenericHelper::GetParameterValueString(param0, GenericValueType::Vec2);
					

					GET_NODE_OUTPUT(_node, in_0, 0, instance);

					out_vars[0] = "var_" + std::to_string(instance->GetNextVarIndex());

					out_code = "float {} = sin({});";

					out_code = fmt::format(out_code, out_vars[0], in_0);


					return out_code;
				}
			}
		},
		{
			ShaderNodeType::Radians_Float,
			{
				{{GenericValueType::Float, 0, 0}},
				{{GenericValueType::Float, 0}},
				[](std::string* out_vars, GenericNode* node, GenericGraphInstance* graph_instance) -> std::string
				{
					ShaderGraphInstance* instance = (ShaderGraphInstance*)graph_instance;
					ConstantNode* _node = (ConstantNode*)node;
					std::string out_code = "";


					GenericParameterData& param0 = _node->GetDefaultParameterData()[0];
					std::string in_0 = GenericHelper::GetParameterValueString(param0, GenericValueType::Vec2);
					

					GET_NODE_OUTPUT(_node, in_0, 0, instance);

					out_vars[0] = "var_" + std::to_string(instance->GetNextVarIndex());

					out_code = "float {} = radians({});";

					out_code = fmt::format(out_code, out_vars[0], in_0);


					return out_code;
				}
			}
		},
		{
			ShaderNodeType::Fract_Float,
			{
				{{GenericValueType::Float, 0, 0}},
				{{GenericValueType::Float, 0}},
				[](std::string* out_vars, GenericNode* node, GenericGraphInstance* graph_instance) -> std::string
				{
					ShaderGraphInstance* instance = (ShaderGraphInstance*)graph_instance;
					ConstantNode* _node = (ConstantNode*)node;
					std::string out_code = "";


					GenericParameterData& param0 = _node->GetDefaultParameterData()[0];
					std::string in_0 = GenericHelper::GetParameterValueString(param0, GenericValueType::Vec2);
					

					GET_NODE_OUTPUT(_node, in_0, 0, instance);

					out_vars[0] = "var_" + std::to_string(instance->GetNextVarIndex());

					out_code = "float {} = fract({});";

					out_code = fmt::format(out_code, out_vars[0], in_0);


					return out_code;
				}
			}
		},
		{
			ShaderNodeType::Screen_UV_Variable,
			{
				{},
				{{GenericValueType::Vec2, 0}},
				[](std::string* out_vars, GenericNode* node, GenericGraphInstance* graph_instance) -> std::string
				{
					ShaderGraphInstance* instance = (ShaderGraphInstance*)graph_instance;
					ConstantNode* _node = (ConstantNode*)node;
					std::string out_code = "";

					out_vars[0] = "screen_uv";

					return out_code;
				}
			}
		}
	};

	bool AddNode::Instanciate(GenericGraphInstance* graph_instance)
	{
		ShaderGraphInstance* instance = (ShaderGraphInstance*)graph_instance;

		instance->GetNodesData()[m_uuid] = new NodeData;

		return true;
	}

	void AddNode::Run(GenericGraphInstance* graph_instance)
	{
		ShaderGraphInstance* instance = (ShaderGraphInstance*)graph_instance;
		NodeData* info = (NodeData*)instance->GetNodesData()[m_uuid];

		if (info->frame_index == Application::get_instance()->GetUpdateID())
		{
			return;
		}
		info->frame_index = Application::get_instance()->GetUpdateID();

		std::string in_a = GenericHelper::GetParameterValueString(m_a, m_type);
		

		if (m_inputs[0].node_id != 0)
		{
			ShaderGraphNode* in_a_node = (ShaderGraphNode*)instance->GetShaderGraph()->GetNode(m_inputs[0].node_id);
			if (in_a_node)
			{
				in_a_node->Run(instance);
				in_a = *(std::string*)in_a_node->GetValueInternal(instance, m_inputs[0].value_index);
			}
		}
		
		std::string in_b = GenericHelper::GetParameterValueString(m_b, m_type);

		if (m_inputs[1].node_id != 0)
		{
			ShaderGraphNode* in_b_node = (ShaderGraphNode*)instance->GetShaderGraph()->GetNode(m_inputs[1].node_id);
			if (in_b_node)
			{
				in_b_node->Run(instance);
				in_b = *(std::string*)in_b_node->GetValueInternal(instance, m_inputs[1].value_index);
			}
		}

		std::string type_name = GenericHelper::GetTypeString(m_type);
		std::string var_name = "var_" + std::to_string(instance->GetNextVarIndex());

		info->code = "{} {} = {} + {};";
		info->code = fmt::format(info->code, type_name, var_name, in_a, in_b, "sdsd");
		info->out_0 = var_name;

		instance->WriteNodeCode(info->code);


	}

	void AddNode::Init(GenericGraph* graph)
	{
		GenericNodeInput input = {};
		input.node_id = 0;
		input.type = m_type;
		input.value_index = INVALID_ID;

		m_inputs.push_back(input);
		m_inputs.push_back(input);

		GenericNodeOutput output = {};
		output.type = m_type;
		output.value_index = 0;

		m_outputs.push_back(output);
	}

	void AddNode::Destroy(GenericGraph* graph)
	{
	}

	std::string& AddNode::GetCode(GenericGraphInstance* graph_instance)
	{
		ShaderGraphInstance* instance = (ShaderGraphInstance*)graph_instance;
		NodeData* info = (NodeData*)instance->GetNodesData()[m_uuid];

		return info->code;
	}

	void* AddNode::GetValueInternal(GenericGraphInstance* graph_instance, uint32_t value_index)
	{
		ShaderGraphInstance* instance = (ShaderGraphInstance*)graph_instance;
		NodeData* info = (NodeData*)instance->GetNodesData()[m_uuid];
		switch (value_index)
		{
		case 0:
		{
			return &info->out_0;
			break;
		}
		}

		return nullptr;
	}
	
	
	bool ConstantNode::Instanciate(GenericGraphInstance* graph_instance)
	{
		ShaderGraphInstance* instance = (ShaderGraphInstance*)graph_instance;

		instance->GetNodesData()[m_uuid] = new NodeData;

		return true;
	}

	void ConstantNode::Run(GenericGraphInstance* graph_instance)
	{
		ShaderGraphInstance* instance = (ShaderGraphInstance*)graph_instance;
		NodeData* info = (NodeData*)instance->GetNodesData()[m_uuid];

		if (info->frame_index == Application::get_instance()->GetUpdateID())
		{
			return;
		}
		info->frame_index = Application::get_instance()->GetUpdateID();


		if (m_nodeType == ShaderNodeType::UnKnown)
		{
			return;
		}

		info->code = node_meta_data[m_nodeType].gen_code(info->out_, this, instance);

		instance->WriteNodeCode(info->code);


	}

	void ConstantNode::Init(GenericGraph* graph)
	{
		GenericNodeOutput output = {};
		output.type = m_type;
		output.value_index = 0;

		m_outputs.push_back(output);
	}

	void ConstantNode::Destroy(GenericGraph* graph)
	{
	}

	void ConstantNode::SetType(ShaderNodeType type)
	{
		m_nodeType = type;
		m_inputs = node_meta_data[type].inputs;
		m_outputs = node_meta_data[type].outputs;
	}

	std::string& ConstantNode::GetCode(GenericGraphInstance* graph_instance)
	{
		ShaderGraphInstance* instance = (ShaderGraphInstance*)graph_instance;
		NodeData* info = (NodeData*)instance->GetNodesData()[m_uuid];

		return info->code;
	}

	void* ConstantNode::GetValueInternal(GenericGraphInstance* graph_instance, uint32_t value_index)
	{
		ShaderGraphInstance* instance = (ShaderGraphInstance*)graph_instance;
		NodeData* info = (NodeData*)instance->GetNodesData()[m_uuid];
		switch (value_index)
		{
		case 0:
		{
			return &info->out_[0];
			break;
		}
		case 1:
		{
			return &info->out_[1];
			break;
		}
		case 2:
		{
			return &info->out_[2];
			break;
		}
		case 3:
		{
			return &info->out_[3];
			break;
		}
		case 4:
		{
			return &info->out_[4];
			break;
		}
		case 5:
		{
			return &info->out_[5];
			break;
		}
		}

		return nullptr;
	}

	bool GetVariableNode::Instanciate(GenericGraphInstance* graph_instance)
	{
		ShaderGraphInstance* instance = (ShaderGraphInstance*)graph_instance;

		instance->GetNodesData()[m_uuid] = new NodeData;

		return true;
	}

	void GetVariableNode::Run(GenericGraphInstance* graph_instance)
	{
		ShaderGraphInstance* instance = (ShaderGraphInstance*)graph_instance;
		NodeData* info = (NodeData*)instance->GetNodesData()[m_uuid];

		if (info->frame_index == Application::get_instance()->GetUpdateID())
		{
			return;
		}
		info->frame_index = Application::get_instance()->GetUpdateID();

		if (m_varIndex < 0)
		{
			return;
		}

		GenericParameter& param = instance->GetShaderGraph()->GetParameters()[m_varIndex];


		switch (param.type)
		{
		case GenericValueType::Float:
		case GenericValueType::Int:
		case GenericValueType::Vec2:
		case GenericValueType::Vec3:
		case GenericValueType::Vec4:
		{
			info->out_0 = "objects[binding_index.draw_instance_index.x]." + *instance->GetParamString(param.name);
			break;
		}
		case GenericValueType::Sampler2D:
		{
			info->out_0 = *instance->GetParamString(param.name);
			break;
		}
		}


	}

	void GetVariableNode::Init(GenericGraph* graph)
	{
		GenericNodeOutput output = {};
		output.type = GenericValueType::Unknown;
		output.value_index = 0;

		m_outputs.push_back(output);
	}

	void GetVariableNode::Destroy(GenericGraph* graph)
	{
	}

	std::string& GetVariableNode::GetCode(GenericGraphInstance* graph_instance)
	{
		ShaderGraphInstance* instance = (ShaderGraphInstance*)graph_instance;
		NodeData* info = (NodeData*)instance->GetNodesData()[m_uuid];

		return info->code;
	}

	void* GetVariableNode::GetValueInternal(GenericGraphInstance* graph_instance, uint32_t value_index)
	{
		ShaderGraphInstance* instance = (ShaderGraphInstance*)graph_instance;
		NodeData* info = (NodeData*)instance->GetNodesData()[m_uuid];
		switch (value_index)
		{
		case 0:
		{
			return &info->out_0;
			break;
		}
		}

		return nullptr;
	}

	bool FinalPBRNode::Instanciate(GenericGraphInstance* graph_instance)
	{
		ShaderGraphInstance* instance = (ShaderGraphInstance*)graph_instance;

		instance->GetNodesData()[m_uuid] = new NodeData;

		return true;
	}

	void FinalPBRNode::Run(GenericGraphInstance* graph_instance)
	{
		ShaderGraphInstance* instance = (ShaderGraphInstance*)graph_instance;
		NodeData* info = (NodeData*)instance->GetNodesData()[m_uuid];

		if (info->frame_index == Application::get_instance()->GetUpdateID())
		{
			return;
		}
		info->frame_index = Application::get_instance()->GetUpdateID();

		std::string in_color = GenericHelper::GetParameterValueString(color, GenericValueType::Vec3);


		if (m_inputs[0].node_id != 0)
		{
			ShaderGraphNode* in_color_node = (ShaderGraphNode*)instance->GetShaderGraph()->GetNode(m_inputs[0].node_id);
			if (in_color_node)
			{
				in_color_node->Run(instance);
				in_color = *(std::string*)in_color_node->GetValueInternal(instance, m_inputs[0].value_index);
			}
		}
		
		std::string in_normal = "_normal_";


		if (m_inputs[1].node_id != 0)
		{
			ShaderGraphNode* in_normal_node = (ShaderGraphNode*)instance->GetShaderGraph()->GetNode(m_inputs[1].node_id);
			if (in_normal_node)
			{
				in_normal_node->Run(instance);
				in_normal = *(std::string*)in_normal_node->GetValueInternal(instance, m_inputs[1].value_index);
			}
		}
		
		std::string in_occlusion = GenericHelper::GetParameterValueString(occlusion, GenericValueType::Float);


		if (m_inputs[2].node_id != 0)
		{
			ShaderGraphNode* in_occlusion_node = (ShaderGraphNode*)instance->GetShaderGraph()->GetNode(m_inputs[2].node_id);
			if (in_occlusion_node)
			{
				in_occlusion_node->Run(instance);
				in_occlusion = *(std::string*)in_occlusion_node->GetValueInternal(instance, m_inputs[2].value_index);
			}
		}
		
		std::string in_metallic = GenericHelper::GetParameterValueString(metallic, GenericValueType::Float);


		if (m_inputs[3].node_id != 0)
		{
			ShaderGraphNode* in_metallic_node = (ShaderGraphNode*)instance->GetShaderGraph()->GetNode(m_inputs[3].node_id);
			if (in_metallic_node)
			{
				in_metallic_node->Run(instance);
				in_metallic = *(std::string*)in_metallic_node->GetValueInternal(instance, m_inputs[3].value_index);
			}
		}
		
		std::string in_roughness = GenericHelper::GetParameterValueString(roughness, GenericValueType::Float);


		if (m_inputs[4].node_id != 0)
		{
			ShaderGraphNode* in_roughness_node = (ShaderGraphNode*)instance->GetShaderGraph()->GetNode(m_inputs[4].node_id);
			if (in_roughness_node)
			{
				in_roughness_node->Run(instance);
				in_roughness = *(std::string*)in_roughness_node->GetValueInternal(instance, m_inputs[4].value_index);
			}
		}
		
		std::string in_emissionColor = GenericHelper::GetParameterValueString(emissionColor, GenericValueType::Vec3);


		if (m_inputs[5].node_id != 0)
		{
			ShaderGraphNode* in_emissionColor_node = (ShaderGraphNode*)instance->GetShaderGraph()->GetNode(m_inputs[5].node_id);
			if (in_emissionColor_node)
			{
				in_emissionColor_node->Run(instance);
				in_emissionColor = *(std::string*)in_emissionColor_node->GetValueInternal(instance, m_inputs[5].value_index);
			}
		}

		std::string code = R"(
	FRAG_POS = _fragPos;
	FRAG_NORMAL = {};
	FRAG_NORMAL_W = {};
	vec4 final_color_TRC_internal = vec4({}, 1.0f);
	uint color_compressed = vec4ToUint32(final_color_TRC_internal);
    FRAG_COLOR_R = color_compressed;
	float metal = {};
    float rough = {};
	vec4 surface_data_TRC_internal = vec4(metal, rough, 0.0f, 0.0f);
    uint surface_data_compressed = vec4ToUint32(surface_data_TRC_internal);
    FRAG_COLOR_G = surface_data_compressed;
	g_Emission.xyz = {};
			)";

		info->code = fmt::format(code, in_normal, in_occlusion, in_color, in_metallic, in_roughness, in_emissionColor);

		instance->WriteNodeCode(info->code);

	}

	void FinalPBRNode::Init(GenericGraph* graph)
	{
		GenericNodeInput input = {};
		input.node_id = 0;
		input.type = GenericValueType::Vec3;
		input.value_index = INVALID_ID;

		m_inputs.push_back(input);

		input.type = GenericValueType::Vec3;
		m_inputs.push_back(input);
		
		input.type = GenericValueType::Float;
		m_inputs.push_back(input);
		m_inputs.push_back(input);
		m_inputs.push_back(input);
		
		input.type = GenericValueType::Vec3;
		m_inputs.push_back(input);
	}

	void FinalPBRNode::Destroy(GenericGraph* graph)
	{
	}

	std::string& FinalPBRNode::GetCode(GenericGraphInstance* graph_instance)
	{
		ShaderGraphInstance* instance = (ShaderGraphInstance*)graph_instance;
		NodeData* info = (NodeData*)instance->GetNodesData()[m_uuid];

		return info->code;
	}

}