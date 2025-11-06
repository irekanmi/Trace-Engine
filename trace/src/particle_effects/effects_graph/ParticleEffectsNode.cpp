#include "pch.h"

#include "particle_effects/effects_graph/ParticleEffectsNode.h"
#include "particle_effects/ParticleGenerator.h"
#include "node_system/GenericGraph.h"
#include "core/Application.h"

#include "glm/gtx/compatibility.hpp"

namespace trace {


	static std::unordered_map<EffectsNodeType, EffectsNodeMetaData> node_meta_data =
	{
		{
			EffectsNodeType::Split_Vec4,
			{
				{GenericNodeInput{GenericValueType::Execute, 0, 0}, GenericNodeInput{GenericValueType::Vec4, 0, 0}},
				{{GenericValueType::Execute, 0} , {GenericValueType::Vec3, 0}, {GenericValueType::Vec2, 1}, {GenericValueType::Float, 2}, {GenericValueType::Float, 3}, {GenericValueType::Float, 4}, {GenericValueType::Float, 5}},
				[](int32_t particle_index, GenericGraphInstance* graph_instance, float deltaTime, GenericNode* node)
				{
					ParticleGeneratorInstance* instance = (ParticleGeneratorInstance*)graph_instance;
					GenericEffectNode* effect_node = (GenericEffectNode*)node;

					GenericEffectNode::NodeData* info = (GenericEffectNode::NodeData*)instance->GetNodesData()[node->GetUUID()];

					info->out_0 = effect_node->GetNodeValues()[0];

					GenericNodeInput& input_1 = node->GetInputs()[1];
					if (input_1.node_id != 0)
					{
						ParticleEffectNode* node_1 = (ParticleEffectNode*)instance->GetGenerator()->GetNode(input_1.node_id);

						node_1->Update(particle_index, instance, deltaTime);
						
						info->out_0 = *(glm::vec4*)node_1->GetValueInternal(instance, input_1.value_index);
					}

					info->out_1 = info->out_0;
					info->out_2.x = info->out_0.x;
					info->out_3.x = info->out_0.y;
					info->out_4.x = info->out_0.z;
					info->out_5.x = info->out_0.w;
					
				}
			}
		},
		{
			EffectsNodeType::Vec4_To_Quat,
			{
				{GenericNodeInput{GenericValueType::Execute, 0, 0}, GenericNodeInput{GenericValueType::Vec4, 0, 0}},
				{{GenericValueType::Execute, 0} , {GenericValueType::Quat, 1}},
				[](int32_t particle_index, GenericGraphInstance* graph_instance, float deltaTime, GenericNode* node)
				{
					ParticleGeneratorInstance* instance = (ParticleGeneratorInstance*)graph_instance;
					GenericEffectNode* effect_node = (GenericEffectNode*)node;

					GenericEffectNode::NodeData* info = (GenericEffectNode::NodeData*)instance->GetNodesData()[node->GetUUID()];

					info->out_0 = effect_node->GetNodeValues()[0];

					GenericNodeInput& input_1 = node->GetInputs()[1];
					if (input_1.node_id != 0)
					{
						ParticleEffectNode* node_1 = (ParticleEffectNode*)instance->GetGenerator()->GetNode(input_1.node_id);

						node_1->Update(particle_index, instance, deltaTime);
						
						glm::vec4 result = *(glm::vec4*)node_1->GetValueInternal(instance, input_1.value_index);
						glm::quat data(result.w, result.x, result.y, result.z);
						memcpy(&info->out_1, &data, sizeof(glm::quat));
					}					
				}
			}
		},
		{
			EffectsNodeType::Quat_To_Euler,
			{
				{GenericNodeInput{GenericValueType::Execute, 0, 0}, GenericNodeInput{GenericValueType::Quat, 0, 0}},
				{{GenericValueType::Execute, 0} , {GenericValueType::Vec3, 1}},
				[](int32_t particle_index, GenericGraphInstance* graph_instance, float deltaTime, GenericNode* node)
				{
					ParticleGeneratorInstance* instance = (ParticleGeneratorInstance*)graph_instance;
					GenericEffectNode* effect_node = (GenericEffectNode*)node;

					GenericEffectNode::NodeData* info = (GenericEffectNode::NodeData*)instance->GetNodesData()[node->GetUUID()];

					info->out_0 = effect_node->GetNodeValues()[0];

					GenericNodeInput& input_1 = node->GetInputs()[1];
					if (input_1.node_id != 0)
					{
						ParticleEffectNode* node_1 = (ParticleEffectNode*)instance->GetGenerator()->GetNode(input_1.node_id);

						node_1->Update(particle_index, instance, deltaTime);
						
						glm::quat result = *(glm::quat*)node_1->GetValueInternal(instance, input_1.value_index);
						glm::vec3 data = glm::eulerAngles(result);
						memcpy(&info->out_1, &data, sizeof(glm::vec3));
					}					
				}
			}
		},
		{
			EffectsNodeType::Euler_To_Quat,
			{
				{GenericNodeInput{GenericValueType::Execute, 0, 0}, GenericNodeInput{GenericValueType::Vec3, 0, 0}},
				{{GenericValueType::Execute, 0} , {GenericValueType::Quat, 1}},
				[](int32_t particle_index, GenericGraphInstance* graph_instance, float deltaTime, GenericNode* node)
				{
					ParticleGeneratorInstance* instance = (ParticleGeneratorInstance*)graph_instance;
					GenericEffectNode* effect_node = (GenericEffectNode*)node;

					GenericEffectNode::NodeData* info = (GenericEffectNode::NodeData*)instance->GetNodesData()[node->GetUUID()];

					info->out_0 = effect_node->GetNodeValues()[0];

					GenericNodeInput& input_1 = node->GetInputs()[1];
					if (input_1.node_id != 0)
					{
						ParticleEffectNode* node_1 = (ParticleEffectNode*)instance->GetGenerator()->GetNode(input_1.node_id);

						node_1->Update(particle_index, instance, deltaTime);
						
						glm::vec3 result = *(glm::vec3*)node_1->GetValueInternal(instance, input_1.value_index);
						glm::quat data(result);
						memcpy(&info->out_1, &data, sizeof(glm::quat));
					}					
				}
			}
		},
		{
			EffectsNodeType::Quat_To_Vec4,
			{
				{GenericNodeInput{GenericValueType::Execute, 0, 0}, GenericNodeInput{GenericValueType::Quat, 0, 0}},
				{{GenericValueType::Execute, 0} , {GenericValueType::Vec4, 1}},
				[](int32_t particle_index, GenericGraphInstance* graph_instance, float deltaTime, GenericNode* node)
				{
					ParticleGeneratorInstance* instance = (ParticleGeneratorInstance*)graph_instance;
					GenericEffectNode* effect_node = (GenericEffectNode*)node;

					GenericEffectNode::NodeData* info = (GenericEffectNode::NodeData*)instance->GetNodesData()[node->GetUUID()];

					info->out_0 = effect_node->GetNodeValues()[0];

					GenericNodeInput& input_1 = node->GetInputs()[1];
					if (input_1.node_id != 0)
					{
						ParticleEffectNode* node_1 = (ParticleEffectNode*)instance->GetGenerator()->GetNode(input_1.node_id);

						node_1->Update(particle_index, instance, deltaTime);
						
						glm::quat result = *(glm::quat*)node_1->GetValueInternal(instance, input_1.value_index);
						glm::vec4 data(result.x, result.y, result.z, result.w);
						memcpy(&info->out_1, &data, sizeof(glm::vec4));
					}					
				}
			}
		},
		{
			EffectsNodeType::Vec3_Constant,
			{
				{GenericNodeInput{GenericValueType::Execute, 0, 0}, GenericNodeInput{GenericValueType::Float, 0, 0}, GenericNodeInput{GenericValueType::Float, 0, 0}, GenericNodeInput{GenericValueType::Float, 0, 0}},
				{{GenericValueType::Execute, 0} , {GenericValueType::Vec3, 1}},
				[](int32_t particle_index, GenericGraphInstance* graph_instance, float deltaTime, GenericNode* node)
				{
					ParticleGeneratorInstance* instance = (ParticleGeneratorInstance*)graph_instance;
					GenericEffectNode* effect_node = (GenericEffectNode*)node;

					GenericEffectNode::NodeData* info = (GenericEffectNode::NodeData*)instance->GetNodesData()[node->GetUUID()];

					info->out_1 = effect_node->GetNodeValues()[0];

					GenericNodeInput& input_1 = node->GetInputs()[1];
					if (input_1.node_id != 0)
					{
						ParticleEffectNode* node_1 = (ParticleEffectNode*)instance->GetGenerator()->GetNode(input_1.node_id);

						node_1->Update(particle_index, instance, deltaTime);
						
						float result = *(float*)node_1->GetValueInternal(instance, input_1.value_index);
						info->out_1.x = result;
					}	

					GenericNodeInput& input_2 = node->GetInputs()[2];
					if (input_2.node_id != 0)
					{
						ParticleEffectNode* node_2 = (ParticleEffectNode*)instance->GetGenerator()->GetNode(input_2.node_id);

						node_2->Update(particle_index, instance, deltaTime);
						
						float result = *(float*)node_2->GetValueInternal(instance, input_2.value_index);
						info->out_1.y = result;
					}	

					GenericNodeInput& input_3 = node->GetInputs()[3];
					if (input_3.node_id != 0)
					{
						ParticleEffectNode* node_3 = (ParticleEffectNode*)instance->GetGenerator()->GetNode(input_3.node_id);

						node_3->Update(particle_index, instance, deltaTime);
						
						float result = *(float*)node_3->GetValueInternal(instance, input_3.value_index);
						info->out_1.z = result;
					}					
				}
			}
		},
		{
			EffectsNodeType::Vec4_Constant,
			{
				{GenericNodeInput{GenericValueType::Execute, 0, 0}, GenericNodeInput{GenericValueType::Float, 0, 0}, GenericNodeInput{GenericValueType::Float, 0, 0}, GenericNodeInput{GenericValueType::Float, 0, 0}, GenericNodeInput{GenericValueType::Float, 0, 0}},
				{{GenericValueType::Execute, 0} , {GenericValueType::Vec4, 1}},
				[](int32_t particle_index, GenericGraphInstance* graph_instance, float deltaTime, GenericNode* node)
				{
					ParticleGeneratorInstance* instance = (ParticleGeneratorInstance*)graph_instance;
					GenericEffectNode* effect_node = (GenericEffectNode*)node;

					GenericEffectNode::NodeData* info = (GenericEffectNode::NodeData*)instance->GetNodesData()[node->GetUUID()];

					info->out_1 = effect_node->GetNodeValues()[0];

					GenericNodeInput& input_1 = node->GetInputs()[1];
					if (input_1.node_id != 0)
					{
						ParticleEffectNode* node_1 = (ParticleEffectNode*)instance->GetGenerator()->GetNode(input_1.node_id);

						node_1->Update(particle_index, instance, deltaTime);
						
						float result = *(float*)node_1->GetValueInternal(instance, input_1.value_index);
						info->out_1.x = result;
					}	

					GenericNodeInput& input_2 = node->GetInputs()[2];
					if (input_2.node_id != 0)
					{
						ParticleEffectNode* node_2 = (ParticleEffectNode*)instance->GetGenerator()->GetNode(input_2.node_id);

						node_2->Update(particle_index, instance, deltaTime);
						
						float result = *(float*)node_2->GetValueInternal(instance, input_2.value_index);
						info->out_1.y = result;
					}	

					GenericNodeInput& input_3 = node->GetInputs()[3];
					if (input_3.node_id != 0)
					{
						ParticleEffectNode* node_3 = (ParticleEffectNode*)instance->GetGenerator()->GetNode(input_3.node_id);

						node_3->Update(particle_index, instance, deltaTime);
						
						float result = *(float*)node_3->GetValueInternal(instance, input_3.value_index);
						info->out_1.z = result;
					}	

					GenericNodeInput& input_4 = node->GetInputs()[4];
					if (input_4.node_id != 0)
					{
						ParticleEffectNode* node_4 = (ParticleEffectNode*)instance->GetGenerator()->GetNode(input_4.node_id);

						node_4->Update(particle_index, instance, deltaTime);
						
						float result = *(float*)node_4->GetValueInternal(instance, input_4.value_index);
						info->out_1.w = result;
					}					
				}
			}
		},
		{
			EffectsNodeType::Vec2_Constant,
			{
				{GenericNodeInput{GenericValueType::Execute, 0, 0}, GenericNodeInput{GenericValueType::Float, 0, 0}, GenericNodeInput{GenericValueType::Float, 0, 0}},
				{{GenericValueType::Execute, 0} , {GenericValueType::Vec2, 1}},
				[](int32_t particle_index, GenericGraphInstance* graph_instance, float deltaTime, GenericNode* node)
				{
					ParticleGeneratorInstance* instance = (ParticleGeneratorInstance*)graph_instance;
					GenericEffectNode* effect_node = (GenericEffectNode*)node;

					GenericEffectNode::NodeData* info = (GenericEffectNode::NodeData*)instance->GetNodesData()[node->GetUUID()];

					info->out_1 = effect_node->GetNodeValues()[0];

					GenericNodeInput& input_1 = node->GetInputs()[1];
					if (input_1.node_id != 0)
					{
						ParticleEffectNode* node_1 = (ParticleEffectNode*)instance->GetGenerator()->GetNode(input_1.node_id);

						node_1->Update(particle_index, instance, deltaTime);
						
						float result = *(float*)node_1->GetValueInternal(instance, input_1.value_index);
						info->out_1.x = result;
					}	

					GenericNodeInput& input_2 = node->GetInputs()[2];
					if (input_2.node_id != 0)
					{
						ParticleEffectNode* node_2 = (ParticleEffectNode*)instance->GetGenerator()->GetNode(input_2.node_id);

						node_2->Update(particle_index, instance, deltaTime);
						
						float result = *(float*)node_2->GetValueInternal(instance, input_2.value_index);
						info->out_1.y = result;
					}				
				}
			}
		},
		{
			EffectsNodeType::Float_Constant,
			{
				{GenericNodeInput{GenericValueType::Execute, 0, 0}, GenericNodeInput{GenericValueType::Float, 0, 0}},
				{{GenericValueType::Execute, 0} , {GenericValueType::Float, 1}},
				[](int32_t particle_index, GenericGraphInstance* graph_instance, float deltaTime, GenericNode* node)
				{
					ParticleGeneratorInstance* instance = (ParticleGeneratorInstance*)graph_instance;
					GenericEffectNode* effect_node = (GenericEffectNode*)node;

					GenericEffectNode::NodeData* info = (GenericEffectNode::NodeData*)instance->GetNodesData()[node->GetUUID()];

					info->out_1 = effect_node->GetNodeValues()[0];

					GenericNodeInput& input_1 = node->GetInputs()[1];
					if (input_1.node_id != 0)
					{
						ParticleEffectNode* node_1 = (ParticleEffectNode*)instance->GetGenerator()->GetNode(input_1.node_id);

						node_1->Update(particle_index, instance, deltaTime);
						
						float result = *(float*)node_1->GetValueInternal(instance, input_1.value_index);
						info->out_1.x = result;
					}			
				}
			}
		},
		{
			EffectsNodeType::Percentage_Over_Life,
			{
				{GenericNodeInput{GenericValueType::Execute, 0, 0}},
				{{GenericValueType::Execute, 0} , {GenericValueType::Float, 1}},
				[](int32_t particle_index, GenericGraphInstance* graph_instance, float deltaTime, GenericNode* node)
				{
					ParticleGeneratorInstance* instance = (ParticleGeneratorInstance*)graph_instance;
					GenericEffectNode* effect_node = (GenericEffectNode*)node;

					GenericEffectNode::NodeData* info = (GenericEffectNode::NodeData*)instance->GetNodesData()[node->GetUUID()];

					info->out_1 = effect_node->GetNodeValues()[0];

					ParticleData& particles_data = instance->GetParticlesData();
					float current_age = particles_data.positions[particle_index].w;
					float lifetime = particles_data.scale[particle_index].w;

					float percentage = current_age / lifetime;

					info->out_1.x = percentage;
		
				}
			}
		},
		{
			EffectsNodeType::Lerp_Vec4,
			{
				{GenericNodeInput{GenericValueType::Execute, 0, 0}, GenericNodeInput{GenericValueType::Vec4, 0, 0}, GenericNodeInput{GenericValueType::Vec4, 0, 0}, GenericNodeInput{GenericValueType::Float, 0, 0}},
				{{GenericValueType::Execute, 0} , {GenericValueType::Vec4, 1}},
				[](int32_t particle_index, GenericGraphInstance* graph_instance, float deltaTime, GenericNode* node)
				{
					ParticleGeneratorInstance* instance = (ParticleGeneratorInstance*)graph_instance;
					GenericEffectNode* effect_node = (GenericEffectNode*)node;

					GenericEffectNode::NodeData* info = (GenericEffectNode::NodeData*)instance->GetNodesData()[node->GetUUID()];

					glm::vec4 a = effect_node->GetNodeValues()[0];
					glm::vec4 b = effect_node->GetNodeValues()[1];
					float t = effect_node->GetNodeValues()[1].x;

					GenericNodeInput& input_1 = node->GetInputs()[1];
					if (input_1.node_id != 0)
					{
						ParticleEffectNode* node_1 = (ParticleEffectNode*)instance->GetGenerator()->GetNode(input_1.node_id);

						node_1->Update(particle_index, instance, deltaTime);

						glm::vec4 result = *(glm::vec4*)node_1->GetValueInternal(instance, input_1.value_index);
						a = result;
					}

					GenericNodeInput& input_2 = node->GetInputs()[2];
					if (input_2.node_id != 0)
					{
						ParticleEffectNode* node_2 = (ParticleEffectNode*)instance->GetGenerator()->GetNode(input_2.node_id);

						node_2->Update(particle_index, instance, deltaTime);

						glm::vec4 result = *(glm::vec4*)node_2->GetValueInternal(instance, input_2.value_index);
						b = result;
					}

					GenericNodeInput& input_3 = node->GetInputs()[3];
					if (input_3.node_id != 0)
					{
						ParticleEffectNode* node_3 = (ParticleEffectNode*)instance->GetGenerator()->GetNode(input_3.node_id);

						node_3->Update(particle_index, instance, deltaTime);

						float result = *(float*)node_3->GetValueInternal(instance, input_3.value_index);
						t = result;
					}

					glm::vec4 final_result = glm::lerp(a, b, t);
					info->out_1 = glm::vec4(final_result);
				}
			}
		},
		{
			EffectsNodeType::Lerp_Vec3,
			{
				{GenericNodeInput{GenericValueType::Execute, 0, 0}, GenericNodeInput{GenericValueType::Vec3, 0, 0}, GenericNodeInput{GenericValueType::Vec3, 0, 0}, GenericNodeInput{GenericValueType::Float, 0, 0}},
				{{GenericValueType::Execute, 0} , {GenericValueType::Vec3, 1}},
				[](int32_t particle_index, GenericGraphInstance* graph_instance, float deltaTime, GenericNode* node)
				{
					ParticleGeneratorInstance* instance = (ParticleGeneratorInstance*)graph_instance;
					GenericEffectNode* effect_node = (GenericEffectNode*)node;

					GenericEffectNode::NodeData* info = (GenericEffectNode::NodeData*)instance->GetNodesData()[node->GetUUID()];

					glm::vec3 a = effect_node->GetNodeValues()[0];
					glm::vec3 b = effect_node->GetNodeValues()[1];
					float t = effect_node->GetNodeValues()[1].x;

					GenericNodeInput& input_1 = node->GetInputs()[1];
					if (input_1.node_id != 0)
					{
						ParticleEffectNode* node_1 = (ParticleEffectNode*)instance->GetGenerator()->GetNode(input_1.node_id);

						node_1->Update(particle_index, instance, deltaTime);

						glm::vec3 result = *(glm::vec3*)node_1->GetValueInternal(instance, input_1.value_index);
						a = result;
					}

					GenericNodeInput& input_2 = node->GetInputs()[2];
					if (input_2.node_id != 0)
					{
						ParticleEffectNode* node_2 = (ParticleEffectNode*)instance->GetGenerator()->GetNode(input_2.node_id);

						node_2->Update(particle_index, instance, deltaTime);

						glm::vec3 result = *(glm::vec3*)node_2->GetValueInternal(instance, input_2.value_index);
						b = result;
					}

					GenericNodeInput& input_3 = node->GetInputs()[3];
					if (input_3.node_id != 0)
					{
						ParticleEffectNode* node_3 = (ParticleEffectNode*)instance->GetGenerator()->GetNode(input_3.node_id);

						node_3->Update(particle_index, instance, deltaTime);

						float result = *(float*)node_3->GetValueInternal(instance, input_3.value_index);
						t = result;
					}

					glm::vec3 final_result = glm::lerp(a, b, t);
					info->out_1 = glm::vec4(final_result, 0.0f);
				}
			}
		},
		{
			EffectsNodeType::Lerp_Vec2,
			{
				{GenericNodeInput{GenericValueType::Execute, 0, 0}, GenericNodeInput{GenericValueType::Vec2, 0, 0}, GenericNodeInput{GenericValueType::Vec2, 0, 0}, GenericNodeInput{GenericValueType::Float, 0, 0}},
				{{GenericValueType::Execute, 0} , {GenericValueType::Vec2, 1}},
				[](int32_t particle_index, GenericGraphInstance* graph_instance, float deltaTime, GenericNode* node)
				{
					ParticleGeneratorInstance* instance = (ParticleGeneratorInstance*)graph_instance;
					GenericEffectNode* effect_node = (GenericEffectNode*)node;

					GenericEffectNode::NodeData* info = (GenericEffectNode::NodeData*)instance->GetNodesData()[node->GetUUID()];

					glm::vec2 a = effect_node->GetNodeValues()[0];
					glm::vec2 b = effect_node->GetNodeValues()[1];
					float t = effect_node->GetNodeValues()[1].x;

					GenericNodeInput& input_1 = node->GetInputs()[1];
					if (input_1.node_id != 0)
					{
						ParticleEffectNode* node_1 = (ParticleEffectNode*)instance->GetGenerator()->GetNode(input_1.node_id);

						node_1->Update(particle_index, instance, deltaTime);

						glm::vec2 result = *(glm::vec2*)node_1->GetValueInternal(instance, input_1.value_index);
						a = result;
					}

					GenericNodeInput& input_2 = node->GetInputs()[2];
					if (input_2.node_id != 0)
					{
						ParticleEffectNode* node_2 = (ParticleEffectNode*)instance->GetGenerator()->GetNode(input_2.node_id);

						node_2->Update(particle_index, instance, deltaTime);

						glm::vec2 result = *(glm::vec2*)node_2->GetValueInternal(instance, input_2.value_index);
						b = result;
					}

					GenericNodeInput& input_3 = node->GetInputs()[3];
					if (input_3.node_id != 0)
					{
						ParticleEffectNode* node_3 = (ParticleEffectNode*)instance->GetGenerator()->GetNode(input_3.node_id);

						node_3->Update(particle_index, instance, deltaTime);

						float result = *(float*)node_3->GetValueInternal(instance, input_3.value_index);
						t = result;
					}

					glm::vec2 final_result = glm::lerp(a, b, t);
					info->out_1 = glm::vec4(final_result, 0.0f, 0.0f);
				}
			}
		},
		{
			EffectsNodeType::Lerp_Float,
			{
				{GenericNodeInput{GenericValueType::Execute, 0, 0}, GenericNodeInput{GenericValueType::Float, 0, 0}, GenericNodeInput{GenericValueType::Float, 0, 0}, GenericNodeInput{GenericValueType::Float, 0, 0}},
				{{GenericValueType::Execute, 0} , {GenericValueType::Float, 1}},
				[](int32_t particle_index, GenericGraphInstance* graph_instance, float deltaTime, GenericNode* node)
				{
					ParticleGeneratorInstance* instance = (ParticleGeneratorInstance*)graph_instance;
					GenericEffectNode* effect_node = (GenericEffectNode*)node;

					GenericEffectNode::NodeData* info = (GenericEffectNode::NodeData*)instance->GetNodesData()[node->GetUUID()];

					float a = effect_node->GetNodeValues()[0].x;
					float b = effect_node->GetNodeValues()[1].x;
					float t = effect_node->GetNodeValues()[1].x;

					GenericNodeInput& input_1 = node->GetInputs()[1];
					if (input_1.node_id != 0)
					{
						ParticleEffectNode* node_1 = (ParticleEffectNode*)instance->GetGenerator()->GetNode(input_1.node_id);

						node_1->Update(particle_index, instance, deltaTime);

						float result = *(float*)node_1->GetValueInternal(instance, input_1.value_index);
						a = result;
					}

					GenericNodeInput& input_2 = node->GetInputs()[2];
					if (input_2.node_id != 0)
					{
						ParticleEffectNode* node_2 = (ParticleEffectNode*)instance->GetGenerator()->GetNode(input_2.node_id);

						node_2->Update(particle_index, instance, deltaTime);

						float result = *(float*)node_2->GetValueInternal(instance, input_2.value_index);
						b = result;
					}

					GenericNodeInput& input_3 = node->GetInputs()[3];
					if (input_3.node_id != 0)
					{
						ParticleEffectNode* node_3 = (ParticleEffectNode*)instance->GetGenerator()->GetNode(input_3.node_id);

						node_3->Update(particle_index, instance, deltaTime);

						float result = *(float*)node_3->GetValueInternal(instance, input_3.value_index);
						t = result;
					}

					float final_result = glm::lerp(a, b, t);
					info->out_1 = glm::vec4(final_result, 0.0f, 0.0f, 0.0f);
				}
			}
		},
		{
			EffectsNodeType::Multiply_Vec4,
			{
				{GenericNodeInput{GenericValueType::Execute, 0, 0}, GenericNodeInput{GenericValueType::Vec4, 0, 0}, GenericNodeInput{GenericValueType::Vec4, 0, 0}},
				{{GenericValueType::Execute, 0} , {GenericValueType::Vec4, 1}},
				[](int32_t particle_index, GenericGraphInstance* graph_instance, float deltaTime, GenericNode* node)
				{
					ParticleGeneratorInstance* instance = (ParticleGeneratorInstance*)graph_instance;
					GenericEffectNode* effect_node = (GenericEffectNode*)node;

					GenericEffectNode::NodeData* info = (GenericEffectNode::NodeData*)instance->GetNodesData()[node->GetUUID()];

					glm::vec4 a = effect_node->GetNodeValues()[0];
					glm::vec4 b = effect_node->GetNodeValues()[1];

					GenericNodeInput& input_1 = node->GetInputs()[1];
					if (input_1.node_id != 0)
					{
						ParticleEffectNode* node_1 = (ParticleEffectNode*)instance->GetGenerator()->GetNode(input_1.node_id);

						node_1->Update(particle_index, instance, deltaTime);

						glm::vec4 result = *(glm::vec4*)node_1->GetValueInternal(instance, input_1.value_index);
						a = result;
					}

					GenericNodeInput& input_2 = node->GetInputs()[2];
					if (input_2.node_id != 0)
					{
						ParticleEffectNode* node_2 = (ParticleEffectNode*)instance->GetGenerator()->GetNode(input_2.node_id);

						node_2->Update(particle_index, instance, deltaTime);

						glm::vec4 result = *(glm::vec4*)node_2->GetValueInternal(instance, input_2.value_index);
						b = result;
					}

					glm::vec4 final_result = a * b;
					info->out_1 = glm::vec4(final_result);
				}
			}
		},
		{
			EffectsNodeType::Multiply_Vec3,
			{
				{GenericNodeInput{GenericValueType::Execute, 0, 0}, GenericNodeInput{GenericValueType::Vec3, 0, 0}, GenericNodeInput{GenericValueType::Vec3, 0, 0}},
				{{GenericValueType::Execute, 0} , {GenericValueType::Float, 1}},
				[](int32_t particle_index, GenericGraphInstance* graph_instance, float deltaTime, GenericNode* node)
				{
					ParticleGeneratorInstance* instance = (ParticleGeneratorInstance*)graph_instance;
					GenericEffectNode* effect_node = (GenericEffectNode*)node;

					GenericEffectNode::NodeData* info = (GenericEffectNode::NodeData*)instance->GetNodesData()[node->GetUUID()];

					glm::vec3 a = effect_node->GetNodeValues()[0];
					glm::vec3 b = effect_node->GetNodeValues()[1];

					GenericNodeInput& input_1 = node->GetInputs()[1];
					if (input_1.node_id != 0)
					{
						ParticleEffectNode* node_1 = (ParticleEffectNode*)instance->GetGenerator()->GetNode(input_1.node_id);

						node_1->Update(particle_index, instance, deltaTime);

						glm::vec3 result = *(glm::vec3*)node_1->GetValueInternal(instance, input_1.value_index);
						a = result;
					}

					GenericNodeInput& input_2 = node->GetInputs()[2];
					if (input_2.node_id != 0)
					{
						ParticleEffectNode* node_2 = (ParticleEffectNode*)instance->GetGenerator()->GetNode(input_2.node_id);

						node_2->Update(particle_index, instance, deltaTime);

						glm::vec3 result = *(glm::vec3*)node_2->GetValueInternal(instance, input_2.value_index);
						b = result;
					}

					glm::vec3 final_result = a * b;
					info->out_1 = glm::vec4(final_result, 0.0f);
				}
			}
		},
		{
			EffectsNodeType::Multiply_Vec2,
			{
				{GenericNodeInput{GenericValueType::Execute, 0, 0}, GenericNodeInput{GenericValueType::Vec2, 0, 0}, GenericNodeInput{GenericValueType::Vec2, 0, 0}},
				{{GenericValueType::Execute, 0} , {GenericValueType::Vec2, 1}},
				[](int32_t particle_index, GenericGraphInstance* graph_instance, float deltaTime, GenericNode* node)
				{
					ParticleGeneratorInstance* instance = (ParticleGeneratorInstance*)graph_instance;
					GenericEffectNode* effect_node = (GenericEffectNode*)node;

					GenericEffectNode::NodeData* info = (GenericEffectNode::NodeData*)instance->GetNodesData()[node->GetUUID()];

					glm::vec2 a = effect_node->GetNodeValues()[0];
					glm::vec2 b = effect_node->GetNodeValues()[1];

					GenericNodeInput& input_1 = node->GetInputs()[1];
					if (input_1.node_id != 0)
					{
						ParticleEffectNode* node_1 = (ParticleEffectNode*)instance->GetGenerator()->GetNode(input_1.node_id);

						node_1->Update(particle_index, instance, deltaTime);

						glm::vec2 result = *(glm::vec2*)node_1->GetValueInternal(instance, input_1.value_index);
						a = result;
					}

					GenericNodeInput& input_2 = node->GetInputs()[2];
					if (input_2.node_id != 0)
					{
						ParticleEffectNode* node_2 = (ParticleEffectNode*)instance->GetGenerator()->GetNode(input_2.node_id);

						node_2->Update(particle_index, instance, deltaTime);

						glm::vec2 result = *(glm::vec2*)node_2->GetValueInternal(instance, input_2.value_index);
						b = result;
					}

					glm::vec2 final_result = a * b;
					info->out_1 = glm::vec4(final_result, 0.0f, 0.0f);
				}
			}
		},
		{
			EffectsNodeType::Multiply_Vec4_Float,
			{
				{GenericNodeInput{GenericValueType::Execute, 0, 0}, GenericNodeInput{GenericValueType::Vec4, 0, 0}, GenericNodeInput{GenericValueType::Float, 0, 0}},
				{{GenericValueType::Execute, 0} , {GenericValueType::Vec4, 1}},
				[](int32_t particle_index, GenericGraphInstance* graph_instance, float deltaTime, GenericNode* node)
				{
					ParticleGeneratorInstance* instance = (ParticleGeneratorInstance*)graph_instance;
					GenericEffectNode* effect_node = (GenericEffectNode*)node;

					GenericEffectNode::NodeData* info = (GenericEffectNode::NodeData*)instance->GetNodesData()[node->GetUUID()];

					glm::vec4 a = effect_node->GetNodeValues()[0];
					float b = effect_node->GetNodeValues()[1].x;

					GenericNodeInput& input_1 = node->GetInputs()[1];
					if (input_1.node_id != 0)
					{
						ParticleEffectNode* node_1 = (ParticleEffectNode*)instance->GetGenerator()->GetNode(input_1.node_id);

						node_1->Update(particle_index, instance, deltaTime);

						glm::vec4 result = *(glm::vec4*)node_1->GetValueInternal(instance, input_1.value_index);
						a = result;
					}

					GenericNodeInput& input_2 = node->GetInputs()[2];
					if (input_2.node_id != 0)
					{
						ParticleEffectNode* node_2 = (ParticleEffectNode*)instance->GetGenerator()->GetNode(input_2.node_id);

						node_2->Update(particle_index, instance, deltaTime);

						float result = *(float*)node_2->GetValueInternal(instance, input_2.value_index);
						b = result;
					}

					glm::vec4 final_result = a * b;
					info->out_1 = glm::vec4(final_result);
				}
			}
		},
		{
			EffectsNodeType::Multiply_Vec3_Float, 
			{
				{GenericNodeInput{GenericValueType::Execute, 0, 0}, GenericNodeInput{GenericValueType::Vec3, 0, 0}, GenericNodeInput{GenericValueType::Float, 0, 0}},
				{{GenericValueType::Execute, 0} , {GenericValueType::Vec3, 1}},
				[](int32_t particle_index, GenericGraphInstance* graph_instance, float deltaTime, GenericNode* node)
				{
					ParticleGeneratorInstance* instance = (ParticleGeneratorInstance*)graph_instance;
					GenericEffectNode* effect_node = (GenericEffectNode*)node;

					GenericEffectNode::NodeData* info = (GenericEffectNode::NodeData*)instance->GetNodesData()[node->GetUUID()];

					glm::vec3 a = effect_node->GetNodeValues()[0];
					float b = effect_node->GetNodeValues()[1].x;

					GenericNodeInput& input_1 = node->GetInputs()[1];
					if (input_1.node_id != 0)
					{
						ParticleEffectNode* node_1 = (ParticleEffectNode*)instance->GetGenerator()->GetNode(input_1.node_id);

						node_1->Update(particle_index, instance, deltaTime);

						glm::vec3 result = *(glm::vec3*)node_1->GetValueInternal(instance, input_1.value_index);
						a = result;
					}

					GenericNodeInput& input_2 = node->GetInputs()[2];
					if (input_2.node_id != 0)
					{
						ParticleEffectNode* node_2 = (ParticleEffectNode*)instance->GetGenerator()->GetNode(input_2.node_id);

						node_2->Update(particle_index, instance, deltaTime);

						float result = *(float*)node_2->GetValueInternal(instance, input_2.value_index);
						b = result;
					}

					glm::vec3 final_result = a * b;
					info->out_1 = glm::vec4(final_result, 0.0f);
				}
			}
		},
		{
			EffectsNodeType::Multiply_Vec2_Float,
			{
				{GenericNodeInput{GenericValueType::Execute, 0, 0}, GenericNodeInput{GenericValueType::Vec2, 0, 0}, GenericNodeInput{GenericValueType::Float, 0, 0}},
				{{GenericValueType::Execute, 0} , {GenericValueType::Vec2, 1}},
				[](int32_t particle_index, GenericGraphInstance* graph_instance, float deltaTime, GenericNode* node)
				{
					ParticleGeneratorInstance* instance = (ParticleGeneratorInstance*)graph_instance;
					GenericEffectNode* effect_node = (GenericEffectNode*)node;

					GenericEffectNode::NodeData* info = (GenericEffectNode::NodeData*)instance->GetNodesData()[node->GetUUID()];

					glm::vec2 a = effect_node->GetNodeValues()[0];
					float b = effect_node->GetNodeValues()[1].x;

					GenericNodeInput& input_1 = node->GetInputs()[1];
					if (input_1.node_id != 0)
					{
						ParticleEffectNode* node_1 = (ParticleEffectNode*)instance->GetGenerator()->GetNode(input_1.node_id);

						node_1->Update(particle_index, instance, deltaTime);

						glm::vec2 result = *(glm::vec2*)node_1->GetValueInternal(instance, input_1.value_index);
						a = result;
					}

					GenericNodeInput& input_2 = node->GetInputs()[2];
					if (input_2.node_id != 0)
					{
						ParticleEffectNode* node_2 = (ParticleEffectNode*)instance->GetGenerator()->GetNode(input_2.node_id);

						node_2->Update(particle_index, instance, deltaTime);

						float result = *(float*)node_2->GetValueInternal(instance, input_2.value_index);
						b = result;
					}

					glm::vec2 final_result = a * b;
					info->out_1 = glm::vec4(final_result, 0.0f, 0.0f);
				}
			}
		},
		{
			EffectsNodeType::Multiply_Float,
			{
				{GenericNodeInput{GenericValueType::Execute, 0, 0}, GenericNodeInput{GenericValueType::Float, 0, 0}, GenericNodeInput{GenericValueType::Float, 0, 0}},
				{{GenericValueType::Execute, 0} , {GenericValueType::Float, 1}},
				[](int32_t particle_index, GenericGraphInstance* graph_instance, float deltaTime, GenericNode* node)
				{
					ParticleGeneratorInstance* instance = (ParticleGeneratorInstance*)graph_instance;
					GenericEffectNode* effect_node = (GenericEffectNode*)node;

					GenericEffectNode::NodeData* info = (GenericEffectNode::NodeData*)instance->GetNodesData()[node->GetUUID()];

					float a = effect_node->GetNodeValues()[0].x;
					float b = effect_node->GetNodeValues()[1].x;

					GenericNodeInput& input_1 = node->GetInputs()[1];
					if (input_1.node_id != 0)
					{
						ParticleEffectNode* node_1 = (ParticleEffectNode*)instance->GetGenerator()->GetNode(input_1.node_id);

						node_1->Update(particle_index, instance, deltaTime);

						float result = *(float*)node_1->GetValueInternal(instance, input_1.value_index);
						a = result;
					}

					GenericNodeInput& input_2 = node->GetInputs()[2];
					if (input_2.node_id != 0)
					{
						ParticleEffectNode* node_2 = (ParticleEffectNode*)instance->GetGenerator()->GetNode(input_2.node_id);

						node_2->Update(particle_index, instance, deltaTime);

						float result = *(float*)node_2->GetValueInternal(instance, input_2.value_index);
						b = result;
					}

					float final_result = a * b;
					info->out_1 = glm::vec4(final_result, 0.0f, 0.0f, 0.0f);
				}
			}
		},
		{
			EffectsNodeType::Random_Vec4,
			{
				{GenericNodeInput{GenericValueType::Execute, 0, 0}},
				{{GenericValueType::Execute, 0} , {GenericValueType::Vec4, 1}},
				[](int32_t particle_index, GenericGraphInstance* graph_instance, float deltaTime, GenericNode* node)
				{
					ParticleGeneratorInstance* instance = (ParticleGeneratorInstance*)graph_instance;
					GenericEffectNode* effect_node = (GenericEffectNode*)node;

					GenericEffectNode::NodeData* info = (GenericEffectNode::NodeData*)instance->GetNodesData()[node->GetUUID()];

					
					info->out_1 = glm::vec4(RandomFloat(), RandomFloat(), RandomFloat(), RandomFloat());
				}
			}
		},
		{
			EffectsNodeType::Random_Vec3,
			{
				{GenericNodeInput{GenericValueType::Execute, 0, 0}},
				{{GenericValueType::Execute, 0} , {GenericValueType::Vec3, 1}},
				[](int32_t particle_index, GenericGraphInstance* graph_instance, float deltaTime, GenericNode* node)
				{
					ParticleGeneratorInstance* instance = (ParticleGeneratorInstance*)graph_instance;
					GenericEffectNode* effect_node = (GenericEffectNode*)node;

					GenericEffectNode::NodeData* info = (GenericEffectNode::NodeData*)instance->GetNodesData()[node->GetUUID()];

					
					info->out_1 = glm::vec4(RandomFloat(), RandomFloat(), RandomFloat(), 0.0f);
				}
			}
		},
		{
			EffectsNodeType::Random_Vec2,
			{
				{GenericNodeInput{GenericValueType::Execute, 0, 0}},
				{{GenericValueType::Execute, 0} , {GenericValueType::Vec2, 1}},
				[](int32_t particle_index, GenericGraphInstance* graph_instance, float deltaTime, GenericNode* node)
				{
					ParticleGeneratorInstance* instance = (ParticleGeneratorInstance*)graph_instance;
					GenericEffectNode* effect_node = (GenericEffectNode*)node;

					GenericEffectNode::NodeData* info = (GenericEffectNode::NodeData*)instance->GetNodesData()[node->GetUUID()];

					
					info->out_1 = glm::vec4(RandomFloat(), RandomFloat(), 0.0f, 0.0f);
				}
			}
		},
		{
			EffectsNodeType::Random_Float,
			{
				{GenericNodeInput{GenericValueType::Execute, 0, 0}},
				{{GenericValueType::Execute, 0} , {GenericValueType::Float, 1}},
				[](int32_t particle_index, GenericGraphInstance* graph_instance, float deltaTime, GenericNode* node)
				{
					ParticleGeneratorInstance* instance = (ParticleGeneratorInstance*)graph_instance;
					GenericEffectNode* effect_node = (GenericEffectNode*)node;

					GenericEffectNode::NodeData* info = (GenericEffectNode::NodeData*)instance->GetNodesData()[node->GetUUID()];

					
					info->out_1 = glm::vec4(RandomFloat(), 0.0f, 0.0f, 0.0f);
				}
			}
		},
		{
			EffectsNodeType::Split_Vec3,
			{
				{GenericNodeInput{GenericValueType::Execute, 0, 0}, GenericNodeInput{GenericValueType::Vec3, 0, 0}},
				{{GenericValueType::Execute, 0} , {GenericValueType::Vec2, 0}, {GenericValueType::Float, 1}, {GenericValueType::Float, 2}, {GenericValueType::Float, 3}},
				[](int32_t particle_index, GenericGraphInstance* graph_instance, float deltaTime, GenericNode* node)
				{
					ParticleGeneratorInstance* instance = (ParticleGeneratorInstance*)graph_instance;
					GenericEffectNode* effect_node = (GenericEffectNode*)node;

					GenericEffectNode::NodeData* info = (GenericEffectNode::NodeData*)instance->GetNodesData()[node->GetUUID()];

					info->out_0 = effect_node->GetNodeValues()[0];

					GenericNodeInput& input_1 = node->GetInputs()[1];
					if (input_1.node_id != 0)
					{
						ParticleEffectNode* node_1 = (ParticleEffectNode*)instance->GetGenerator()->GetNode(input_1.node_id);

						node_1->Update(particle_index, instance, deltaTime);
						
						info->out_0 = *(glm::vec4*)node_1->GetValueInternal(instance, input_1.value_index);
					}

					info->out_1 = info->out_0;
					info->out_2.x = info->out_0.x;
					info->out_3.x = info->out_0.y;
					info->out_4.x = info->out_0.z;
					info->out_5.x = info->out_0.w;
					
				}
			}
		},
		{
			EffectsNodeType::Split_Vec2,
			{
				{GenericNodeInput{GenericValueType::Execute, 0, 0}, GenericNodeInput{GenericValueType::Vec2, 0, 0}},
				{{GenericValueType::Execute, 0} , {GenericValueType::Float, 0}, {GenericValueType::Float, 1}},
				[](int32_t particle_index, GenericGraphInstance* graph_instance, float deltaTime, GenericNode* node)
				{
					ParticleGeneratorInstance* instance = (ParticleGeneratorInstance*)graph_instance;
					GenericEffectNode* effect_node = (GenericEffectNode*)node;

					GenericEffectNode::NodeData* info = (GenericEffectNode::NodeData*)instance->GetNodesData()[node->GetUUID()];

					info->out_0 = effect_node->GetNodeValues()[0];

					GenericNodeInput& input_1 = node->GetInputs()[1];
					if (input_1.node_id != 0)
					{
						ParticleEffectNode* node_1 = (ParticleEffectNode*)instance->GetGenerator()->GetNode(input_1.node_id);

						node_1->Update(particle_index, instance, deltaTime);
						
						info->out_0 = *(glm::vec4*)node_1->GetValueInternal(instance, input_1.value_index);
					}

					info->out_1 = info->out_0;
					info->out_2.x = info->out_0.x;
					info->out_3.x = info->out_0.y;
					info->out_4.x = info->out_0.z;
					info->out_5.x = info->out_0.w;
					
				}
			}
		},
	};

