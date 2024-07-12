
#include "Importer.h"
#include "../TraceEditor.h"

#include "assimp/Importer.hpp"
#include <filesystem>

namespace trace {
	bool Importer::ImportMeshFile(const std::string& file_path)
	{
		std::filesystem::path path(file_path);

		Assimp::Importer importer;

		return true;
	}
}
