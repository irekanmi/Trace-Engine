#pragma once

#include "core/Core.h"
#include "core/Enums.h"
#include "core/io/Logging.h"

#include <any>
#include <unordered_map>
#include <typeindex>

namespace trace {

	class TRACE_API RGBlackBoard
	{

	public:
		RGBlackBoard(){}
		~RGBlackBoard(){}

		template<typename T, typename... Args> T& add(Args&& ...args)
		{
			return m_bucket[typeid(T)].emplace<T>(T{ std::forward<Args>(args)... });
		}

		template<typename T> T& get()
		{
			TRC_ASSERT(has<T>(), "Please ensure type has been added to the board");
			return std::any_cast<T&>(m_bucket.at(typeid(T)));
		}

		template<typename T> T* try_get()
		{
			auto it = m_bucket.find(typeid(T));
			return m_bucket.find(typeid(T)) != m_bucket.end() ? std::any_cast<T*>(&it->second) : nullptr;
		}

		template<typename T> bool has()
		{
			return m_bucket.find(typeid(T)) != m_bucket.end() ? true : false;
		}

	private:
		std::unordered_map<std::type_index, std::any> m_bucket;

	protected:

	};

}