	void ParticleEffectNode::Init(GenericGraph* graph)
	{


		GenericNodeInput input = {};
		input.node_id = 0;
		input.type = GenericValueType::Execute;
		input.value_index = INVALID_ID;

		m_inputs.resize(1);
		m_inputs[0] = input;

		GenericNodeOutput output = {};
		output.type = GenericValueType::Execute;
		output.value_index = 0;

		m_outputs.resize(1);
		m_outputs[0] = output;
		

	}

	void EffectsFinalNode::Init(GenericGraph* graph)
	{
		GenericNodeInput input = {};
		input.node_id = 0;
		input.type = GenericValueType::Execute;
		input.value_index = INVALID_ID;

		m_inputs.resize(1);
		m_inputs[0] = input;
	}

	void EffectsFinalNode::Destroy(GenericGraph* graph)
	{
	}

	bool EffectsFinalNode::Instanciate(GenericGraphInstance* graph_instance)
	{
		graph_instance->GetNodesData()[m_uuid] = new NodeData;

		return true;
	}

	void EffectsFinalNode::Update(int32_t particle_index, GenericGraphInstance* graph_instance, float deltaTime)
	{
		ParticleGeneratorInstance* instance = (ParticleGeneratorInstance*)graph_instance;

		NodeData* info = (NodeData*)instance->GetNodesData()[m_uuid];

		/*if (info->frame_index == Application::get_instance()->GetUpdateID())
		{
			return;
		}
		info->frame_index = Application::get_instance()->GetUpdateID();*/

		if (m_inputs[0].node_id == 0)
		{
			return;
		}

		ParticleEffectNode* node = (ParticleEffectNode*)instance->GetGenerator()->GetNode(m_inputs[0].node_id);

		node->Update(particle_index, instance, deltaTime);


	}

