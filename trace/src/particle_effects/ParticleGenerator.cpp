#include "pch.h"

#include "particle_effects/ParticleGenerator.h"
#include "particle_effects/ParticleEffect.h"
#include "core/io/Logging.h"
#include "serialize/GenericSerializer.h"
#include "resource/GenericAssetManager.h"
#include "render/Camera.h"
#include "particle_effects/effects_graph/ParticleEffectsNode.h"

namespace trace {



	bool ParticleGenerator::Create()
	{

		return true;
	}
	
	bool ParticleGenerator::Create(void* ptr)
	{

		UUID root_node = CreateNode<EffectsRootNode>();

		m_effectRoot = root_node;

		return true;
	}

	void ParticleGenerator::Destroy()
	{
		//TODO: Use Custom Allocator
		delete m_spawner;
		m_spawner = nullptr;
		for (auto& i : m_initializers)
		{
			delete i;
			i = nullptr;
		}

		for (auto& i : m_updates)
		{
			delete i;
			i = nullptr;
		}
		
		for (auto& i : m_renderers)
		{
			delete i;
			i = nullptr;
		}

	}

	Ref<ParticleGenerator> ParticleGenerator::Deserialize(UUID id)
	{
		Ref<ParticleGenerator> result;

		if (AppSettings::is_editor)
		{
			std::string file_path = GetPathFromUUID(id).string();
			if (!file_path.empty())
			{
				result = GenericSerializer::Deserialize<ParticleGenerator>(file_path);
			}
		}
		else
		{
			return GenericAssetManager::get_instance()->Load_Runtime<ParticleGenerator>(id);
		}

		return result;
	}

	Ref<ParticleGenerator> ParticleGenerator::Deserialize(DataStream* stream)
	{
		return GenericSerializer::Deserialize<ParticleGenerator>(stream);
	}

	void ParticleGenerator::CreateParameter(const std::string& param_name, GenericValueType type)
	{
	}

	bool ParticleGeneratorInstance::CreateInstance(Ref<ParticleGenerator> generator)
	{
		if (!generator)
		{
			TRC_ERROR("Invalid Particle Generator Handle, Function: {}", __FUNCTION__);
			return false;
		}
		bool result = true;
		m_gen = generator;

		for (auto& i : m_gen->GetNodes())
		{
			result = i.second->Instanciate(this) && result;
		}

		std::vector<StringID>& custom_data = m_gen->GetCustomData();

		for (StringID& i : custom_data)
		{
			m_particleData.custom_data.emplace(std::make_pair(i, std::vector<glm::vec4>()));
		}

		m_particleData.positions.resize(m_gen->GetCapacity());
		m_particleData.color.resize(m_gen->GetCapacity());
		m_particleData.velocities.resize(m_gen->GetCapacity());
		m_particleData.scale.resize(m_gen->GetCapacity());
		m_particleData.rotation.resize(m_gen->GetCapacity());

		for (auto& i : m_particleData.custom_data)
		{
			i.second.resize(m_gen->GetCapacity());
		}

		return result;
	}

	void ParticleGeneratorInstance::DestroyInstance()
	{
		for (auto& i : m_nodesData)
		{
			delete i.second;
		}

	}

	void ParticleGeneratorInstance::Start(ParticleEffectInstance* effect_instance)
	{
		m_effectInstance = effect_instance;

		for (ParticleInitializer*& init : m_gen->GetInitializers())
		{
			init->Init(this);
		}
				

		m_elaspedTime = 0.0f;
	}

	void ParticleGeneratorInstance::Update(ParticleEffectInstance* effect_instance, float deltaTime)
	{
		int32_t num_alive = m_numAlive;
		for (int32_t index = num_alive - 1; index >= 0; index--)
		{
			float& life_time = m_particleData.positions[index].w;
			if (life_time <= 0.0f)
			{
				kill_particle(index);
			}
			else
			{
				life_time -= deltaTime;
			}
		}

		m_gen->GetSpawner()->Run(this, deltaTime);

		for (ParticleUpdate*& update : m_gen->GetUpdates())
		{
			for (uint32_t index = 0; index < m_numAlive; index++)
			{
				update->UpdateParticle(this, index, deltaTime);
			}
		}

	}

	void ParticleGeneratorInstance::Stop(ParticleEffectInstance* effect_instance)
	{
		m_numAlive = 0;
		m_elaspedTime = 0.0f;

		m_effectInstance = nullptr;
	}

