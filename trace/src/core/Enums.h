#pragma once


#define SAFE_DELETE(a, placeholder)	            \
	void* x_##placeholder = a;					\
	delete x_##placeholder;						\
	x_##placeholder = nullptr;                  

#define KB 1024
#define MB KB * KB
#define GB MB * KB

#define BLOCK_SIZE KB
#define BIND_EVENT_FN(func) std::bind(&func, this, std::placeholders::_1)
#define BIND_RESOURCE_UNLOAD_FN(func, ptr) std::bind(&func, ptr, std::placeholders::_1)

#define INVAILD_ID 4294967295


#define _STR(x) #x

#define BIT(n) 1U << n
constexpr auto _NAME_ = "TRACE";
constexpr auto _ENGINE_NAME_ = "TRACE ENGINE";

#define TRC_CLAMP(value, low, high) (value <= low) ? low : ( value >= high ) ? high : value
#define TRC_HAS_FLAG(lhs, rhs) (lhs & rhs) == rhs ? true : false