	void EffectsRootNode::Init(GenericGraph* graph)
	{
		GenericNodeInput input = {};
		input.node_id = 0;
		input.type = GenericValueType::Execute;
		input.value_index = INVALID_ID;

		m_inputs.resize(1);
		m_inputs[0] = input;
	}

	void EffectsRootNode::Destroy(GenericGraph* graph)
	{
	}

	bool EffectsRootNode::Instanciate(GenericGraphInstance* graph_instance)
	{
		return true;
	}

	void EffectsRootNode::Update(int32_t particle_index, GenericGraphInstance* instance, float deltaTime)
	{
	}

	void GenericEffectNode::Init(GenericGraph* graph)
	{
		
	}

	void GenericEffectNode::Destroy(GenericGraph* graph)
	{
	}

	bool GenericEffectNode::Instanciate(GenericGraphInstance* graph_instance)
	{
		graph_instance->GetNodesData()[m_uuid] = new NodeData;

		return true;
	}

	void GenericEffectNode::Update(int32_t particle_index, GenericGraphInstance* graph_instance, float deltaTime)
	{
		ParticleGeneratorInstance* instance = (ParticleGeneratorInstance*)graph_instance;

		NodeData* info = (NodeData*)instance->GetNodesData()[m_uuid];

		/*if (info->frame_index == Application::get_instance()->GetUpdateID())
		{
			return;
		}
		info->frame_index = Application::get_instance()->GetUpdateID();*/

		if (m_inputs[0].node_id != 0)
		{
			ParticleEffectNode* node = (ParticleEffectNode*)instance->GetGenerator()->GetNode(m_inputs[0].node_id);

			node->Update(particle_index, instance, deltaTime);
		}

		node_meta_data[m_type].update_code(particle_index, instance, deltaTime, this);


	}


