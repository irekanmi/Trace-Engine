#include "pch.h"

#include "Mesh.h"


namespace trace {



	Mesh::Mesh()
	{
	}

	Mesh::~Mesh()
	{
		
	}

	bool Mesh::Init(const std::vector<Ref<Model>>& models)
	{
		m_models = models;
		return true;
	}

	uint32_t Mesh::GetCount()
	{
		return static_cast<uint32_t>(m_models.size());
	}

}