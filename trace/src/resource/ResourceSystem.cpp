#include "pch.h"

#include "ResourceSystem.h"
#include "core/Enums.h"

namespace trace {

	ResourceSystem* ResourceSystem::s_instance = nullptr;

	ResourceSystem::ResourceSystem()
	{
	}
	ResourceSystem::~ResourceSystem()
	{
	}
	bool ResourceSystem::Init()
	{
		bool result = false;

		//TODO: Configurable
		result = m_textureManager.Init(4096);

		return result;
	}
	void ResourceSystem::ShutDown()
	{
		m_textureManager.ShutDown();
	}
	Texture_Ref ResourceSystem::GetTexture(const std::string& name)
	{
		return { m_textureManager.GetTexture(name), BIND_RESOURCE_UNLOAD_FN(TextureManager::UnloadTexture, &m_textureManager)};
	}
	void ResourceSystem::ReleaseTexture(const std::string& name)
	{
		m_textureManager.ReleaseTexture(name);
	}
	ResourceSystem* ResourceSystem::get_instance()
	{

		if (s_instance == nullptr)
		{
			s_instance = new ResourceSystem();
		}
		return s_instance;
	}
}