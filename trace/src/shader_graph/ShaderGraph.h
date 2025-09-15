#pragma once

#include "resource/Ref.h"
#include "node_system/GenericGraph.h"
#include "shader_graph/ShaderGraphNode.h"
#include "render/Material.h"
#include "serialize/DataStream.h"

namespace trace {

	

	class ShaderGraph : public GenericGraph, public Resource
	{

	public:
		bool Create();
		bool Create(MaterialType type);
		virtual void Destroy() override;

		MaterialType GetType() { return m_type; }
		void SetType(MaterialType type) { m_type = type; }
		virtual void CreateParameter(const std::string& param_name, GenericValueType type) {};

		ShaderGraphNode* GetFragmentShaderNode();

		CullMode GetCullMode() { return m_cullMode; }
		void SetCullMode(CullMode cull_mode) { m_cullMode = cull_mode; }

		static Ref<ShaderGraph> Deserialize(UUID id);
		static Ref<ShaderGraph> Deserialize(DataStream* stream);

	private:
		ShaderGraphNode* vertex_shader_root = nullptr;
		UUID fragment_shader_root = 0;
		MaterialType m_type = MaterialType::NONE;
		CullMode m_cullMode = CullMode::BACK;

	protected:
		ACCESS_CLASS_MEMBERS(ShaderGraph);
		GET_TYPE_ID;

		friend class ShaderGraphInstance;

	};


	class ShaderGraphInstance : public GenericGraphInstance
	{

	public:
		bool CreateInstance(Ref<ShaderGraph> shader_graph);
		virtual void DestroyInstance() override;

		Ref<ShaderGraph> GetShaderGraph() { return m_shaderGraph; }
		void SetShaderGraph(Ref<ShaderGraph> shader_graph) { m_shaderGraph = shader_graph; }

		void WriteNodeCode(std::string& code);
		uint32_t GetNextVarIndex() { return current_var_index++; }
		std::string* GetParamString(const std::string& param_name);

		Ref<GPipeline> CompileGraph();

	private:
		Ref<ShaderGraph> m_shaderGraph;
		std::string final_code;
		uint32_t current_var_index = 100;
		std::unordered_map<std::string, std::string> m_paramString;

	protected:
		virtual void set_parameter_data(const std::string& name, void* data, uint32_t size) override;

	protected:
		ACCESS_CLASS_MEMBERS(ShaderGraphInstance);
		GET_TYPE_ID;
	};

}
