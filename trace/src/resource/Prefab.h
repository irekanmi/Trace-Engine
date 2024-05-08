#pragma once

#include "resource/Resource.h"
#include "scene/Entity.h"

namespace trace {

	class Prefab : public Resource
	{

	public:

		Entity GetHandle() { return m_handle; }
		void SetHandle(Entity handle) { m_handle = handle; }
	private:
		Entity m_handle;
	protected:

	};

}