	void ParticleGeneratorInstance::Render(ParticleEffectInstance* effect_instance, Camera* camera, glm::mat4 transform, int32_t render_graph_index)
	{
		for (ParticleRender*& render : m_gen->GetRenderers())
		{
			render->Render(this, camera, transform, render_graph_index);
		}
	}

	int32_t ParticleGeneratorInstance::emit_particle()
	{
		if (m_numAlive < m_gen->GetCapacity())
		{
			uint32_t index = m_numAlive++;

			return index;
		}

		return -1;
	}

	void ParticleGeneratorInstance::initialize_particle(uint32_t index)
	{
		for (ParticleInitializer*& init : m_gen->GetInitializers())
		{
			init->InitParticle(this, index);
		}
	}
	
	void ParticleGeneratorInstance::kill_particle(uint32_t index)
	{
		uint32_t dst_index = m_numAlive - 1;

		std::swap(m_particleData.positions[index], m_particleData.positions[dst_index]);
		std::swap(m_particleData.color[index], m_particleData.color[dst_index]);
		std::swap(m_particleData.velocities[index], m_particleData.velocities[dst_index]);
		std::swap(m_particleData.scale[index], m_particleData.scale[dst_index]);
		std::swap(m_particleData.rotation[index], m_particleData.rotation[dst_index]);

		for (auto& i : m_particleData.custom_data)
		{
			std::swap(i.second[index], i.second[dst_index]);
		}

		--m_numAlive;
	}

	bool ParticleGeneratorInstance::GetParticleAttribute(StringID attr_name, glm::vec4& out_data, uint32_t particle_index)
	{

		switch (attr_name.value)
		{
		case Reflection::hash("position"):
		{
			out_data = m_particleData.positions[particle_index];
			return true;
			break;
		}
		case Reflection::hash("velocity"):
		{
			out_data = m_particleData.velocities[particle_index];
			return true;
			break;
		}
		case Reflection::hash("lifetime"):
		{
			out_data.x = m_particleData.positions[particle_index].w;
			return true;
			break;
		}
		case Reflection::hash("scale"):
		{
			out_data = m_particleData.scale[particle_index];
			return true;
			break;
		}
		case Reflection::hash("color"):
		{
			out_data = m_particleData.color[particle_index];
			return true;
			break;
		}
		case Reflection::hash("rotation"):
		{
			out_data = m_particleData.rotation[particle_index];
			return true;
			break;
		}
		default:
			auto it = m_particleData.custom_data.find(attr_name);
			if (it == m_particleData.custom_data.end())
			{
				return false;
			}

			out_data = m_particleData.custom_data[attr_name][particle_index];

			return true;
		}

		return false;
	}

	bool ParticleGeneratorInstance::SetParticleAttribute(StringID attr_name, glm::vec4& data, uint32_t particle_index)
	{
		switch (attr_name)
		{
		case Reflection::hash("position"):
		{
			m_particleData.positions[particle_index].x = data.x;
			m_particleData.positions[particle_index].y = data.y;
			m_particleData.positions[particle_index].z = data.z;
			return true;
			break;
		}
		case Reflection::hash("velocity"):
		{
			m_particleData.velocities[particle_index].x = data.x;
			m_particleData.velocities[particle_index].y = data.y;
			m_particleData.velocities[particle_index].z = data.z;
			return true;
			break;
		}
		case Reflection::hash("lifetime"):
		{
			m_particleData.positions[particle_index].w = data.x;
			return true;
			break;
		}
		case Reflection::hash("scale"):
		{
			m_particleData.scale[particle_index].x = data.x;
			m_particleData.scale[particle_index].y = data.y;
			m_particleData.scale[particle_index].z = data.z;
			return true;
			break;
		}
		case Reflection::hash("color"):
		{
			m_particleData.color[particle_index].x = data.x;
			m_particleData.color[particle_index].y = data.y;
			m_particleData.color[particle_index].z = data.z;
			return true;
			break;
		}
		case Reflection::hash("rotation"):
		{
			m_particleData.rotation[particle_index] = data;
			return true;
			break;
		}
		default:
			auto it = m_particleData.custom_data.find(attr_name);
			if (it == m_particleData.custom_data.end())
			{
				return false;
			}

			m_particleData.custom_data[attr_name][particle_index] = data;

			return true;
		}

		return false;
	}

	void ParticleGeneratorInstance::set_parameter_data(const std::string& param_name, void* data, uint32_t size)
	{
	}

}