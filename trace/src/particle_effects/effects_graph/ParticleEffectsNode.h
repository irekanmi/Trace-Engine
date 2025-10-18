#pragma once

#include "node_system/GenericNode.h"

namespace trace {

	
	enum class EffectsNodeType
	{
		UnKnown,
		GetPosition,
		SetPosition,

		Max
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
		
		EffectsNodeType GetType() { return m_type; }
		void SetType(EffectsNodeType type) { m_type = type; }

		struct NodeData
		{
			uint32_t frame_index = 0;
		};

	private:
		EffectsNodeType m_type;

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

		UUID GetAttrID() { return m_attrID; }
		void SetAttrID(UUID attr_id);

		struct NodeData
		{
			uint32_t frame_index = 0;
		};

	private:
		UUID m_attrID;

		ACCESS_CLASS_MEMBERS(GetParticleAttributeNode);
		GET_TYPE_ID;

	};

}
