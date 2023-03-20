#pragma once

#include "core/Core.h"
#include "core/Enums.h"
#include "resource/Resource.h"
#include "Model.h"
#include <vector>



namespace trace {


	class TRACE_API Mesh : public Resource
	{

	public:
		Mesh();
		~Mesh();

		bool Init(const std::vector<Ref<Model>>& models);

		uint32_t GetCount() { return m_models.size(); }
		std::vector<Ref<Model>>& GetModels() { return m_models; }

	private:
		std::vector<Ref<Model>> m_models;

	protected:

	};




}
