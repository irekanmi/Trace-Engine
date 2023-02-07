#include "pch.h"

#include "GShader.h"

namespace trace {
	GShader::GShader()
	{
	}
	GShader::~GShader()
	{
	}
	GShader* GShader::Create_(std::string& src)
	{
		return nullptr;
	}
	GShader* GShader::Create_(FileHandle& file)
	{
		return nullptr;
	}
}