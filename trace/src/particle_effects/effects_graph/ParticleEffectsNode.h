#pragma once

#include "node_system/GenericNode.h"

namespace trace {

	
	enum class EffectsNodeType
	{
		UnKnown,
		Split_Vec4,
		Vec4_To_Quat,
		Quat_To_Euler,
		Euler_To_Quat,
		Quat_To_Vec4,
		Vec3_Constant,
		Vec4_Constant,
		Vec2_Constant,
		Float_Constant,
		Percentage_Over_Life,
		Lerp_Vec4,
		Lerp_Vec3,
		Lerp_Vec2,
		Lerp_Float,
		Multiply_Vec4,
		Multiply_Vec3,
		Multiply_Vec2,
		Multiply_Vec4_Float,
		Multiply_Vec3_Float,
		Multiply_Vec2_Float,
		Multiply_Float,
		Random_Vec4,
		Random_Vec3,
		Random_Vec2,
		Random_Float,
		Split_Vec3,
		Split_Vec2,

		Max
	};

	struct EffectsNodeMetaData
	{
		std::vector<GenericNodeInput> inputs;
		std::vector<GenericNodeOutput> outputs;
		std::function<void(int32_t, GenericGraphInstance*, float, GenericNode*)> update_code;

	};

	class ParticleEffectNode : public GenericNode
	{
	public:
		virtual void Init(GenericGraph* graph) override;
		virtual bool Instanciate(GenericGraphInstance* graph_instance) = 0;
		virtual void Destroy(GenericGraph* graph) = 0;
		virtual void Update(int32_t particle_index, GenericGraphInstance* instance, float deltaTime) = 0;
		


		virtual void Update(GenericGraphInstance* instance, float deltaTime) override {};
		virtual void Run(GenericGraphInstance* instance) override {};


	public:
		virtual void* GetValueInternal(GenericGraphInstance* instance, uint32_t value_index = 0) { return nullptr; };

		ACCESS_CLASS_MEMBERS(ParticleEffectNode);
		GET_TYPE_ID;
	};


	class EffectsFinalNode : public ParticleEffectNode
	{

	public:
		virtual void Init(GenericGraph* graph) override;
		virtual void Destroy(GenericGraph* graph) override;
		virtual bool Instanciate(GenericGraphInstance* graph_instance) override;
		virtual void Update(int32_t particle_index, GenericGraphInstance* instance, float deltaTime) override;
		
		struct NodeData
		{
			uint32_t frame_index = 0;
		};

		ACCESS_CLASS_MEMBERS(EffectsFinalNode);
		GET_TYPE_ID;

	};
	
	
	class EffectsRootNode : public ParticleEffectNode
	{

	public:
		virtual void Init(GenericGraph* graph) override;
		virtual void Destroy(GenericGraph* graph) override;
		virtual bool Instanciate(GenericGraphInstance* graph_instance) override;
		virtual void Update(int32_t particle_index, GenericGraphInstance* instance, float deltaTime) override;
		
		ACCESS_CLASS_MEMBERS(EffectsRootNode);
		GET_TYPE_ID;

	};


	class GenericEffectNode : public ParticleEffectNode
	{

	public:
		virtual void Init(GenericGraph* graph) override;
		virtual void Destroy(GenericGraph* graph) override;
		virtual bool Instanciate(GenericGraphInstance* graph_instance) override;
		virtual void Update(int32_t particle_index, GenericGraphInstance* instance, float deltaTime) override;
		virtual void* GetValueInternal(GenericGraphInstance* instance, uint32_t value_index = 0);

		std::array<glm::vec4, 6>& GetNodeValues() { return m_nodeValues; }
		
		EffectsNodeType GetType() { return m_type; }
		void SetType(EffectsNodeType type);

		struct NodeData
		{
			uint32_t frame_index = 0;
			glm::vec4 out_0;
			glm::vec4 out_1;
			glm::vec4 out_2;
			glm::vec4 out_3;
			glm::vec4 out_4;
			glm::vec4 out_5;
		};

	private:
		EffectsNodeType m_type;
		std::array<glm::vec4, 6> m_nodeValues;// Default Input values

		ACCESS_CLASS_MEMBERS(GenericEffectNode);
		GET_TYPE_ID;

	};

	
	class GetParticleAttributeNode : public ParticleEffectNode
	{

	public:
		virtual void Init(GenericGraph* graph) override;
		virtual void Destroy(GenericGraph* graph) override;
		virtual bool Instanciate(GenericGraphInstance* graph_instance) override;
		virtual void Update(int32_t particle_index, GenericGraphInstance* instance, float deltaTime) override;

		virtual void* GetValueInternal(GenericGraphInstance* instance, uint32_t value_index = 0);

		StringID GetAttrID() { return m_attrID; }
		void SetAttrID(StringID attr_id);

		struct NodeData
		{
			uint32_t frame_index = 0;
			glm::vec4 attr_value = glm::vec4(0.0f);
			bool has_value = false;
		};

	private:
		StringID m_attrID;
		glm::vec4 m_value;

		ACCESS_CLASS_MEMBERS(GetParticleAttributeNode);
		GET_TYPE_ID;

	};


	class SetParticleAttributeNode : public ParticleEffectNode
	{

	public:
		virtual void Init(GenericGraph* graph) override;
		virtual void Destroy(GenericGraph* graph) override;
		virtual bool Instanciate(GenericGraphInstance* graph_instance) override;
		virtual void Update(int32_t particle_index, GenericGraphInstance* instance, float deltaTime) override;

		virtual void* GetValueInternal(GenericGraphInstance* instance, uint32_t value_index = 0);

		StringID GetAttrID() { return m_attrID; }
		void SetAttrID(StringID attr_id);

		glm::vec4 GetDefaultValue() { return m_value; }
		void SetDefaultValue(glm::vec4 value) { m_value = value; }

		struct NodeData
		{
			uint32_t frame_index = 0;
		};

	private:
		StringID m_attrID;
		glm::vec4 m_value;

		ACCESS_CLASS_MEMBERS(SetParticleAttributeNode);
		GET_TYPE_ID;

	};

}
