#include "pch.h"

#include "Mesh.h"


namespace trace {



	Mesh::Mesh()
	{
	}

	Mesh::~Mesh()
	{

		for (Ref<Model>& model : m_models)
		{
			model.~Ref();
		}

		m_models.clear();
	}

	bool Mesh::Init(const std::vector<Ref<Model>>& models)
	{
		m_models = models;
		return true;
	}

}