	void* GenericEffectNode::GetValueInternal(GenericGraphInstance* graph_instance, uint32_t value_index)
	{
		ParticleGeneratorInstance* instance = (ParticleGeneratorInstance*)graph_instance;

		NodeData* info = (NodeData*)instance->GetNodesData()[m_uuid];

		switch (value_index)
		{
		case 0:
		{
			return &info->out_0;
			break;
		}
		case 1:
		{
			return &info->out_1;
			break;
		}
		case 2:
		{
			return &info->out_2;
			break;
		}
		case 3:
		{
			return &info->out_3;
			break;
		}
		case 4:
		{
			return &info->out_4;
			break;
		}
		case 5:
		{
			return &info->out_5;
			break;
		}
		}

		return nullptr;
	}

	void GenericEffectNode::SetType(EffectsNodeType type)
	{
		m_type = type;

		m_inputs = node_meta_data[m_type].inputs;
		m_outputs = node_meta_data[m_type].outputs;

	}

	void GetParticleAttributeNode::Init(GenericGraph* graph)
	{
		ParticleEffectNode::Init(graph);

		GenericNodeOutput output = {};
		output.type = GenericValueType::Vec4;
		output.value_index = 1;

		m_outputs.resize(2);
		m_outputs[1] = output;

	}

	void GetParticleAttributeNode::Destroy(GenericGraph* graph)
	{
	}

