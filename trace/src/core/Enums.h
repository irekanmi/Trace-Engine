#pragma once


#define SAFE_DELETE(a, placeholder)	            \
	void* x_##placeholder = a;					\
	delete x_##placeholder;						\
	x_##placeholder = nullptr;                  \

#define KB 1024
#define MB KB * KB
#define GB MB * KB

#define BLOCK_SIZE KB
#define BIND_EVENT_FN(func) std::bind(&func, this, std::placeholders::_1)

#define _STR(x) #x

#define BIT(n) 1U << n
