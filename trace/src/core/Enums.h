#pragma once

#define MAKE_VERSION(a, b, c, d) (a << 24) | (b << 16) | (c << 8) | d
#define TRACE_VERSION MAKE_VERSION(0, 0, 0, 0)


#define SAFE_DELETE(a, placeholder)	            \
	void* x_##placeholder = a;					\
	delete x_##placeholder;						\
	x_##placeholder = nullptr;                  

#define KB 1024
#define MB KB * KB
#define GB MB * KB

#define BLOCK_SIZE KB
#define BIND_EVENT_FN(func)  [this](auto&&... args) -> decltype(auto){ return this->func(std::forward<decltype(args)>(args)...);} //std::bind(&func, this, std::placeholders::_1)
#define BIND_RENDER_COMMAND_FN(func) [this](auto&&... args) -> decltype(auto){ return this->func(std::forward<decltype(args)>(args)...);}
#define BIND_RESOURCE_UNLOAD_FN(func, ptr) std::bind(&func, ptr, std::placeholders::_1)

#define INVALID_ID 4294967295


#define _STR(x) #x

#define BIT(n) 1U << n
constexpr auto _NAME_ = "TRACE";
constexpr auto _ENGINE_NAME_ = "TRACE ENGINE";

#define TRC_CLAMP(value, low, high) (value <= low) ? low : ( value >= high ) ? high : value
#define TRC_HAS_FLAG(lhs, rhs) (lhs & rhs) == rhs ? true : false

#define TRC_COL32_R_SHIFT    0
#define TRC_COL32_G_SHIFT    8
#define TRC_COL32_B_SHIFT    16
#define TRC_COL32_A_SHIFT    24
#define TRC_COL32_A_MASK     0xFF000000
#define TRC_COL32(R,G,B,A)    (((uint32_t)(A)<<TRC_COL32_A_SHIFT) | ((uint32_t)(B)<<TRC_COL32_B_SHIFT) | ((uint32_t)(G)<<TRC_COL32_G_SHIFT) | ((uint32_t)(R)<<TRC_COL32_R_SHIFT))
#define TRC_COL32_WHITE       TRC_COL32(255,255,255,255)  // Opaque white = 0xFFFFFFFF
#define TRC_COL32_BLACK       TRC_COL32(0,0,0,255)        // Opaque black
#define TRC_COL32_BLACK_TRANS TRC_COL32(0,0,0,0)          // Transparent black = 0x00000000

//Temp ------
inline unsigned int get_alignment(unsigned int value, unsigned int alignment)
{
	return (value + (alignment - 1)) & ~(alignment - 1);
}
