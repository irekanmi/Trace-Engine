
#include <trace.h>
#include <stdio.h>
#include <unordered_map>
using namespace trace;

//Temp =================================
#include "glm/gtc/matrix_transform.hpp"
//========================

trace::FileHandle g_fileTest;

class SampleOverLay : public trace::Layer
{

public:
	SampleOverLay()
		:trace::Layer("SampleOverlay")
	{

	}
	~SampleOverLay()
	{

	}

	virtual void OnAttach() override
	{
		TRC_DEBUG("SampleOverlay Attached");
	}
	virtual void OnDetach() override
	{
		TRC_DEBUG("SampleOverlay Detached");
	}
	virtual void Update(float deltaTime) override
	{

	}
	virtual void OnEvent(trace::Event* p_event) override
	{
		switch (p_event->m_type)
		{
		case trace::EventType::TRC_KEY_RELEASED:
		{
			trace::KeyReleased* press = reinterpret_cast<trace::KeyReleased*>(p_event);
			if (press->m_keycode == trace::Keys::KEY_ESCAPE)
			{
				press->m_handled = true;
				trace::Application::get_instance()->PopOverLay(this);
			}
			break;
		}
		}
	}

private:
protected:

};

class SampleLayer : public trace::Layer
{

public:
	SampleLayer()
		:trace::Layer("SampleLayer")
	{
	}

	~SampleLayer()
	{

	}

