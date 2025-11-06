

#include "node_system/GenericGraph.h"
#include "shader_graph/ShaderGraphNode.h"
#include "shader_graph/ShaderGraph.h"
#include "../panels/GenericGraphEditor.h"
#include "particle_effects/ParticleEffect.h"
#include "particle_effects/ParticleGenerator.h"
#include "particle_effects/effects_graph/ParticleEffectsNode.h"
#include "particle_effects/particle_renderers/BillboardRender.h"

#include "imgui.h"
#include "imgui_stdlib.h"
#include "imnodes/imnodes.h"

namespace trace {

	std::string get_class_display_name(uint64_t class_id);

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
		IM_COL32(0, 194, 65, 128),
		IM_COL32(10, 94, 165, 128),
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
		IM_COL32(0, 194, 65, 255),
		IM_COL32(10, 94, 165, 255),
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
		static std::unordered_map<EffectsNodeType, ShaderGraphNodeTypeInfo> effect_node_type_info;
	};

	std::unordered_map<uint64_t, std::string> _node_names =
	{
		{Reflection::TypeID<FinalPBRNode>(), "PBR Node"},
		{Reflection::TypeID<EffectsRootNode>(), "Effects Node"},
		{Reflection::TypeID<GetParticleAttributeNode>(), "Get Particle Attribute"},
		{Reflection::TypeID<SetParticleAttributeNode>(), "Set Particle Attribute"},
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
		},
		{
			Reflection::TypeID<EffectsRootNode>(),
			[](GenericNode* node, int32_t node_index, GenericGraphEditor* editor)
			{
				EffectsRootNode* _node = (EffectsRootNode*)node;
				ParticleGenerator* generator = (ParticleGenerator*)editor->GetCurrentGraph();

				std::string node_name = "";

				ImNodes::BeginNode(node_index);

				ImNodes::BeginNodeTitleBar();
				ImGui::Text(node_name.c_str());
				ImNodes::EndNodeTitleBar();

				ImGui::Text("Spawner: ");
				ParticleSpawner* spawner = generator->GetSpawner();
				std::string spawner_name = spawner ? get_class_display_name(spawner->GetTypeID()) : "None";
				ImGui::SameLine();
				if (ImGui::Button(spawner_name.c_str()))
				{
					editor->SetUserData(spawner);
					ImGui::OpenPopup("Spawner Select Popup");
				}

				if (ImGui::BeginPopup("Spawner Select Popup"))
				{
					if (ImGui::MenuItem("Rate Spawner"))
					{

						if (spawner)
						{
							delete spawner;//TODO: Use custom allocator
						}

						

						RateSpawner* rate_spawner = new RateSpawner;//TODO: Use custom allocator

						generator->SetSpawner(rate_spawner);
						if (spawner == editor->GetUserData())
						{
							editor->SetUserData(rate_spawner);
						}

						spawner = rate_spawner;
					}


					ImGui::EndPopup();
				}

				ImGui::Dummy(ImVec2(0.0f, 7.0f));

				ImGui::Text("Initializers: ");
				
				auto& initializers = generator->GetInitializers();

				int remove_index = -1;
				int index = -1;
				for (ParticleInitializer* init : initializers)
				{
					++index;

					ImGui::PushID(index);
					std::string display_name = get_class_display_name(init->GetTypeID());
					if (init->GetTypeID() == Reflection::TypeID<CustomParticleInitializer>())
					{
						CustomParticleInitializer* custom_init = (CustomParticleInitializer*)init;
						GenericNode* graph_node = generator->GetNode(custom_init->GetNodeID());
						int32_t graph_node_index = editor->get_node_index(graph_node->GetUUID());

						std::string& node_name = STRING_FROM_ID(custom_init->GetName());

						display_name = node_name.empty() ? display_name : node_name;

						GenericNodeInput& input_0 = graph_node->GetInputs()[0];
						ImNodes::PushColorStyle(ImNodesCol_Pin, generic_value_color[(int)input_0.type]);
						ImNodes::PushColorStyle(ImNodesCol_PinHovered, generic_value_color_hovered[(int)input_0.type]);
						ImNodes::BeginInputAttribute(((0 + 1) << shift_amount) | graph_node_index, ImNodesPinShape_Quad);
						
						if (ImGui::Button(display_name.c_str()))
						{
							editor->SetUserData(init);
						}

						ImNodes::EndInputAttribute();
						ImNodes::PopColorStyle();
						ImNodes::PopColorStyle();
					}
					else
					{

						if (ImGui::Button(display_name.c_str()))
						{
							editor->SetUserData(init);
						}



					}

					if (ImGui::BeginPopupContextItem("Initializer_Item_Popup"))
					{

						if (ImGui::MenuItem("Delete"))
						{
							remove_index = index;
							void* user_data = editor->GetUserData();
							if (user_data == init)
							{
								editor->SetUserData(nullptr);
							}

							if (init->GetTypeID() == Reflection::TypeID<CustomParticleInitializer>())
							{
								GenericNode* root_node = generator->GetNode(generator->GetEffectRoot());
								std::vector<GenericNodeInput>& root_inputs = root_node->GetInputs();
								UUID node_id = ((CustomParticleInitializer*)init)->GetNodeID();

								for (GenericNodeInput& in : root_inputs)
								{
									if (in.node_id == node_id)
									{
										std::swap(in, root_inputs.back());
										break;
									}
								}
								root_inputs.pop_back();

								generator->DestroyNode(node_id);
								editor->generate_current_node_links();
							}

						}

						ImGui::EndPopup();
					}
					ImGui::PopID();
				}

				//Delete 
				if (remove_index > -1)
				{
					ParticleBase* particle_base = initializers[remove_index];
					
					initializers.erase(initializers.begin() + remove_index);

					delete particle_base;//TODO: Use custom allocator
				}


				if (ImGui::Button(" + ###Initializer_Add"))
				{
					ImGui::OpenPopup("Initializer Select Popup");
				}

				if (ImGui::BeginPopup("Initializer Select Popup"))
				{
					if (ImGui::MenuItem("Velocity Initializer"))
					{
						initializers.push_back(new VelocityInitializer);
					}
					if (ImGui::MenuItem("Lifetime Initializer"))
					{
						initializers.push_back(new LifetimeInitializer);
					}
					if (ImGui::MenuItem("Custom Initializer"))
					{
						CustomParticleInitializer* new_init = new CustomParticleInitializer;//TODO: Use custom allocator
						UUID new_node = generator->CreateNode<EffectsFinalNode>();
						new_init->SetNodeID(new_node);

						GenericNodeInput input = {};
						input.node_id = new_node;
						input.type = GenericValueType::Execute;
						input.value_index = INVALID_ID;

						GenericNode* root_node = generator->GetNode(generator->GetEffectRoot());
						root_node->GetInputs().push_back(input);

						initializers.push_back(new_init);

						editor->add_new_node_not_child(new_node);
					}



					ImGui::EndPopup();
				}

				ImGui::Dummy(ImVec2(0.0f, 7.0f));

				ImGui::Text("Updates: ");

				remove_index = -1;
				index = -1;
				
				auto& updates = generator->GetUpdates();

				for (ParticleUpdate* update : updates)
				{
					++index;

					ImGui::PushID(index);
					std::string display_name = get_class_display_name(update->GetTypeID());
					if (update->GetTypeID() == Reflection::TypeID<CustomParticleUpdate>())
					{
						CustomParticleUpdate* custom_update = (CustomParticleUpdate*)update;
						GenericNode* graph_node = generator->GetNode(custom_update->GetNodeID());
						int32_t graph_node_index = editor->get_node_index(graph_node->GetUUID());

						std::string& node_name = STRING_FROM_ID(custom_update->GetName());

						display_name = node_name.empty() ? display_name : node_name;

						GenericNodeInput& input_0 = graph_node->GetInputs()[0];
						ImNodes::PushColorStyle(ImNodesCol_Pin, generic_value_color[(int)input_0.type]);
						ImNodes::PushColorStyle(ImNodesCol_PinHovered, generic_value_color_hovered[(int)input_0.type]);
						ImNodes::BeginInputAttribute(((0 + 1) << shift_amount) | graph_node_index, ImNodesPinShape_Quad);

						if (ImGui::Button(display_name.c_str()))
						{
							editor->SetUserData(update);
						}

						ImNodes::EndInputAttribute();
						ImNodes::PopColorStyle();
						ImNodes::PopColorStyle();
					}
					else
					{

						if (ImGui::Button(display_name.c_str()))
						{
							editor->SetUserData(update);
						}



					}


					if (ImGui::BeginPopupContextItem("Update_Item_Popup"))
					{

						if (ImGui::MenuItem("Delete"))
						{
							remove_index = index;
							void* user_data = editor->GetUserData();
							if (user_data == update)
							{
								editor->SetUserData(nullptr);
							}

							if (update->GetTypeID() == Reflection::TypeID<CustomParticleUpdate>())
							{
								GenericNode* root_node = generator->GetNode(generator->GetEffectRoot());
								std::vector<GenericNodeInput>& root_inputs = root_node->GetInputs();
								UUID node_id = ((CustomParticleInitializer*)update)->GetNodeID();

								for (GenericNodeInput& in : root_inputs)
								{
									if (in.node_id == node_id)
									{
										std::swap(in, root_inputs.back());
										break;
									}
								}
								root_inputs.pop_back();

								generator->DestroyNode(node_id);
								editor->generate_current_node_links();
							}

						}

						ImGui::EndPopup();
					}
					ImGui::PopID();
					

				}

				//Delete 
				if (remove_index > -1)
				{
					ParticleBase* particle_base = updates[remove_index];

					updates.erase(updates.begin() + remove_index);

					delete particle_base;//TODO: Use custom allocator
				}

				if (ImGui::Button(" + ###Update_Add"))
				{
					ImGui::OpenPopup("Update Select Popup");
				}

				if (ImGui::BeginPopup("Update Select Popup"))
				{
					if (ImGui::MenuItem("Gravity Update"))
					{
						updates.push_back(new GravityUpdate);
					}
					if (ImGui::MenuItem("Drag Update"))
					{
						updates.push_back(new DragUpdate);
					}
					if (ImGui::MenuItem("Wind Update"))
					{
						updates.push_back(new WindUpdate);
					}
					if (ImGui::MenuItem("Velocity Update"))
					{
						updates.push_back(new VelocityUpdate);
					}
					if (ImGui::MenuItem("Custom Update"))
					{
						CustomParticleUpdate* new_update = new CustomParticleUpdate;//TODO: Use custom allocator
						UUID new_node = generator->CreateNode<EffectsFinalNode>();
						new_update->SetNodeID(new_node);

						GenericNodeInput input = {};
						input.node_id = new_node;
						input.type = GenericValueType::Execute;
						input.value_index = INVALID_ID;

						GenericNode* root_node = generator->GetNode(generator->GetEffectRoot());
						root_node->GetInputs().push_back(input);

						updates.push_back(new_update);

						editor->add_new_node_not_child(new_node);
					}


					ImGui::EndPopup();
				}

				ImGui::Dummy(ImVec2(0.0f, 7.0f));

				ImGui::Text("Renderers: ");

				remove_index = -1;
				index = -1;

				auto& renderers = generator->GetRenderers();

				for (ParticleRender* renderer : renderers)
				{
					++index;

					ImGui::PushID(index);
					if (renderer->GetTypeID() == Reflection::TypeID<CustomParticleInitializer>())
					{
						continue;
					}

					std::string display_name = get_class_display_name(renderer->GetTypeID());

					if (ImGui::Button(display_name.c_str()))
					{
						editor->SetUserData(renderer);
					}

					if (ImGui::BeginPopupContextItem("Renderer_Item_Popup"))
					{

						if (ImGui::MenuItem("Delete"))
						{
							remove_index = index;
							void* user_data = editor->GetUserData();
							if (user_data == renderer)
							{
								editor->SetUserData(nullptr);
							}

							if (renderer->GetTypeID() == Reflection::TypeID<CustomParticleInitializer>())
							{
								GenericNode* root_node = generator->GetNode(generator->GetEffectRoot());
								std::vector<GenericNodeInput>& root_inputs = root_node->GetInputs();
								UUID node_id = ((CustomParticleInitializer*)renderer)->GetNodeID();

								for (GenericNodeInput& in : root_inputs)
								{
									if (in.node_id == node_id)
									{
										std::swap(in, root_inputs.back());
										break;
									}
								}
								root_inputs.pop_back();

								generator->DestroyNode(node_id);
								editor->generate_current_node_links();
							}

						}

						ImGui::EndPopup();
					}

					ImGui::PopID();
				}

				//Delete 
				if (remove_index > -1)
				{
					ParticleBase* particle_base = renderers[remove_index];

					renderers.erase(renderers.begin() + remove_index);

					delete particle_base;//TODO: Use custom allocator
				}

				if (ImGui::Button(" + ###Renderer_Add"))
				{
					ImGui::OpenPopup("Renderer Select Popup");
				}

				if (ImGui::BeginPopup("Renderer Select Popup"))
				{
					if (ImGui::MenuItem("BillBoard Render"))
					{
						renderers.push_back(new BillBoardRender);
					}


					ImGui::EndPopup();
				}


				ImNodes::EndNode();
				
			}
		},
		{
			Reflection::TypeID<GenericEffectNode>(),
			[](GenericNode* node, int32_t node_index, GenericGraphEditor* editor)
			{
				GenericEffectNode* _node = (GenericEffectNode*)node;
				ParticleGenerator* generator = (ParticleGenerator*)editor->GetCurrentGraph();

				std::string& node_name = AccessHelper::effect_node_type_info.at(_node->GetType()).display_name;

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
					std::string& input_name = AccessHelper::effect_node_type_info.at(_node->GetType()).inputs[j];
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
					std::string& output_name = AccessHelper::effect_node_type_info.at(_node->GetType()).outputs[j];
					ImGui::Text(output_name.c_str());
					ImNodes::EndOutputAttribute();
					ImNodes::PopColorStyle();
					ImNodes::PopColorStyle();
				}

				ImNodes::EndNode();
				
			}
		},
		{
			Reflection::TypeID<GetParticleAttributeNode>(),
			[](GenericNode* node, int32_t node_index, GenericGraphEditor* editor)
			{
				GetParticleAttributeNode* _node = (GetParticleAttributeNode*)node;
				ParticleGenerator* generator = (ParticleGenerator*)editor->GetCurrentGraph();

				

				ImNodes::BeginNode(node_index);

				ImNodes::BeginNodeTitleBar();
				ImGui::Text("Get Attribute");
				ImNodes::EndNodeTitleBar();

				GenericNodeInput& input_0 = node->GetInputs()[0];
				ImNodes::PushColorStyle(ImNodesCol_Pin, generic_value_color[(int)input_0.type]);
				ImNodes::PushColorStyle(ImNodesCol_PinHovered, generic_value_color_hovered[(int)input_0.type]);
				ImNodes::BeginInputAttribute(((0 + 1) << shift_amount) | node_index, ImNodesPinShape_Quad);
				ImGui::Text("<-");
				ImNodes::EndInputAttribute();
				ImNodes::PopColorStyle();
				ImNodes::PopColorStyle();

				GenericNodeOutput& output_0 = node->GetOutputs()[0];
				ImNodes::PushColorStyle(ImNodesCol_Pin, generic_value_color[(int)output_0.type]);
				ImNodes::PushColorStyle(ImNodesCol_PinHovered, generic_value_color_hovered[(int)output_0.type]);
				ImNodes::BeginOutputAttribute(((0 + generic_output_start_index) << shift_amount) | node_index, ImNodesPinShape_Quad);
				ImGui::Text("->");
				ImNodes::EndOutputAttribute();
				ImNodes::PopColorStyle();
				ImNodes::PopColorStyle();

				GenericNodeOutput& output_1 = node->GetOutputs()[1];
				ImNodes::PushColorStyle(ImNodesCol_Pin, generic_value_color[(int)output_1.type]);
				ImNodes::PushColorStyle(ImNodesCol_PinHovered, generic_value_color_hovered[(int)output_1.type]);
				ImNodes::BeginOutputAttribute(((1 + generic_output_start_index) << shift_amount) | node_index, ImNodesPinShape_Quad);
				ImGui::Text("Value");
				ImNodes::EndOutputAttribute();
				ImNodes::PopColorStyle();
				ImNodes::PopColorStyle();

				std::string& attr_name = STRING_FROM_ID(_node->GetAttrID());
				ImGui::Button(attr_name.empty() ? "None(Attribute)" : attr_name.c_str());


				ImNodes::EndNode();
				
			}
		},
		{
			Reflection::TypeID<SetParticleAttributeNode>(),
			[](GenericNode* node, int32_t node_index, GenericGraphEditor* editor)
			{
				SetParticleAttributeNode* _node = (SetParticleAttributeNode*)node;
				ParticleGenerator* generator = (ParticleGenerator*)editor->GetCurrentGraph();

				

				ImNodes::BeginNode(node_index);

				ImNodes::BeginNodeTitleBar();
				ImGui::Text("Set Attribute");
				ImNodes::EndNodeTitleBar();

				GenericNodeInput& input_0 = node->GetInputs()[0];
				ImNodes::PushColorStyle(ImNodesCol_Pin, generic_value_color[(int)input_0.type]);
				ImNodes::PushColorStyle(ImNodesCol_PinHovered, generic_value_color_hovered[(int)input_0.type]);
				ImNodes::BeginInputAttribute(((0 + 1) << shift_amount) | node_index, ImNodesPinShape_Quad);
				ImGui::Text("<-");
				ImNodes::EndInputAttribute();
				ImNodes::PopColorStyle();
				ImNodes::PopColorStyle();

				GenericNodeInput& input_1 = node->GetInputs()[1];
				ImNodes::PushColorStyle(ImNodesCol_Pin, generic_value_color[(int)input_1.type]);
				ImNodes::PushColorStyle(ImNodesCol_PinHovered, generic_value_color_hovered[(int)input_1.type]);
				ImNodes::BeginInputAttribute(((1 + 1) << shift_amount) | node_index, ImNodesPinShape_Quad);
				ImGui::Text("Value");
				ImNodes::EndInputAttribute();
				ImNodes::PopColorStyle();
				ImNodes::PopColorStyle();

				GenericNodeOutput& output_0 = node->GetOutputs()[0];
				ImNodes::PushColorStyle(ImNodesCol_Pin, generic_value_color[(int)output_0.type]);
				ImNodes::PushColorStyle(ImNodesCol_PinHovered, generic_value_color_hovered[(int)output_0.type]);
				ImNodes::BeginOutputAttribute(((0 + generic_output_start_index) << shift_amount) | node_index, ImNodesPinShape_Quad);
				ImGui::Text("->");
				ImNodes::EndOutputAttribute();
				ImNodes::PopColorStyle();
				ImNodes::PopColorStyle();

				std::string& attr_name = STRING_FROM_ID(_node->GetAttrID());
				ImGui::Button(attr_name.empty() ? "None(Attribute)" : attr_name.c_str());


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
		},
		{
			Reflection::TypeID<GenericEffectNode>(),
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				GenericEffectNode* _node = (GenericEffectNode*)node;
				
				AccessHelper::effect_node_type_info.at(_node->GetType()).details_render(node, editor);

			}
		},
		{
			Reflection::TypeID<GetParticleAttributeNode>(),
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				GetParticleAttributeNode* _node = (GetParticleAttributeNode*)node;
				
				std::string attr_name = STRING_FROM_ID(_node->GetAttrID());
				if (ImGui::InputText("Attribute Name", &attr_name) && !attr_name.empty())
				{
					_node->SetAttrID(STR_ID(attr_name));
				}

			}
		},
		{
			Reflection::TypeID<SetParticleAttributeNode>(),
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				SetParticleAttributeNode* _node = (SetParticleAttributeNode*)node;
				
				std::string attr_name = STRING_FROM_ID(_node->GetAttrID());
				if (ImGui::InputText("Attribute Name", &attr_name) && !attr_name.empty())
				{
					_node->SetAttrID(STR_ID(attr_name));
				}

				glm::vec4 value = _node->GetDefaultValue();
				if (ImGui::DragFloat4("Default", (float*)&value, 0.001f))
				{
					_node->SetDefaultValue(value);
				}

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
		},
		{
			Reflection::TypeID<ParticleGenerator>(),
			[](GenericGraph* graph, GenericGraphEditor* editor)
			{
				if (ImGui::MenuItem("Get Attribute"))
				{
					UUID new_node = graph->CreateNode<GetParticleAttributeNode>();
					editor->add_new_node(new_node);
				}
				
				if (ImGui::MenuItem("Get Position"))
				{
					UUID new_node = graph->CreateNode<GetParticleAttributeNode>();
					GetParticleAttributeNode* _node = (GetParticleAttributeNode*)graph->GetNode(new_node);
					_node->SetAttrID(STR_ID("position"));
					editor->add_new_node(new_node);
				}
				
				if (ImGui::MenuItem("Get Velocity"))
				{
					UUID new_node = graph->CreateNode<GetParticleAttributeNode>();
					GetParticleAttributeNode* _node = (GetParticleAttributeNode*)graph->GetNode(new_node);
					_node->SetAttrID(STR_ID("velocity"));
					editor->add_new_node(new_node);
				}
				
				if (ImGui::MenuItem("Set Attribute"))
				{
					UUID new_node = graph->CreateNode<SetParticleAttributeNode>();
					editor->add_new_node(new_node);
				}
				
				if (ImGui::MenuItem("Set Position"))
				{
					UUID new_node = graph->CreateNode<SetParticleAttributeNode>();
					SetParticleAttributeNode* _node = (SetParticleAttributeNode*)graph->GetNode(new_node);
					_node->SetAttrID(STR_ID("position"));
					editor->add_new_node(new_node);
				}


				if (ImGui::MenuItem("Set Velocity"))
				{
					UUID new_node = graph->CreateNode<SetParticleAttributeNode>();
					SetParticleAttributeNode* _node = (SetParticleAttributeNode*)graph->GetNode(new_node);
					_node->SetAttrID(STR_ID("velocity"));
					editor->add_new_node(new_node);
				}

				for (auto& i : AccessHelper::effect_node_type_info)
				{
					if (ImGui::MenuItem(i.second.display_name.c_str()))
					{
						UUID new_node = graph->CreateNode<GenericEffectNode>();
						GenericEffectNode* _node = (GenericEffectNode*)graph->GetNode(new_node);

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
			"Add Float",
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
		},
		{
			ShaderNodeType::Subtract_Vec4_Function,
			{
			"Subtract Vec4",
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
			ShaderNodeType::Subtract_Vec4_Float_Function,
			{
			"Subtract Vec4_Float",
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
			ShaderNodeType::Subtract_Vec3_Function,
			{
			"Subtract Vec3",
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
			ShaderNodeType::Subtract_Vec3_Float_Function,
			{
			"Subtract Vec3_Float",
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
			ShaderNodeType::Subtract_Vec2_Function,
			{
			"Subtract Vec2",
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
			ShaderNodeType::Subtract_Vec2_Float_Function,
			{
			"Subtract Vec2_Float",
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
			ShaderNodeType::Subtract_Float_Function,
			{
			"Subtract Float",
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
			ShaderNodeType::Lerp_Float_Function,
			{
			"Lerp Float",
			{"A", "B", "t"},
			{"Value"},
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				ConstantNode* _node = (ConstantNode*)node;
				ImGui::DragFloat("A", (float*)&_node->GetDefaultParameterData()[0], 0.001f);
				ImGui::DragFloat("B", (float*)&_node->GetDefaultParameterData()[1], 0.001f);
				ImGui::DragFloat("t", (float*)&_node->GetDefaultParameterData()[2], 0.001f);
			}
			}
		},
		{
			ShaderNodeType::Lerp_Vec4_Function,
			{
			"Lerp Vec4",
			{"A", "B", "t"},
			{"Value"},
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				ConstantNode* _node = (ConstantNode*)node;
				ImGui::DragFloat4("A", (float*)&_node->GetDefaultParameterData()[0], 0.001f);
				ImGui::DragFloat4("B", (float*)&_node->GetDefaultParameterData()[1], 0.001f);
				ImGui::DragFloat("t", (float*)&_node->GetDefaultParameterData()[2], 0.001f);
			}
			}
		},
		{
			ShaderNodeType::Lerp_Vec3_Function,
			{
			"Lerp Vec3",
			{"A", "B", "t"},
			{"Value"},
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				ConstantNode* _node = (ConstantNode*)node;
				ImGui::DragFloat3("A", (float*)&_node->GetDefaultParameterData()[0], 0.001f);
				ImGui::DragFloat3("B", (float*)&_node->GetDefaultParameterData()[1], 0.001f);
				ImGui::DragFloat("t", (float*)&_node->GetDefaultParameterData()[2], 0.001f);
			}
			}
		},
		{
			ShaderNodeType::Lerp_Vec2_Function,
			{
			"Lerp Vec2",
			{"A", "B", "t"},
			{"Value"},
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				ConstantNode* _node = (ConstantNode*)node;
				ImGui::DragFloat2("A", (float*)&_node->GetDefaultParameterData()[0], 0.001f);
				ImGui::DragFloat2("B", (float*)&_node->GetDefaultParameterData()[1], 0.001f);
				ImGui::DragFloat("t", (float*)&_node->GetDefaultParameterData()[2], 0.001f);
			}
			}
		},
		{
			ShaderNodeType::Particle_Billboard_Node,
			{
			"Particle Billboard",
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
			ShaderNodeType::Particle_Position_Variable,
			{
			"Particle Position",
			{},
			{"Value"},
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				ConstantNode* _node = (ConstantNode*)node;
			}
			}
		},
		{
			ShaderNodeType::Particle_Scale_Variable,
			{
			"Particle Scale",
			{},
			{"Value"},
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				ConstantNode* _node = (ConstantNode*)node;
			}
			}
		},
		{
			ShaderNodeType::Particle_Color_Variable,
			{
			"Particle Color",
			{},
			{"Value"},
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				ConstantNode* _node = (ConstantNode*)node;
			}
			}
		},
		{
			ShaderNodeType::Particle_Lifetime_Variable,
			{
			"Particle Lifetime",
			{},
			{"Value"},
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				ConstantNode* _node = (ConstantNode*)node;
			}
			}
		},
		{
			ShaderNodeType::Power_Float_Function,
			{
			"Pow Float",
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
			ShaderNodeType::Particle_Percentage_Life_Variable,
			{
			"Particle Percentage_Life",
			{},
			{"Value"},
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				ConstantNode* _node = (ConstantNode*)node;
			}
			}
		},
		{
			ShaderNodeType::Rotate_Point_Function,
			{
			"Rotate Point",
			{"Point", "Center", "Angle"},
			{"Value"},
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				ConstantNode* _node = (ConstantNode*)node;
				ImGui::DragFloat2("Point", (float*)&_node->GetDefaultParameterData()[0], 0.001f);
				ImGui::DragFloat2("Center", (float*)&_node->GetDefaultParameterData()[1], 0.001f);
				ImGui::DragFloat("Angle", (float*)&_node->GetDefaultParameterData()[2], 0.001f);
			}
			}
		},
	};

	std::unordered_map<EffectsNodeType, ShaderGraphNodeTypeInfo> AccessHelper::effect_node_type_info =
	{
		{
			EffectsNodeType::Split_Vec4,
			{
			"Split Vec4",
			{ "<-", "In"},
			{"->", "XYZ", "XY", "X", "Y", "Z", "W"},
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				GenericEffectNode* _node = (GenericEffectNode*)node;
				ImGui::DragFloat4("Default", (float*)&_node->GetNodeValues()[0], 0.001f);
			}
			}
		},
		{
			EffectsNodeType::Vec4_To_Quat,
			{
			"Vec4 To Quat",
			{ "<-", "In"},
			{"->", "Out"},
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				GenericEffectNode* _node = (GenericEffectNode*)node;
			}
			}
		},
		{
			EffectsNodeType::Quat_To_Euler,
			{
			"Quat To Euler",
			{ "<-", "In"},
			{"->", "Out"},
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				GenericEffectNode* _node = (GenericEffectNode*)node;
			}
			}
		},
		{
			EffectsNodeType::Euler_To_Quat,
			{
			"Euler To Quat",
			{ "<-", "In"},
			{"->", "Out"},
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				GenericEffectNode* _node = (GenericEffectNode*)node;
			}
			}
		},
		{
			EffectsNodeType::Quat_To_Vec4,
			{
			"Quat To Vec4",
			{ "<-", "In"},
			{"->", "Out"},
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				GenericEffectNode* _node = (GenericEffectNode*)node;
			}
			}
		},
		{
			EffectsNodeType::Vec3_Constant,
			{
			"Vec3",
			{ "<-", "X", "Y", "Z"},
			{"->", "Out"},
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				GenericEffectNode* _node = (GenericEffectNode*)node;
				ImGui::DragFloat3("Default", (float*)&_node->GetNodeValues()[0], 0.001f);
			}
			}
		},
		{
			EffectsNodeType::Vec4_Constant,
			{
			"Vec4",
			{ "<-", "X", "Y", "Z", "W"},
			{"->", "Out"},
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				GenericEffectNode* _node = (GenericEffectNode*)node;
				ImGui::DragFloat4("Default", (float*)&_node->GetNodeValues()[0], 0.001f);
			}
			}
		},
		{
			EffectsNodeType::Vec2_Constant,
			{
			"Vec2",
			{ "<-", "X", "Y"},
			{"->", "Out"},
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				GenericEffectNode* _node = (GenericEffectNode*)node;
				ImGui::DragFloat2("Default", (float*)&_node->GetNodeValues()[0], 0.001f);
			}
			}
		},
		{
			EffectsNodeType::Vec2_Constant,
			{
			"Vec2",
			{ "<-", "X", "Y"},
			{"->", "Out"},
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				GenericEffectNode* _node = (GenericEffectNode*)node;
				ImGui::DragFloat2("Default", (float*)&_node->GetNodeValues()[0], 0.001f);
			}
			}
		},
		{
			EffectsNodeType::Float_Constant,
			{
			"Float",
			{ "<-", "X"},
			{"->", "Out"},
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				GenericEffectNode* _node = (GenericEffectNode*)node;
				ImGui::DragFloat("Default", (float*)&_node->GetNodeValues()[0], 0.001f);
			}
			}
		},
		{
			EffectsNodeType::Percentage_Over_Life,
			{
			"Percentage Over Life",
			{ "<-"},
			{"->", "Value"},
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				GenericEffectNode* _node = (GenericEffectNode*)node;
			}
			}
		},
		{
			EffectsNodeType::Lerp_Vec4,
			{
			"Lerp Vec4",
			{ "<-", "A", "B", "t"},
			{"->", "Out"},
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				GenericEffectNode* _node = (GenericEffectNode*)node;
				ImGui::DragFloat4("A", (float*)&_node->GetNodeValues()[0], 0.001f);
				ImGui::DragFloat4("B", (float*)&_node->GetNodeValues()[1], 0.001f);
				ImGui::DragFloat("t", (float*)&_node->GetNodeValues()[2], 0.001f);
			}
			}
		},
		{
			EffectsNodeType::Lerp_Vec3,
			{
			"Lerp Vec3",
			{ "<-", "A", "B", "t"},
			{"->", "Out"},
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				GenericEffectNode* _node = (GenericEffectNode*)node;
				ImGui::DragFloat3("A", (float*)&_node->GetNodeValues()[0], 0.001f);
				ImGui::DragFloat3("B", (float*)&_node->GetNodeValues()[1], 0.001f);
				ImGui::DragFloat("t", (float*)&_node->GetNodeValues()[2], 0.001f);
			}
			}
		},
		{
			EffectsNodeType::Lerp_Vec2,
			{
			"Lerp Vec2",
			{ "<-", "A", "B", "t"},
			{"->", "Out"},
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				GenericEffectNode* _node = (GenericEffectNode*)node;
				ImGui::DragFloat2("A", (float*)&_node->GetNodeValues()[0], 0.001f);
				ImGui::DragFloat2("B", (float*)&_node->GetNodeValues()[1], 0.001f);
				ImGui::DragFloat("t", (float*)&_node->GetNodeValues()[2], 0.001f);
			}
			}
		},
		{
			EffectsNodeType::Lerp_Float,
			{
			"Lerp Float",
			{ "<-", "A", "B", "t"},
			{"->", "Out"},
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				GenericEffectNode* _node = (GenericEffectNode*)node;
				ImGui::DragFloat("A", (float*)&_node->GetNodeValues()[0], 0.001f);
				ImGui::DragFloat("B", (float*)&_node->GetNodeValues()[1], 0.001f);
				ImGui::DragFloat("t", (float*)&_node->GetNodeValues()[2], 0.001f);
			}
			}
		},
		{
			EffectsNodeType::Multiply_Vec4,
			{
			"Multiply Vec4",
			{ "<-", "A", "B"},
			{"->", "Out"},
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				GenericEffectNode* _node = (GenericEffectNode*)node;
				ImGui::DragFloat4("A", (float*)&_node->GetNodeValues()[0], 0.001f);
				ImGui::DragFloat4("B", (float*)&_node->GetNodeValues()[1], 0.001f);
			}
			}
		},
		{
			EffectsNodeType::Multiply_Vec3,
			{
			"Multiply Vec3",
			{ "<-", "A", "B"},
			{"->", "Out"},
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				GenericEffectNode* _node = (GenericEffectNode*)node;
				ImGui::DragFloat3("A", (float*)&_node->GetNodeValues()[0], 0.001f);
				ImGui::DragFloat3("B", (float*)&_node->GetNodeValues()[1], 0.001f);
			}
			}
		},
		{
			EffectsNodeType::Multiply_Vec2,
			{
			"Multiply Vec2",
			{ "<-", "A", "B"},
			{"->", "Out"},
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				GenericEffectNode* _node = (GenericEffectNode*)node;
				ImGui::DragFloat2("A", (float*)&_node->GetNodeValues()[0], 0.001f);
				ImGui::DragFloat2("B", (float*)&_node->GetNodeValues()[1], 0.001f);
			}
			}
		},
		{
			EffectsNodeType::Multiply_Vec4_Float,
			{
			"Multiply Vec4_Float",
			{ "<-", "A", "B"},
			{"->", "Out"},
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				GenericEffectNode* _node = (GenericEffectNode*)node;
				ImGui::DragFloat4("A", (float*)&_node->GetNodeValues()[0], 0.001f);
				ImGui::DragFloat("B", (float*)&_node->GetNodeValues()[1], 0.001f);
			}
			}
		},
		{
			EffectsNodeType::Multiply_Vec3_Float,
			{
			"Multiply Vec3_Float",
			{ "<-", "A", "B"},
			{"->", "Out"},
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				GenericEffectNode* _node = (GenericEffectNode*)node;
				ImGui::DragFloat3("A", (float*)&_node->GetNodeValues()[0], 0.001f);
				ImGui::DragFloat("B", (float*)&_node->GetNodeValues()[1], 0.001f);
			}
			}
		},
		{
			EffectsNodeType::Multiply_Vec2_Float,
			{
			"Multiply Vec2_Float",
			{ "<-", "A", "B"},
			{"->", "Out"},
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				GenericEffectNode* _node = (GenericEffectNode*)node;
				ImGui::DragFloat2("A", (float*)&_node->GetNodeValues()[0], 0.001f);
				ImGui::DragFloat("B", (float*)&_node->GetNodeValues()[1], 0.001f);
			}
			}
		},
		{
			EffectsNodeType::Multiply_Float,
			{
			"Multiply Float",
			{ "<-", "A", "B"},
			{"->", "Out"},
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				GenericEffectNode* _node = (GenericEffectNode*)node;
				ImGui::DragFloat("A", (float*)&_node->GetNodeValues()[0], 0.001f);
				ImGui::DragFloat("B", (float*)&_node->GetNodeValues()[1], 0.001f);
			}
			}
		},
		{
			EffectsNodeType::Random_Vec4,
			{
			"Random Vec4",
			{ "<-", },
			{"->", "Out"},
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				GenericEffectNode* _node = (GenericEffectNode*)node;
			}
			}
		},
		{
			EffectsNodeType::Random_Vec3,
			{
			"Random Vec3",
			{ "<-", },
			{"->", "Out"},
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				GenericEffectNode* _node = (GenericEffectNode*)node;
			}
			}
		},
		{
			EffectsNodeType::Random_Vec2,
			{
			"Random Vec2",
			{ "<-", },
			{"->", "Out"},
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				GenericEffectNode* _node = (GenericEffectNode*)node;
			}
			}
		},
		{
			EffectsNodeType::Random_Float,
			{
			"Random Float",
			{ "<-", },
			{"->", "Out"},
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				GenericEffectNode* _node = (GenericEffectNode*)node;
			}
			}
		},
		{
			EffectsNodeType::Split_Vec3,
			{
			"Split Vec3",
			{ "<-", "In"},
			{"->", "XY", "X", "Y", "Z"},
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				GenericEffectNode* _node = (GenericEffectNode*)node;
				ImGui::DragFloat3("Default", (float*)&_node->GetNodeValues()[0], 0.001f);
			}
			}
		},
		{
			EffectsNodeType::Split_Vec2,
			{
			"Split Vec2",
			{ "<-", "In"},
			{"->", "X", "Y"},
			[](GenericNode* node, GenericGraphEditor* editor)
			{
				GenericEffectNode* _node = (GenericEffectNode*)node;
				ImGui::DragFloat2("Default", (float*)&_node->GetNodeValues()[0], 0.001f);
			}
			}
		},
	};

	std::string get_class_display_name(uint64_t class_id)
	{
		switch (class_id)
		{
		case Reflection::TypeID<RateSpawner>():
		{
			return "Rate Spawner";
			break;
		}
		case Reflection::TypeID<VelocityInitializer>():
		{
			return "Velocity Initializer";
			break;
		}
		case Reflection::TypeID<LifetimeInitializer>():
		{
			return "Lifetime Initializer";
			break;
		}
		case Reflection::TypeID<GravityUpdate>():
		{
			return "Gravity Update";
			break;
		}
		case Reflection::TypeID<DragUpdate>():
		{
			return "Drag Update";
			break;
		}
		case Reflection::TypeID<VelocityUpdate>():
		{
			return "Velocity Update";
			break;
		}
		case Reflection::TypeID<WindUpdate>():
		{
			return "Wind Update";
			break;
		}
		case Reflection::TypeID<BillBoardRender>():
		{
			return "BillBoard Render";
			break;
		}
		case Reflection::TypeID<PointVolume>():
		{
			return "Point Volume";
			break;
		}
		case Reflection::TypeID<SphereVolume>():
		{
			return "Sphere Volume";
			break;
		}
		case Reflection::TypeID<BoxVolume>():
		{
			return "Box Volume";
			break;
		}
		case Reflection::TypeID<CircleVolume>():
		{
			return "Circle Volume";
			break;
		}
		case Reflection::TypeID<CustomParticleInitializer>():
		{
			return "Custom Particle Initializer";
			break;
		}
		case Reflection::TypeID<CustomParticleUpdate>():
		{
			return "Custom Particle Update";
			break;
		}
		}

		return "UnKnown ClassType";
	}

}
