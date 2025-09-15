

#include "node_system/GenericGraph.h"
#include "shader_graph/ShaderGraphNode.h"
#include "shader_graph/ShaderGraph.h"
#include "../panels/GenericGraphEditor.h"

#include "imgui.h"
#include "imnodes/imnodes.h"

namespace trace {

	int generic_output_start_index = 64;
	int shift_amount = 24;

	uint32_t generic_value_color[(int)GenericValueType::Max] =
	{
		IM_COL32(255, 204, 153, 128),
		IM_COL32(51, 102, 255, 128),
		IM_COL32(51, 204, 204, 128),
		IM_COL32(153, 204, 0, 128),
		IM_COL32(255, 204, 0, 128),
		IM_COL32(200, 104, 0, 128),
		IM_COL32(155, 194, 85, 128),
		IM_COL32(198, 104, 65, 128),
	};

	uint32_t generic_value_color_hovered[(int)GenericValueType::Max] =
	{
		IM_COL32(255, 204, 153, 255),
		IM_COL32(51, 102, 255, 255),
		IM_COL32(51, 204, 204, 255),
		IM_COL32(153, 204, 0, 255),
		IM_COL32(255, 204, 0, 255),
		IM_COL32(200, 104, 0, 255),
		IM_COL32(155, 194, 85, 255),
		IM_COL32(198, 104, 65, 255),
	};

	struct ShaderGraphNodeTypeInfo
	{
		std::string display_name;
		std::vector<std::string> inputs;
		std::vector<std::string> outputs;
		std::function<void(GenericNode* node, GenericGraphEditor* editor)> details_render;
	};

	
	struct AccessHelper
	{
		static std::unordered_map<ShaderNodeType, ShaderGraphNodeTypeInfo> node_type_info;
	};

	std::unordered_map<uint64_t, std::string> _node_names =
	{
		{Reflection::TypeID<FinalPBRNode>(), "PBR Node"},
	};


	std::unordered_map<uint64_t, std::pair<std::vector<std::string>, std::vector<std::string>>> _node_pins_name = //first is name of inputs and second is that of outputs
	{

		{
			Reflection::TypeID<FinalPBRNode>(),
			{
				{"Color", "Normal", "Occlussion", "Metallic", "Roughness", "Emission Color"},
				{""}
			}
		},
		{
			Reflection::TypeID<GetVariableNode>(),
			{
				{"None"},
				{""}
			}
		}

	};

