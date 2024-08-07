#include "pch.h"

#include "yaml_util.h"
#include "core/FileSystem.h"
#include "core/io/Logging.h"


namespace YAML {

	Node convert<glm::vec2>::encode(const glm::vec2& value)
	{
		Node node;
		node.push_back(value.x);
		node.push_back(value.y);
		return node;
	}

	bool convert<glm::vec2>::decode(const Node& node, glm::vec2& value)
	{
		if (!node.IsSequence() || node.size() != 2)
			return false;

		value.x = node[0].as<float>();
		value.y = node[1].as<float>();
		return true;
	}



	Node convert<glm::ivec2>::encode(const glm::ivec2& value)
	{
		Node node;
		node.push_back(value.x);
		node.push_back(value.y);
		return node;
	}

	bool convert<glm::ivec2>::decode(const Node& node, glm::ivec2& value)
	{
		if (!node.IsSequence() || node.size() != 2)
			return false;

		value.x = node[0].as<int>();
		value.y = node[1].as<int>();
		return true;
	}



	Node convert<glm::vec3>::encode(const glm::vec3& value)
	{
		Node node;
		node.push_back(value.x);
		node.push_back(value.y);
		node.push_back(value.z);
		return node;
	}

	bool convert<glm::vec3>::decode(const Node& node, glm::vec3& value)
	{
		if (!node.IsSequence() || node.size() != 3)
			return false;

		value.x = node[0].as<float>();
		value.y = node[1].as<float>();
		value.z = node[2].as<float>();
		return true;
	}



	Node convert<glm::ivec3>::encode(const glm::ivec3& value)
	{
		Node node;
		node.push_back(value.x);
		node.push_back(value.y);
		node.push_back(value.z);
		return node;
	}

	bool convert<glm::ivec3>::decode(const Node& node, glm::ivec3& value)
	{
		if (!node.IsSequence() || node.size() != 3)
			return false;

		value.x = node[0].as<int>();
		value.y = node[1].as<int>();
		value.z = node[2].as<int>();
		return true;
	}

	Node convert<glm::vec4>::encode(const glm::vec4& value)
	{
		Node node;
		node.push_back(value.x);
		node.push_back(value.y);
		node.push_back(value.z);
		node.push_back(value.w);
		return node;
	}

	bool convert<glm::vec4>::decode(const Node& node, glm::vec4& value)
	{
		if (!node.IsSequence() || node.size() != 4)
			return false;

		value.x = node[0].as<float>();
		value.y = node[1].as<float>();
		value.z = node[2].as<float>();
		value.w = node[3].as<float>();
		return true;
	}



	Node convert<glm::ivec4>::encode(const glm::ivec4& value)
	{
		Node node;
		node.push_back(value.x);
		node.push_back(value.y);
		node.push_back(value.z);
		node.push_back(value.w);
		return node;
	}

	bool convert<glm::ivec4>::decode(const Node& node, glm::ivec4& value)
	{
		if (!node.IsSequence() || node.size() != 4)
			return false;

		value.x = node[0].as<int>();
		value.y = node[1].as<int>();
		value.z = node[2].as<int>();
		value.w = node[3].as<int>();
		return true;
	}



	Node convert<glm::quat>::encode(const glm::quat& value)
	{
		Node node;
		node.push_back(value.x);
		node.push_back(value.y);
		node.push_back(value.z);
		node.push_back(value.w);
		return node;
	}

	bool convert<glm::quat>::decode(const Node& node, glm::quat& value)
	{
		if (!node.IsSequence() || node.size() != 4)
			return false;

		value.x = node[0].as<float>();
		value.y = node[1].as<float>();
		value.z = node[2].as<float>();
		value.w = node[3].as<float>();
		return true;
	}


	Emitter& operator <<(Emitter& emit, const glm::vec2& value)
	{
		emit << Flow;
		emit << BeginSeq << value.x << value.y << EndSeq;
		return emit;
	}

	Emitter& operator <<(Emitter& emit, const glm::ivec2& value)
	{
		emit << Flow;
		emit << BeginSeq << value.x << value.y << EndSeq;
		return emit;
	}

	Emitter& operator <<(Emitter& emit, const glm::vec3& value)
	{
		emit << Flow;
		emit << BeginSeq << value.x << value.y << value.z << EndSeq;
		return emit;
	}

	Emitter& operator <<(Emitter& emit, const glm::ivec3& value)
	{
		emit << Flow;
		emit << BeginSeq << value.x << value.y << value.z << EndSeq;
		return emit;
	}

	Emitter& operator <<(Emitter& emit, const glm::vec4& value)
	{
		emit << Flow;
		emit << BeginSeq << value.x << value.y << value.z << value.w << EndSeq;
		return emit;
	}

	Emitter& operator <<(Emitter& emit, const glm::ivec4& value)
	{
		emit << Flow;
		emit << BeginSeq << value.x << value.y << value.z << value.w << EndSeq;
		return emit;
	}

	Emitter& operator <<(Emitter& emit, const glm::quat& value)
	{
		emit << Flow;
		emit << BeginSeq << value.x << value.y << value.z << value.w << EndSeq;
		return emit;
	}

	bool save_emitter_data(Emitter& emit, const std::string& file_path)
	{
		trace::FileHandle out_handle;
		if (trace::FileSystem::open_file(file_path, trace::FileMode::WRITE, out_handle))
		{
			trace::FileSystem::writestring(out_handle, emit.c_str());
			trace::FileSystem::close_file(out_handle);

			return true;
		}

		return false;
	}

	bool load_yaml_data(const std::string& file_path, Node& out_data)
	{
		trace::FileHandle in_handle;
		if (!trace::FileSystem::open_file(file_path, trace::FileMode::READ, in_handle))
		{
			TRC_ERROR("Unable to open file {}", file_path);
			return false;
		}
		std::string file_data;
		trace::FileSystem::read_all_lines(in_handle, file_data);
		trace::FileSystem::close_file(in_handle);

		out_data = YAML::Load(file_data);

		return true;
	}

}