	virtual void OnAttach() override
	{
		TRC_INFO("Sample Layer Attached");

		//Temp=======================

		TextureDesc texture_desc;
		texture_desc.m_addressModeU = texture_desc.m_addressModeW = texture_desc.m_addressModeV = AddressMode::REPEAT;
		texture_desc.m_format = Format::R8G8B8A8_UNORM;
		texture_desc.m_minFilterMode = texture_desc.m_magFilterMode = FilterMode::LINEAR;
		texture_desc.m_flag = BindFlag::SHADER_RESOURCE_BIT;
		texture_desc.m_usage = UsageFlag::DEFAULT;


		Material _mat;
		_mat.m_albedoMap = ResourceSystem::get_instance()->LoadTexture("cobblestone.png");
		_mat.m_diffuseColor = glm::vec4(0.3f, 0.5f, 0.45f, 1.0f);
		_mat.m_shininess = 128.0f;
		_mat.m_specularMap = ResourceSystem::get_instance()->LoadTexture("cobblestone_SPEC.png");
		_mat.m_normalMap = ResourceSystem::get_instance()->LoadTexture("cobblestone_NRM.png", texture_desc);

		_squareModel = ResourceSystem::get_instance()->GetDefaultMesh("Cube");
		//_sponzaScene = ResourceSystem::get_instance()->LoadMesh("sponza.obj");
		//_falcon = ResourceSystem::get_instance()->LoadMesh("falcon.obj");


		std::vector<std::string> cube_maps = {
			"sky_right.jpg",
			"sky_left.jpg",
			"sky_top.jpg",
			"sky_bottom.jpg",
			"sky_front.jpg",
			"sky_back.jpg"
		};

		TextureDesc cubeMapDesc;
		cubeMapDesc.m_image_type = ImageType::CUBE_MAP;
		cubeMapDesc.m_addressModeU = cubeMapDesc.m_addressModeV = cubeMapDesc.m_addressModeW = AddressMode::CLAMP_TO_EDGE;
		cubeMapDesc.m_numLayers = 6;
		cubeMapDesc.m_format = Format::R8G8B8A8_SRBG;
		cubeMapDesc.m_minFilterMode = cubeMapDesc.m_magFilterMode = FilterMode::LINEAR;
		cubeMapDesc.m_usage = UsageFlag::DEFAULT;
		cubeMapDesc.m_flag = BindFlag::SHADER_RESOURCE_BIT;

		Texture_Ref temp = ResourceSystem::get_instance()->LoadTexture(cube_maps, cubeMapDesc, "cube_map");

		sky_box = SkyBox(temp);

		std::string vert_src;
		std::string frag_src;

		vert_src = ShaderParser::load_shader_file("../assets/shaders/trace_core.shader.vert.glsl");
		frag_src = ShaderParser::load_shader_file("../assets/shaders/reflect.frag.glsl");

		TRC_TRACE(vert_src);
		TRC_TRACE(frag_src);

		GShader* VertShader = GShader::Create_(vert_src, ShaderStage::VERTEX_SHADER);
		GShader* FragShader = GShader::Create_(frag_src, ShaderStage::PIXEL_SHADER);

		ShaderResourceBinding projection;
		projection.shader_stage = ShaderStage::VERTEX_SHADER;
		projection.resource_stage = ShaderResourceStage::RESOURCE_STAGE_GLOBAL;
		projection.resource_size = sizeof(glm::mat4);
		projection.resource_type = ShaderResourceType::SHADER_RESOURCE_TYPE_UNIFORM_BUFFER;
		projection.resource_name = "projection";
		projection.count = 1;
		projection.index = 0;
		projection.slot = 0;
		projection.resource_data_type = ShaderData::CUSTOM_DATA_MAT4;

		ShaderResourceBinding view;
		view.shader_stage = ShaderStage::VERTEX_SHADER;
		view.resource_stage = ShaderResourceStage::RESOURCE_STAGE_GLOBAL;
		view.resource_size = sizeof(glm::mat4);
		view.resource_type = ShaderResourceType::SHADER_RESOURCE_TYPE_UNIFORM_BUFFER;
		view.resource_name = "view";
		view.count = 1;
		view.index = 0;
		view.slot = 0;
		view.resource_data_type = ShaderData::CUSTOM_DATA_MAT4;

		ShaderResourceBinding view_position;
		view_position.shader_stage = ShaderStage::VERTEX_SHADER;
		view_position.resource_stage = ShaderResourceStage::RESOURCE_STAGE_GLOBAL;
		view_position.resource_size = sizeof(glm::vec3);
		view_position.resource_type = ShaderResourceType::SHADER_RESOURCE_TYPE_UNIFORM_BUFFER;
		view_position.resource_name = "view_position";
		view_position.count = 1;
		view_position.index = 0;
		view_position.slot = 0;
		view_position.resource_data_type = ShaderData::CUSTOM_DATA_VEC3;

		ShaderResourceBinding _test;
		_test.shader_stage = ShaderStage::VERTEX_SHADER;
		_test.resource_stage = ShaderResourceStage::RESOURCE_STAGE_GLOBAL;
		_test.resource_size = sizeof(glm::vec2);
		_test.resource_type = ShaderResourceType::SHADER_RESOURCE_TYPE_UNIFORM_BUFFER;
		_test.resource_name = "_test";
		_test.count = 1;
		_test.index = 0;
		_test.slot = 0;
		_test.resource_data_type = ShaderData::CUSTOM_DATA_VEC2;

		ShaderResourceBinding normal_map;
		normal_map.shader_stage = ShaderStage::PIXEL_SHADER;
		normal_map.resource_stage = ShaderResourceStage::RESOURCE_STAGE_INSTANCE;
		normal_map.resource_size = 0;
		normal_map.resource_type = ShaderResourceType::SHADER_RESOURCE_TYPE_COMBINED_SAMPLER;
		normal_map.resource_name = "normal_map";
		normal_map.count = 1;
		normal_map.index = 0;
		normal_map.slot = 1;
		normal_map.resource_data_type = ShaderData::MATERIAL_NORMAL;


		ShaderResourceBinding rest;
		rest.shader_stage = ShaderStage::PIXEL_SHADER;
		rest.resource_stage = ShaderResourceStage::RESOURCE_STAGE_GLOBAL;
		rest.resource_size = sizeof(glm::ivec4);
		rest.resource_type = ShaderResourceType::SHADER_RESOURCE_TYPE_UNIFORM_BUFFER;
		rest.resource_name = "rest";
		rest.count = 1;
		rest.index = 0;
		rest.slot = 2;
		rest.resource_data_type = ShaderData::CUSTOM_DATA_VEC4;

		ShaderResourceBinding cube_data;
		cube_data.shader_stage = ShaderStage::PIXEL_SHADER;
		cube_data.resource_stage = ShaderResourceStage::RESOURCE_STAGE_GLOBAL;
		cube_data.resource_size = 0;
		cube_data.resource_type = ShaderResourceType::SHADER_RESOURCE_TYPE_COMBINED_SAMPLER;
		cube_data.resource_name = "CubeMap";
		cube_data.count = 1;
		cube_data.index = 0;
		cube_data.slot = 1;
		cube_data.resource_data_type = ShaderData::CUSTOM_DATA_TEXTURE;
		cube_data.data = temp.get();

		ShaderResourceBinding model;
		model.shader_stage = ShaderStage::VERTEX_SHADER;
		model.resource_stage = ShaderResourceStage::RESOURCE_STAGE_LOCAL;
		model.resource_size = sizeof(glm::mat4);
		model.resource_type = ShaderResourceType::SHADER_RESOURCE_TYPE_UNIFORM_BUFFER;
		model.resource_name = "model";
		model.count = 1;
		model.index = 0;
		model.slot = 3;
		model.resource_data_type = ShaderData::CUSTOM_DATA_MAT4;

		std::vector<ShaderResourceBinding> scene = {
			projection,
			view,
			view_position,
			_test,
			normal_map,
			cube_data,
			model,
			rest
		};

		PipelineStateDesc _ds;
		_ds.resource_bindings_count = 8;
		_ds.resource_bindings = scene;
		_ds.vertex_shader = VertShader;
		_ds.pixel_shader = FragShader;

		AutoFillPipelineDesc(
			_ds
		);

		if (!ResourceSystem::get_instance()->CreatePipeline(_ds, "reflect_pipeline"))
		{
			TRC_ERROR("Failed to initialize or create reflect pipeline");
		}

		Ref<GPipeline> reflect_p = ResourceSystem::get_instance()->GetPipeline("reflect_pipeline");
		delete VertShader;
		delete FragShader;


		ResourceSystem::get_instance()->CreateMaterial(
			"Material",
			_mat,
			ResourceSystem::get_instance()->GetDefaultPipeline("standard")
			//reflect_p
		);

		for (auto& i : _squareModel->GetModels())
		{
			i->m_matInstance = ResourceSystem::get_instance()->GetMaterial("Material");
		}


		M_sponzaScene.SetScale(glm::vec3(0.15f));
		M_falcon.SetScale(glm::vec3(3.0f));
		M_falcon.Translate(glm::vec3(3.0f, 3.0f, 0.0f));
		M_squareModel.SetScale(glm::vec3(11.0f));
		M_squareModel.Translate(glm::vec3(0.0f, 0.0f, 0.0f));


		//=============================
	}

