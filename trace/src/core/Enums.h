#pragma once


#define SAFE_DELETE(a, placeholder)	            \
	void* x_##placeholder = a;					\
	delete x_##placeholder;						\
	x_##placeholder = nullptr;                  \

#define KB 1024
#define MB KB * KB
#define GB MB * KB

#define BLOCK_SIZE KB

