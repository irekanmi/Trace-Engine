#pragma once

#include "core/Enums.h"
#include <iostream>



namespace trace {


	class Resource
	{

	public:
		Resource();
		virtual ~Resource();

	public:
		uint32_t m_refCount = 0;
		uint32_t m_id = INVALID_ID;

	private:
	protected:

	};
}