	bool GetParticleAttributeNode::Instanciate(GenericGraphInstance* graph_instance)
	{
		graph_instance->GetNodesData()[m_uuid] = new NodeData;

		return true;
	}

	void GetParticleAttributeNode::Update(int32_t particle_index, GenericGraphInstance* graph_instance, float deltaTime)
	{
		ParticleGeneratorInstance* instance = (ParticleGeneratorInstance*)graph_instance;

		NodeData* info = (NodeData*)instance->GetNodesData()[m_uuid];

		/*if (info->frame_index == Application::get_instance()->GetUpdateID())
		{
			return;
		}
		info->frame_index = Application::get_instance()->GetUpdateID();*/

		if (m_inputs[0].node_id != 0)
		{
			ParticleEffectNode* node = (ParticleEffectNode*)instance->GetGenerator()->GetNode(m_inputs[0].node_id);

			node->Update(particle_index, instance, deltaTime);
		}

		info->has_value = instance->GetParticleAttribute(m_attrID, info->attr_value, particle_index);

	}

	void* GetParticleAttributeNode::GetValueInternal(GenericGraphInstance* graph_instance, uint32_t value_index)
	{
		ParticleGeneratorInstance* instance = (ParticleGeneratorInstance*)graph_instance;

		NodeData* info = (NodeData*)instance->GetNodesData()[m_uuid];

		switch (value_index)
		{
		case 1:
		{
			return info->has_value ? &info->attr_value : &m_value;
			break;
		}
		}

		return nullptr;
	}

