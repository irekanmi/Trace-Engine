#pragma once

#include "core/Enums.h"
#include <filesystem>



namespace trace {


	class Resource
	{

	public:
		Resource();
		virtual ~Resource();

		virtual void Destroy() {}

		//NOTE: To be used only for the editor
		std::string GetName() { return m_path.filename().string(); }

	public:
		uint32_t m_refCount = 0;
		uint32_t m_id = INVALID_ID;
		//NOTE: To be used only for the editor
		std::filesystem::path m_path;

	private:
	protected:

	};
}

