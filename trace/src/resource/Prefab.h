#pragma once

#include "resource/Resource.h"
#include "scene/UUID.h"
#include "resource/Ref.h"

namespace trace {


	class Prefab : public Resource
	{

	public:

		UUID GetHandle() { return m_handle; }
		void SetHandle(UUID handle) { m_handle = handle; }



		static Ref<Prefab> Deserialize(UUID id);

	private:
		UUID m_handle;
	protected:

	};

}
