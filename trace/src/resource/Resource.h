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
		int m_refCount = 0;
		uint32_t m_id = INVAILD_ID;

	private:
	protected:

	};
}

