#pragma once

#include "node_system/GenericNode.h"
#include "node_system/GenericGraph.h"

#include <string>
#include <functional>
#include <array>

namespace trace {


	enum class ShaderNodeType
	{
		UnKnown,
		Float_Constant,
		Int_Constant,
		Vec2_Constant,
		Vec3_Constant,
		Vec4_Constant,
		Sample_Texture_Function,
		Normal_Texture_To_World_Function,
		Frensel_Function,
		Multiply_Vec4_Function,
		Multiply_Vec4_Float_Function,
		Multiply_Vec3_Function,
		Multiply_Vec3_Float_Function,
		Multiply_Vec2_Function,
		Multiply_Vec2_Float_Function,
		Multiply_Float_Function,
		Add_Vec4_Function,
		Add_Vec4_Float_Function,
		Add_Vec3_Function,
		Add_Vec3_Float_Function,
		Add_Vec2_Function,
		Add_Vec2_Float_Function,
		Add_Float_Function,
		Object_UV_Variable,
		Object_Normal_Variable,
		Object_Tangent_Variable,
		View_Direction_Variable,
		Split_Vec4_Helper,
		Split_Vec3_Helper,
		Split_Vec2_Helper,
		Transparent_UnLit_Node,
		Screen_Color_Variable,
		Simple_Noise_Function,
		Gradient_Noise_Function,
		Twist_Function,
		Normal_From_Gradient_Noise,
		Time_Variable,
		Sin_Float,
		Radians_Float,
		Fract_Float,
		Screen_UV_Variable,
		Subtract_Vec4_Function,
		Subtract_Vec4_Float_Function,
		Subtract_Vec3_Function,
		Subtract_Vec3_Float_Function,
		Subtract_Vec2_Function,
		Subtract_Vec2_Float_Function,
		Subtract_Float_Function,
		Lerp_Float_Function,
		Lerp_Vec4_Function,
		Lerp_Vec3_Function,
		Lerp_Vec2_Function
	};

	struct ShaderNodeMetaData
	{
		std::vector<GenericNodeInput> inputs;
		std::vector<GenericNodeOutput> outputs;
		std::function<std::string(std::string*, GenericNode*, GenericGraphInstance*)> gen_code;
		
	};

	class ShaderGraphNode : public GenericNode
	{
	public:
		virtual bool Instanciate(GenericGraphInstance* graph_instance) = 0;
		virtual void Update(GenericGraphInstance* instance, float deltaTime) {};
		virtual void Run(GenericGraphInstance* instance) = 0;
		virtual void Init(GenericGraph* graph) = 0;
		virtual void Destroy(GenericGraph* graph) = 0;
		virtual std::string& GetCode(GenericGraphInstance* graph_instance) = 0;


	public:
		virtual void* GetValueInternal(GenericGraphInstance* instance, uint32_t value_index = 0) { return nullptr; };

		ACCESS_CLASS_MEMBERS(ShaderGraphNode);
		GET_TYPE_ID;

	};

	class AddNode : public ShaderGraphNode
	{
	public:
		virtual bool Instanciate(GenericGraphInstance* graph_instance) override;
		virtual void Run(GenericGraphInstance* instance) override;
		virtual void Init(GenericGraph* graph) override;
		virtual void Destroy(GenericGraph* graph) override;

		virtual std::string& GetCode(GenericGraphInstance* graph_instance) override;

		struct NodeData
		{
			std::string code;
			std::string out_0;
			uint32_t frame_index = 0;
		};

	private:
		GenericValueType m_type = GenericValueType::Float;
		GenericParameterData m_a;// Default value
		GenericParameterData m_b;// Default value

	public:
		virtual void* GetValueInternal(GenericGraphInstance* instance, uint32_t value_index = 0) override;

	protected:
		ACCESS_CLASS_MEMBERS(AddNode);
		GET_TYPE_ID;
	};

	class ConstantNode : public ShaderGraphNode
	{
	public:
		virtual bool Instanciate(GenericGraphInstance* graph_instance) override;
		virtual void Run(GenericGraphInstance* instance) override;
		virtual void Init(GenericGraph* graph) override;
		virtual void Destroy(GenericGraph* graph) override;

		void SetType(ShaderNodeType type);
		ShaderNodeType GetType() { return m_nodeType; }

		struct NodeData
		{
			std::string code;
			std::string out_[6];
			
			uint32_t frame_index = 0;
		};

		virtual std::string& GetCode(GenericGraphInstance* graph_instance) override;
		virtual void* GetValueInternal(GenericGraphInstance* instance, uint32_t value_index = 0) override;

		GenericParameterData* GetDefaultParameterData() { return m_nodeValues.data(); }

	private:
		GenericValueType m_type = GenericValueType::Float;
		std::array<GenericParameterData, 6> m_nodeValues;// Default Input values
		ShaderNodeType m_nodeType;

	protected:
		ACCESS_CLASS_MEMBERS(ConstantNode);
		GET_TYPE_ID;
	};
	
	class GetVariableNode : public ShaderGraphNode
	{
	public:
		virtual bool Instanciate(GenericGraphInstance* graph_instance) override;
		virtual void Run(GenericGraphInstance* instance) override;
		virtual void Init(GenericGraph* graph) override;
		virtual void Destroy(GenericGraph* graph) override;

		struct NodeData
		{
			std::string code;
			std::string out_0;
			uint32_t frame_index = 0;
		};

		int32_t GetVar() { return m_varIndex; }
		void SetVar(int32_t var_index) { m_varIndex = var_index; }

		virtual std::string& GetCode(GenericGraphInstance* graph_instance) override;
		virtual void* GetValueInternal(GenericGraphInstance* instance, uint32_t value_index = 0) override;

	private:
		int32_t m_varIndex = -1;

	protected:
		ACCESS_CLASS_MEMBERS(GetVariableNode);
		GET_TYPE_ID;
	};
	
	class FinalPBRNode : public ShaderGraphNode
	{
	public:
		virtual bool Instanciate(GenericGraphInstance* graph_instance) override;
		virtual void Run(GenericGraphInstance* instance) override;
		virtual void Init(GenericGraph* graph) override;
		virtual void Destroy(GenericGraph* graph) override;

		struct NodeData
		{
			std::string code;
			uint32_t frame_index = 0;
		};

		virtual std::string& GetCode(GenericGraphInstance* graph_instance) override;
		virtual void* GetValueInternal(GenericGraphInstance* instance, uint32_t value_index = 0) override { return nullptr; }

	public:
		GenericParameterData color;
		GenericParameterData occlusion;
		GenericParameterData metallic;
		GenericParameterData roughness;
		GenericParameterData emissionColor;


	protected:
		ACCESS_CLASS_MEMBERS(FinalPBRNode);
		GET_TYPE_ID;
	};

	



}
