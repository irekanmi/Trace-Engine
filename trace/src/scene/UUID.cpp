#include "pch.h"
#include "UUID.h"

#include <random>

namespace trace {

	static std::random_device s_device;
	static std::mt19937_64 s_engine(s_device());
	static std::uniform_int_distribution<uint64_t> s_distributor;

	UUID::UUID()
	{
	}
	UUID UUID::GenUUID()
	{
		return UUID(s_distributor(s_engine));
	}



	static std::random_device s_device32;
	static std::mt19937_64 s_engine32(s_device32());
	static std::uniform_int_distribution<uint32_t> s_distributor32;

	UUID_32::UUID_32()
	{
	}
	UUID_32 UUID_32::GenUUID_32()
	{
		return UUID_32(s_distributor32(s_engine32));
	}

}