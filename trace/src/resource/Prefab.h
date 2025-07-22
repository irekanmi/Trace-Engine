#pragma once

#include "resource/Resource.h"
#include "scene/UUID.h"
#include "resource/Ref.h"
#include "serialize/DataStream.h"


namespace trace {

	class Entity;

	class Prefab : public Resource
	{

	public:

		bool Create(Entity handle);
		bool Create(void* null_init) { return true; };
		virtual void Destroy() override;

		UUID GetHandle() { return m_handle; }
		void SetHandle(UUID handle) { m_handle = handle; }



		static Ref<Prefab> Deserialize(UUID id);
		static Ref<Prefab> Deserialize(DataStream* stream);

	private:
		UUID m_handle;
	protected:

	};

}
