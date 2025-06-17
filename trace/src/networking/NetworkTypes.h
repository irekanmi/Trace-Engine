#pragma once

namespace trace::Network {

#define SERVER_PORT 2367
#define DISCOVERY_PORT 6932

	enum class NetType
	{
		UNKNOWN,
		CLIENT,
		LISTEN_SERVER
	};

	enum class RPCType
	{
		UNKNOW,
		CLIENT,
		SERVER
	};


}
