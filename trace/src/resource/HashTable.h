#pragma once


#include <string>


template<typename T>
class HashTable
{

public:
	HashTable()
	{

	}
	HashTable(uint32_t element_count)
	{
		Init(element_count);
	}
	~HashTable()
	{
		delete[] table;
	}

	void Init(uint32_t count)
	{
		elementCount = count;
		//TODO: Use a Custom allocator for allocation
		table = new T[elementCount];
	}

	uint64_t Hash(const std::string& name)
	{
		static const uint64_t multiplier = 97;

		uint64_t hash = 0;

		unsigned const char* ch = (unsigned const char*)name.c_str();

		for (; *ch; ch++)
		{
			hash = hash * multiplier + *ch;
		}

		hash %= elementCount;

		return hash;
	}

	void Fill(T data)
	{
		for (uint32_t i = 0; i < elementCount; i++)
		{
			table[i] = data;
		}
	}

	void Set(const std::string& name, T data)
	{
		uint64_t hash = Hash(name);

		table[hash] = data;
	}

	T Get(const std::string& name)
	{
		uint64_t hash = Hash(name);


		return table[hash];
	}

	T& Get_Ref(const std::string& name)
	{
		uint64_t hash = Hash(name);


		return table[hash];
	}

private:
	uint32_t elementCount;
	T* table;


protected:

};

