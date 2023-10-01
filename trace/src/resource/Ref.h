#pragma once
#include <functional>
#include "core/io/Logging.h"
#include "core/Enums.h"

template<typename T>
using _action = std::function<void(T*)>;


template<typename T>
class Ref
{
public:


	Ref()
	{
		_ptr = nullptr;
	}

	void operator=(Ref& other)
	{
		if (is_valid())
		{
			_ptr->m_refCount--;
			if (_ptr->m_refCount <= 0)
			{
				Unload(_ptr);
			}
		}

		TRC_ASSERT(other.get() != nullptr, "Ensure to assign a vaild Reference Ref<{}>", _STR(T));
		other.get()->m_refCount++;
		_ptr = other._ptr;
		Unload = other.Unload;
	}
	void operator=(Ref&& other)
	{
		if (is_valid())
		{
			_ptr->m_refCount--;
			if (_ptr->m_refCount <= 0)
			{
				Unload(_ptr);
			}
		}

		//TRC_ASSERT(other.get() != nullptr, "Ensure to assign a vaild Reference Ref<{}>", _STR(T));
		_ptr = other._ptr;
		Unload = other.Unload;
		if(_ptr) other.get()->m_refCount++;
	}


	Ref(T* value, _action<T> unload)
	{
		TRC_ASSERT(value != nullptr, "Can't not construct from a null pointer Ref<{}>", typeid(T).name());
		Unload = unload;
		_ptr = value;
		_ptr->m_refCount++;
	}

	Ref(Ref&& other)
	{
		if (other.is_valid())
		{
			other.get()->m_refCount++;
			_ptr = other._ptr;
			Unload = other.Unload;
		}
	}


	Ref(Ref& other)
	{
		if (other.is_valid())
		{
			other.get()->m_refCount++;
			_ptr = other._ptr;
			Unload = other.Unload;
		}
	}
	
	Ref(const Ref& other)
	{

		_ptr = other._ptr;
		Unload = other.Unload;
		if (_ptr)
		{
			_ptr->m_refCount++;
		}
	}

	~Ref()
	{
		release();
	}

	T* operator->()
	{
		TRC_ASSERT(_ptr != nullptr, "Can't not dereference an invaild Ref<{}>", _STR(T));
		return _ptr;

	}

	T* get()
	{
		TRC_ASSERT(_ptr != nullptr, "Can't not dereference an invaild Ref<{}>", _STR(T));
		return _ptr;
	}

	uint32_t get_count()
	{
		if (_ptr != nullptr)
			return (_ptr->m_refCount);

		return 0;
	}

	bool is_valid()
	{
		return _ptr != nullptr;
	}

	void release()
	{
		if (_ptr != nullptr)
		{
			_ptr->m_refCount--;
			if (_ptr->m_refCount <= 0)
			{
				Unload(_ptr);
				_ptr = nullptr;
			}
		}
	}

	operator bool() { return is_valid(); }

	_action<T> Unload;
private:
	T* _ptr = nullptr;

protected:

};