	virtual void OnEvent(trace::Event* p_event) override
	{
		switch (p_event->m_type)
		{
		case trace::EventType::TRC_KEY_PRESSED:
		{
			trace::KeyPressed* press = reinterpret_cast<trace::KeyPressed*>(p_event);
			//printf("%c", press->m_keycode);
			break;
		}
		case trace::EventType::TRC_KEY_RELEASED:
		{
			trace::KeyReleased* release = reinterpret_cast<trace::KeyReleased*>(p_event);
			break;
		}
		}
	}

	virtual void OnDetach() override
	{
		TRC_WARN("Sample Layer detached");
	}

	virtual void Update(float deltaTime) override
	{

		static float x = 70.0f;
		static float r = -0.5f;

		x += r * deltaTime;
		M_squareModel.Rotate(glm::radians(x * deltaTime), glm::vec3(0.0f, 1.0f, 0.0f));
		M_falcon.Rotate(glm::radians(x * deltaTime), glm::vec3(0.0f, 1.0f, 0.0f));
		if (trace::InputSystem::get_instance()->GetKeyState(trace::Keys::KEY_T) == trace::KeyState::KEY_RELEASE)
		{
			TRC_INFO(" ------____----TRACE------______----");
		}
		CommandList cmd_list = Renderer::get_instance()->BeginCommandList();
		//Renderer::get_instance()->DrawMesh(cmd_list, _squareModel, M_squareModel.GetLocalMatrix() );
		//Renderer::get_instance()->DrawMesh(cmd_list, _falcon, M_falcon.GetLocalMatrix());
		//Renderer::get_instance()->DrawMesh(cmd_list, _sponzaScene, M_sponzaScene.GetLocalMatrix());
		//Renderer::get_instance()->DrawSky(cmd_list, &sky_box);
		Renderer::get_instance()->SubmitCommandList(cmd_list);

		

	}
private:
	//Temp ==================
	Ref<Mesh> _squareModel;
	Ref<Mesh> _sponzaScene;
	Ref<Mesh> _falcon;

	Transform M_squareModel;
	Transform M_sponzaScene;
	Transform M_falcon;
	SkyBox sky_box;
	//=======================
protected:


};



void Start()
{
	TRC_INFO("Client Start");
	trace::Application::get_instance()->PushLayer(new SampleLayer);
	trace::Application::get_instance()->PushOverLay(new SampleOverLay());

	
	
}

void Update(float deltaTime)
{
	
}

void End()
{
	TRC_WARN("Client End");
}

trace::trc_app_data trace::CreateApp()
{
	trace::trc_app_data app_data;
	app_data.winprop = trace::WindowDecl();
	app_data.wintype = trace::WindowType::GLFW_WINDOW;
	app_data.graphics_api = trace::RenderAPI::Vulkan;
	app_data.platform_api = trace::PlatformAPI::WINDOWS;
	app_data.windowed = true;
	app_data.enable_vsync = false;
	app_data.client_start = Start;
	app_data.client_update = Update;
	app_data.client_end = End;


	return app_data;

}



