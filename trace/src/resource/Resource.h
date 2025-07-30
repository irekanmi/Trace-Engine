#pragma once

#include "core/Enums.h"
#include "scene/UUID.h"
#include "multithreading/SpinLock.h"

#include <filesystem>


namespace trace {


	class Resource
	{

	public:
		Resource();
		virtual ~Resource();

		virtual void Destroy();

		//NOTE: To be used only for the editor
		std::string GetName() { return m_path.filename().string(); }
		UUID GetUUID();


		void Increment();
		void Decrement();
		

	public:
		//TODO: Determine if these is the right thing to do or if it is optimal enough
		uint32_t m_refCount;
		uint32_t m_id;
		UUID m_assetID;
		//NOTE: To be used only for the editor
		std::filesystem::path m_path;

	private:
		SpinLock m_refLock;
	protected:

	};
}

