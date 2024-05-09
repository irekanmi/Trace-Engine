#pragma once

#include "resource/Resource.h"
#include "scene/UUID.h"

namespace trace {


	class Prefab : public Resource
	{

	public:

		UUID GetHandle() { return m_handle; }
		void SetHandle(UUID handle) { m_handle = handle; }
	private:
		UUID m_handle;
	protected:

	};

}
