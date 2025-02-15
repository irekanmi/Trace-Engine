#pragma once

#define YAML_CPP_STATIC_DEFINE
#include "yaml-cpp/yaml.h"

#include "scene/UUID.h"

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"

namespace YAML {

	template<>
	struct convert<glm::vec2>
	{
		static Node encode(const glm::vec2& value);
		static bool decode(const Node& node, glm::vec2& value);
	};

	template<>
	struct convert<glm::ivec2>
	{
		static Node encode(const glm::ivec2& value);
		static bool decode(const Node& node, glm::ivec2& value);
	};

	template<>
	struct convert<glm::vec3>
	{
		static Node encode(const glm::vec3& value);
		static bool decode(const Node& node, glm::vec3& value);
	};

	template<>
	struct convert<glm::ivec3>
	{
		static Node encode(const glm::ivec3& value);
		static bool decode(const Node& node, glm::ivec3& value);
	};

	template<>
	struct convert<glm::vec4>
	{
		static Node encode(const glm::vec4& value);
		static bool decode(const Node& node, glm::vec4& value);
	};

	template<>
	struct convert<glm::ivec4>
	{
		static Node encode(const glm::ivec4& value);
		static bool decode(const Node& node, glm::ivec4& value);
	};

	template<>
	struct convert<glm::mat4>
	{
		static Node encode(const glm::mat4& value);
		static bool decode(const Node& node, glm::mat4& value);
	};

	template<>
	struct convert<glm::quat>
	{
		static Node encode(const glm::quat& value);
		static bool decode(const Node& node, glm::quat& value);
	};

	template<>
	struct convert<trace::UUID>
	{
		static Node encode(const trace::UUID& value);
		static bool decode(const Node& node, trace::UUID& value);
	};

	Emitter& operator <<(Emitter& emit, const glm::vec2& value);

	Emitter& operator <<(Emitter& emit, const glm::ivec2& value);

	Emitter& operator <<(Emitter& emit, const glm::vec3& value);

	Emitter& operator <<(Emitter& emit, const glm::ivec3& value);

	Emitter& operator <<(Emitter& emit, const glm::vec4& value);

	Emitter& operator <<(Emitter& emit, const glm::ivec4& value);

	Emitter& operator <<(Emitter& emit, const glm::mat4& value);

	Emitter& operator <<(Emitter& emit, const glm::quat& value);

	Emitter& operator <<(Emitter& emit, const trace::UUID& value);

	bool save_emitter_data(Emitter& emit, const std::string& file_path);
	bool load_yaml_data(const std::string& file_path, Node& out_data);
	
}
