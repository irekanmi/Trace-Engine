#include "pch.h"

#include "yaml_util.h"
#include "core/FileSystem.h"
#include "core/io/Logging.h"


namespace YAML {

	static void encode_vec4(Node& node, const glm::vec4& value)
	{
		node.push_back(value.x);
		node.push_back(value.y);
		node.push_back(value.z);
		node.push_back(value.w);
	}
	static void decode_vec4(const Node& node, glm::vec4& value, int start_index)
	{
		value.x = node[0 + start_index].as<float>();
		value.y = node[1 + start_index].as<float>();
		value.z = node[2 + start_index].as<float>();
		value.w = node[3 + start_index].as<float>();
	}

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
		encode_vec4(node, value);
		return node;
	}

	bool convert<glm::vec4>::decode(const Node& node, glm::vec4& value)
	{
		if (!node.IsSequence() || node.size() != 4)
			return false;

		decode_vec4(node, value, 0);
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


	Node convert<glm::mat4>::encode(const glm::mat4& value)
	{
		Node node;
		encode_vec4(node, value[0]);
		encode_vec4(node, value[1]);
		encode_vec4(node, value[2]);
		encode_vec4(node, value[3]);
		return node;
	}

	bool convert<glm::mat4>::decode(const Node& node, glm::mat4& value)
	{
		if (!node.IsSequence() || node.size() != 16)
			return false;

		decode_vec4(node, value[0], 0);
		decode_vec4(node, value[1], 4);
		decode_vec4(node, value[2], 8);
		decode_vec4(node, value[3], 12);
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

	Emitter& operator <<(Emitter& emit, const glm::mat4& value)
	{
		emit << Flow;
		emit << BeginSeq << value[0].x << value[0].y << value[0].z << value[0].w <<
			value[1].x << value[1].y << value[1].z << value[1].w <<
			value[2].x << value[2].y << value[2].z << value[2].w <<
			value[3].x << value[3].y << value[3].z << value[3].w << EndSeq;
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