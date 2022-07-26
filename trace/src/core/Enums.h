#pragma once

#define SAFE_DELETE(a, placeholder)	\
	void* x_##placeholder = a;					\
	delete x_##placeholder;						\
	x_##placeholder = nullptr;

#define BLOCK_SIZE 1024