	std::unordered_map<uint64_t, std::function<void(GenericNode* node, int32_t node_index, GenericGraphEditor* editor)>> node_render = 
	{
		{
			Reflection::TypeID<GetVariableNode>(),
			[](GenericNode* node, int32_t node_index, GenericGraphEditor* editor)
			{
				ImNodes::BeginNode(node_index);

				ImNodes::BeginNodeTitleBar();
				ImGui::Text("Get Parameter");
				ImNodes::EndNodeTitleBar();

				GetVariableNode* _node = (GetVariableNode*)node;
				ShaderGraph* graph = (ShaderGraph*)editor->GetCurrentGraph();
				int32_t var_index = _node->GetVar();
				std::string name = var_index < 0 ? "None(Parameter)" : graph->GetParameters()[var_index].name;
				

				GenericNodeOutput& output_0 = node->GetOutputs()[0];
				ImNodes::PushColorStyle(ImNodesCol_Pin, generic_value_color[(int)output_0.type]);
				ImNodes::PushColorStyle(ImNodesCol_PinHovered, generic_value_color_hovered[(int)output_0.type]);
				ImNodes::BeginOutputAttribute(((0 + generic_output_start_index) << shift_amount) | node_index, ImNodesPinShape_Quad);
				ImGui::Text(name.c_str());
				ImNodes::EndOutputAttribute();
				ImNodes::PopColorStyle();
				ImNodes::PopColorStyle();

				ImNodes::EndNode();
			}
		},
		{
			Reflection::TypeID<ConstantNode>(),
			[](GenericNode* node, int32_t node_index, GenericGraphEditor* editor)
			{
				ConstantNode* _node = (ConstantNode*)node;
				ShaderGraph* graph = (ShaderGraph*)editor->GetCurrentGraph();

				std::string& node_name = AccessHelper::node_type_info.at(_node->GetType()).display_name;

				ImNodes::BeginNode(node_index);

				ImNodes::BeginNodeTitleBar();
				ImGui::Text(node_name.c_str());
				ImNodes::EndNodeTitleBar();

				for (uint32_t j = 0; j < node->GetInputs().size(); j++)
				{
					GenericNodeInput& input_0 = node->GetInputs()[j];
					ImNodes::PushColorStyle(ImNodesCol_Pin, generic_value_color[(int)input_0.type]);
					ImNodes::PushColorStyle(ImNodesCol_PinHovered, generic_value_color_hovered[(int)input_0.type]);
					ImNodes::BeginInputAttribute(((j + 1) << shift_amount) | node_index, ImNodesPinShape_Quad);
					std::string& input_name = AccessHelper::node_type_info.at(_node->GetType()).inputs[j];
					ImGui::Text(input_name.c_str());
					ImNodes::EndInputAttribute();
					ImNodes::PopColorStyle();
					ImNodes::PopColorStyle();
				}

				for (uint32_t j = 0; j < node->GetOutputs().size(); j++)
				{
					GenericNodeOutput& output_0 = node->GetOutputs()[j];
					ImNodes::PushColorStyle(ImNodesCol_Pin, generic_value_color[(int)output_0.type]);
					ImNodes::PushColorStyle(ImNodesCol_PinHovered, generic_value_color_hovered[(int)output_0.type]);
					ImNodes::BeginOutputAttribute(((j + generic_output_start_index) << shift_amount) | node_index, ImNodesPinShape_Quad);
					std::string& output_name = AccessHelper::node_type_info.at(_node->GetType()).outputs[j];
					ImGui::Text(output_name.c_str());
					ImNodes::EndOutputAttribute();
					ImNodes::PopColorStyle();
					ImNodes::PopColorStyle();
				}

				ImNodes::EndNode();
				
			}
		}
	};
	std::unordered_map<uint64_t, std::function<void(GenericNode* node, GenericGraphEditor* editor)>> node_details_render =
	{
		{
			Reflection::TypeID<FinalPBRNode>(),
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				FinalPBRNode* pbr_node = (FinalPBRNode*)node;
				ImGui::ColorEdit3("Color", (float*)pbr_node->color.data);
				ImGui::DragFloat("Occlusion", (float*)pbr_node->occlusion.data, 0.001f, 0.0f, 1.0f);
				ImGui::DragFloat("Metallic", (float*)pbr_node->metallic.data, 0.001f, 0.0f, 1.0f);
				ImGui::DragFloat("Roughness", (float*)pbr_node->roughness.data, 0.001f, 0.0f, 1.0f);
				ImGui::ColorEdit3("Emission Color", (float*)pbr_node->emissionColor.data, ImGuiColorEditFlags_HDR);
			}
		},
		{
			Reflection::TypeID<GetVariableNode>(),
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				GetVariableNode* _node = (GetVariableNode*)node;
				ShaderGraph* graph = (ShaderGraph*)editor->GetCurrentGraph();
				int32_t var_index = _node->GetVar();
				std::string name = var_index < 0 ? "None(Parameter)" : graph->GetParameters()[var_index].name;


				ImGui::Text("Param Name: ");
				ImGui::SameLine();
				if (ImGui::Button(name.c_str()))
				{
					ImGui::OpenPopup("Parameters Drop Down");
				}
				
				int32_t new_index = editor->paramters_drop_down(var_index);
				if (new_index >= 0 && new_index != var_index)
				{
					GenericParameter& new_param = graph->GetParameters()[new_index];
					_node->SetVar(new_index);
					_node->GetOutputs()[0].type = new_param.type;
					
				}


			}
		},
		{
			Reflection::TypeID<ConstantNode>(),
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				ConstantNode* _node = (ConstantNode*)node;
				ShaderGraph* graph = (ShaderGraph*)editor->GetCurrentGraph();
				
				AccessHelper::node_type_info.at(_node->GetType()).details_render(node, editor);

			}
		}
	};
	

	

	std::unordered_map<uint64_t, std::function<void(GenericGraph* graph, GenericGraphEditor* editor)>> graph_pop_up_render =
	{
		{
			Reflection::TypeID<ShaderGraph>(),
			[](GenericGraph* graph, GenericGraphEditor* editor)
			{
				if (ImGui::MenuItem("Get Parameter"))
				{
					UUID new_node = graph->CreateNode<GetVariableNode>();
					editor->add_new_node(new_node);
				}

				for (auto& i : AccessHelper::node_type_info)
				{
					if (ImGui::MenuItem(i.second.display_name.c_str()))
					{
						UUID new_node = graph->CreateNode<ConstantNode>();
						ConstantNode* _node = (ConstantNode*)graph->GetNode(new_node);

						_node->SetType(i.first);

						editor->add_new_node(new_node);
					}
				}
			}
		}
	};

	std::unordered_map<ShaderNodeType, ShaderGraphNodeTypeInfo> AccessHelper::node_type_info =
	{
		{
			ShaderNodeType::Float_Constant,
			{
			"Float",
			{},
			{"Value"},
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				ConstantNode* _node = (ConstantNode*)node;
				ImGui::DragFloat("Value", (float*)&_node->GetDefaultParameterData()[0], 0.001f);
			}
			}
		},
		{
			ShaderNodeType::Int_Constant,
			{
			"Int",
			{},
			{"Value"},
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				ConstantNode* _node = (ConstantNode*)node;
				ImGui::DragInt("Value", (int*)&_node->GetDefaultParameterData()[0], 0.45f);
			}
			}
		},
		{
			ShaderNodeType::Vec2_Constant,
			{
			"Vec2",
			{"X", "Y"},
			{"Value"},
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				ConstantNode* _node = (ConstantNode*)node;
				ImGui::DragFloat("X", (float*)&_node->GetDefaultParameterData()[0], 0.001f);
				ImGui::DragFloat("Y", (float*)&_node->GetDefaultParameterData()[1], 0.001f);
			}
			}
		},
		{
			ShaderNodeType::Vec3_Constant,
			{
			"Vec3",
			{"X", "Y", "Z"},
			{"Value"},
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				ConstantNode* _node = (ConstantNode*)node;
				ImGui::DragFloat("X", (float*)&_node->GetDefaultParameterData()[0], 0.001f);
				ImGui::DragFloat("Y", (float*)&_node->GetDefaultParameterData()[1], 0.001f);
				ImGui::DragFloat("Z", (float*)&_node->GetDefaultParameterData()[2], 0.001f);
			}
			}
		},
		{
			ShaderNodeType::Vec4_Constant,
			{
			"Vec4",
			{"X", "Y", "Z", "W"},
			{"Value"},
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				ConstantNode* _node = (ConstantNode*)node;
				ImGui::DragFloat("X", (float*)&_node->GetDefaultParameterData()[0], 0.001f);
				ImGui::DragFloat("Y", (float*)&_node->GetDefaultParameterData()[1], 0.001f);
				ImGui::DragFloat("Z", (float*)&_node->GetDefaultParameterData()[2], 0.001f);
				ImGui::DragFloat("W", (float*)&_node->GetDefaultParameterData()[3], 0.001f);
			}
			}
		},
		{
			ShaderNodeType::Sample_Texture_Function,
			{
			"Sample Texture",
			{"Texture", "UV"},
			{"RGBA", "RGB", "R", "G", "B", "A"},
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				ConstantNode* _node = (ConstantNode*)node;
				ImGui::DragFloat2("UV", (float*)&_node->GetDefaultParameterData()[1], 0.001f);
			}
			}
		},
		{
			ShaderNodeType::Normal_Texture_To_World_Function,
			{
			"Normal Texture To World",
			{"Normal", "Tangent", "Texture Normal"},
			{"Value"},
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				ConstantNode* _node = (ConstantNode*)node;
			}
			}
		},
		{
			ShaderNodeType::Frensel_Function,
			{
			"Frensel",
			{"View Direction", "Normal", "Power"},
			{"Value"},
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				ConstantNode* _node = (ConstantNode*)node;
				ImGui::DragFloat("Power", (float*)&_node->GetDefaultParameterData()[2], 0.001f);
			}
			}
		},
		{
			ShaderNodeType::Multiply_Vec4_Function,
			{
			"Multiply Vec4",
			{"A", "B"},
			{"Value"},
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				ConstantNode* _node = (ConstantNode*)node;
				ImGui::DragFloat4("A", (float*)&_node->GetDefaultParameterData()[0], 0.001f);
				ImGui::DragFloat4("B", (float*)&_node->GetDefaultParameterData()[1], 0.001f);
			}
			}
		},
		{
			ShaderNodeType::Multiply_Vec4_Float_Function,
			{
			"Multiply Vec4_Float",
			{"A", "B"},
			{"Value"},
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				ConstantNode* _node = (ConstantNode*)node;
				ImGui::DragFloat4("A", (float*)&_node->GetDefaultParameterData()[0], 0.001f);
				ImGui::DragFloat("B", (float*)&_node->GetDefaultParameterData()[1], 0.001f);
			}
			}
		},
		{
			ShaderNodeType::Multiply_Vec3_Function,
			{
			"Multiply Vec3",
			{"A", "B"},
			{"Value"},
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				ConstantNode* _node = (ConstantNode*)node;
				ImGui::DragFloat3("A", (float*)&_node->GetDefaultParameterData()[0], 0.001f);
				ImGui::DragFloat3("B", (float*)&_node->GetDefaultParameterData()[1], 0.001f);
			}
			}
		},
		{
			ShaderNodeType::Multiply_Vec3_Float_Function,
			{
			"Multiply Vec3_Float",
			{"A", "B"},
			{"Value"},
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				ConstantNode* _node = (ConstantNode*)node;
				ImGui::DragFloat3("A", (float*)&_node->GetDefaultParameterData()[0], 0.001f);
				ImGui::DragFloat("B", (float*)&_node->GetDefaultParameterData()[1], 0.001f);
			}
			}
		},
		{
			ShaderNodeType::Multiply_Vec2_Function,
			{
			"Multiply Vec2",
			{"A", "B"},
			{"Value"},
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				ConstantNode* _node = (ConstantNode*)node;
				ImGui::DragFloat2("A", (float*)&_node->GetDefaultParameterData()[0], 0.001f);
				ImGui::DragFloat2("B", (float*)&_node->GetDefaultParameterData()[1], 0.001f);
			}
			}
		},
		{
			ShaderNodeType::Multiply_Vec2_Float_Function,
			{
			"Multiply Vec2_Float",
			{"A", "B"},
			{"Value"},
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				ConstantNode* _node = (ConstantNode*)node;
				ImGui::DragFloat2("A", (float*)&_node->GetDefaultParameterData()[0], 0.001f);
				ImGui::DragFloat("B", (float*)&_node->GetDefaultParameterData()[1], 0.001f);
			}
			}
		},
		{
			ShaderNodeType::Multiply_Float_Function,
			{
			"Multiply Float",
			{"A", "B"},
			{"Value"},
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				ConstantNode* _node = (ConstantNode*)node;
				ImGui::DragFloat("A", (float*)&_node->GetDefaultParameterData()[0], 0.001f);
				ImGui::DragFloat("B", (float*)&_node->GetDefaultParameterData()[1], 0.001f);
			}
			}
		},
		{
			ShaderNodeType::Add_Vec4_Function,
			{
			"Add Vec4",
			{"A", "B"},
			{"Value"},
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				ConstantNode* _node = (ConstantNode*)node;
				ImGui::DragFloat4("A", (float*)&_node->GetDefaultParameterData()[0], 0.001f);
				ImGui::DragFloat4("B", (float*)&_node->GetDefaultParameterData()[1], 0.001f);
			}
			}
		},
		{
			ShaderNodeType::Add_Vec4_Float_Function,
			{
			"Add Vec4_Float",
			{"A", "B"},
			{"Value"},
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				ConstantNode* _node = (ConstantNode*)node;
				ImGui::DragFloat4("A", (float*)&_node->GetDefaultParameterData()[0], 0.001f);
				ImGui::DragFloat("B", (float*)&_node->GetDefaultParameterData()[1], 0.001f);
			}
			}
		},
		{
			ShaderNodeType::Add_Vec3_Function,
			{
			"Add Vec3",
			{"A", "B"},
			{"Value"},
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				ConstantNode* _node = (ConstantNode*)node;
				ImGui::DragFloat3("A", (float*)&_node->GetDefaultParameterData()[0], 0.001f);
				ImGui::DragFloat3("B", (float*)&_node->GetDefaultParameterData()[1], 0.001f);
			}
			}
		},
		{
			ShaderNodeType::Add_Vec3_Float_Function,
			{
			"Add Vec3_Float",
			{"A", "B"},
			{"Value"},
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				ConstantNode* _node = (ConstantNode*)node;
				ImGui::DragFloat3("A", (float*)&_node->GetDefaultParameterData()[0], 0.001f);
				ImGui::DragFloat("B", (float*)&_node->GetDefaultParameterData()[1], 0.001f);
			}
			}
		},
		{
			ShaderNodeType::Add_Vec2_Function,
			{
			"Add Vec2",
			{"A", "B"},
			{"Value"},
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				ConstantNode* _node = (ConstantNode*)node;
				ImGui::DragFloat2("A", (float*)&_node->GetDefaultParameterData()[0], 0.001f);
				ImGui::DragFloat2("B", (float*)&_node->GetDefaultParameterData()[1], 0.001f);
			}
			}
		},
		{
			ShaderNodeType::Add_Vec2_Float_Function,
			{
			"Add Vec2_Float",
			{"A", "B"},
			{"Value"},
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				ConstantNode* _node = (ConstantNode*)node;
				ImGui::DragFloat2("A", (float*)&_node->GetDefaultParameterData()[0], 0.001f);
				ImGui::DragFloat("B", (float*)&_node->GetDefaultParameterData()[1], 0.001f);
			}
			}
		},
		{
			ShaderNodeType::Add_Float_Function,
			{
			"Multiply Float",
			{"A", "B"},
			{"Value"},
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				ConstantNode* _node = (ConstantNode*)node;
				ImGui::DragFloat("A", (float*)&_node->GetDefaultParameterData()[0], 0.001f);
				ImGui::DragFloat("B", (float*)&_node->GetDefaultParameterData()[1], 0.001f);
			}
			}
		},
		{
			ShaderNodeType::Object_UV_Variable,
			{
			"Object UV",
			{},
			{"Value"},
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				ConstantNode* _node = (ConstantNode*)node;
			}
			}
		},
		{
			ShaderNodeType::Object_Normal_Variable,
			{
			"Object Normal",
			{},
			{"Value"},
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				ConstantNode* _node = (ConstantNode*)node;
			}
			}
		},
		{
			ShaderNodeType::Object_Tangent_Variable,
			{
			"Object Tangent",
			{},
			{"Value"},
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				ConstantNode* _node = (ConstantNode*)node;
			}
			}
		},
		{
			ShaderNodeType::View_Direction_Variable,
			{
			"View Direction",
			{},
			{"Value"},
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				ConstantNode* _node = (ConstantNode*)node;
			}
			}
		},
		{
			ShaderNodeType::Split_Vec4_Helper,
			{
			"Split Vec4",
			{"In"},
			{"XYZ", "XY", "X", "Y", "Z", "W"},
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				ConstantNode* _node = (ConstantNode*)node;
			}
			}
		},
		{
			ShaderNodeType::Split_Vec3_Helper,
			{
			"Split Vec3",
			{"In"},
			{"XY", "X", "Y", "Z"},
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				ConstantNode* _node = (ConstantNode*)node;
			}
			}
		},
		{
			ShaderNodeType::Split_Vec2_Helper,
			{
			"Split Vec2",
			{"In"},
			{"X", "Y"},
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				ConstantNode* _node = (ConstantNode*)node;
			}
			}
		},
		{
			ShaderNodeType::Transparent_UnLit_Node,
			{
			"Transparent UnLit",
			{"Color", "Alpha"},
			{},
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				ConstantNode* _node = (ConstantNode*)node;
				ImGui::ColorEdit3("Color", (float*)&_node->GetDefaultParameterData()[0], ImGuiColorEditFlags_HDR);
				ImGui::DragFloat("Alpha", (float*)&_node->GetDefaultParameterData()[1], 0.001f);
			}
			}
		},
		{
			ShaderNodeType::Screen_Color_Variable,
			{
			"Screen Color",
			{},
			{"Value"},
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				ConstantNode* _node = (ConstantNode*)node;
				
			}
			}
		},
		{
			ShaderNodeType::Simple_Noise_Function,
			{
			"Simple Noise",
			{"UV", "Scale"},
			{"Value"},
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				ConstantNode* _node = (ConstantNode*)node;
				ImGui::DragFloat2("UV", (float*)&_node->GetDefaultParameterData()[0], 0.001f);
				ImGui::DragFloat("Scale", (float*)&_node->GetDefaultParameterData()[1], 0.001f);
			}
			}
		},
		{
			ShaderNodeType::Gradient_Noise_Function,
			{
			"Gradient Noise",
			{"UV", "Scale"},
			{"Value"},
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				ConstantNode* _node = (ConstantNode*)node;
				ImGui::DragFloat2("UV", (float*)&_node->GetDefaultParameterData()[0], 0.001f);
				ImGui::DragFloat("Scale", (float*)&_node->GetDefaultParameterData()[1], 0.001f);
			}
			}
		},
		{
			ShaderNodeType::Twist_Function,
			{
			"Twist",
			{"UV", "Center", "Strength", "Offset"},
			{"Value"},
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				ConstantNode* _node = (ConstantNode*)node;
				ImGui::DragFloat2("UV", (float*)&_node->GetDefaultParameterData()[0], 0.001f);
				ImGui::DragFloat2("Center", (float*)&_node->GetDefaultParameterData()[1], 0.001f);
				ImGui::DragFloat("Strength", (float*)&_node->GetDefaultParameterData()[2], 0.001f);
				ImGui::DragFloat2("Offset", (float*)&_node->GetDefaultParameterData()[3], 0.001f);
			}
			}
		},
		{
			ShaderNodeType::Normal_From_Gradient_Noise,
			{
			"Normal From Gradient Noise",
			{"UV", "Strength", "Texel Size", "Scale"},
			{"Value"},
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				ConstantNode* _node = (ConstantNode*)node;
				ImGui::DragFloat2("UV", (float*)&_node->GetDefaultParameterData()[0], 0.001f);
				ImGui::DragFloat("Strength", (float*)&_node->GetDefaultParameterData()[1], 0.001f);
				ImGui::DragFloat("Texel Size", (float*)&_node->GetDefaultParameterData()[2], 0.001f);
				ImGui::DragFloat("Scale", (float*)&_node->GetDefaultParameterData()[3], 0.001f);
			}
			}
		},
		{
			ShaderNodeType::Time_Variable,
			{
			"Time",
			{},
			{"Value"},
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				ConstantNode* _node = (ConstantNode*)node;
			}
			}
		},
		{
			ShaderNodeType::Sin_Float,
			{
			"Sin Float",
			{"In"},
			{"Value"},
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				ConstantNode* _node = (ConstantNode*)node;
			}
			}
		},
		{
			ShaderNodeType::Radians_Float,
			{
			"Radians Float",
			{"In"},
			{"Value"},
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				ConstantNode* _node = (ConstantNode*)node;
			}
			}
		},
		{
			ShaderNodeType::Fract_Float,
			{
			"Fract Float",
			{"In"},
			{"Value"},
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				ConstantNode* _node = (ConstantNode*)node;
			}
			}
		},
		{
			ShaderNodeType::Screen_UV_Variable,
			{
			"Screen UV",
			{},
			{"Value"},
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				ConstantNode* _node = (ConstantNode*)node;

			}
			}
		}
	};


}