	void GetParticleAttributeNode::SetAttrID(StringID attr_id)
	{
		m_attrID = attr_id;
	}

	void SetParticleAttributeNode::Init(GenericGraph* graph)
	{
		ParticleEffectNode::Init(graph);

		GenericNodeInput input = {};
		input.type = GenericValueType::Vec4;
		input.value_index = INVALID_ID;
		input.node_id = 0;

		m_inputs.resize(2);
		m_inputs[1] = input;

	}

	void SetParticleAttributeNode::Destroy(GenericGraph* graph)
	{
	}

	bool SetParticleAttributeNode::Instanciate(GenericGraphInstance* graph_instance)
	{
		graph_instance->GetNodesData()[m_uuid] = new NodeData;//TODO: Use custom allocator

		return true;
	}

	void SetParticleAttributeNode::Update(int32_t particle_index, GenericGraphInstance* graph_instance, float deltaTime)
	{
		ParticleGeneratorInstance* instance = (ParticleGeneratorInstance*)graph_instance;

		NodeData* info = (NodeData*)instance->GetNodesData()[m_uuid];

		/*if (info->frame_index == Application::get_instance()->GetUpdateID())
		{
			return;
		}
		info->frame_index = Application::get_instance()->GetUpdateID();*/

		if (m_inputs[0].node_id != 0)
		{
			ParticleEffectNode* node = (ParticleEffectNode*)instance->GetGenerator()->GetNode(m_inputs[0].node_id);

			node->Update(particle_index, instance, deltaTime);
		}
		
		if (m_inputs[1].node_id != 0)
		{
			ParticleEffectNode* node = (ParticleEffectNode*)instance->GetGenerator()->GetNode(m_inputs[1].node_id);

			node->Update(particle_index, instance, deltaTime);

			glm::vec4* data = (glm::vec4*)node->GetValueInternal(instance, m_inputs[1].value_index);

			instance->SetParticleAttribute(m_attrID, *data, particle_index);
		}
		else
		{
			instance->SetParticleAttribute(m_attrID, m_value, particle_index);
		}


		

	}

	void* SetParticleAttributeNode::GetValueInternal(GenericGraphInstance* graph_instance, uint32_t value_index)
	{
		
		return nullptr;
	}

	void SetParticleAttributeNode::SetAttrID(StringID attr_id)
	{
		m_attrID = attr_id;
	}

}