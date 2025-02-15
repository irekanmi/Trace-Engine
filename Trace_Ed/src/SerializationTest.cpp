
#include "scene/UUID.h"
#include "scene/Entity.h"
#include "scene/Scene.h"
#include "core/FileSystem.h"
#include "reflection/TypeRegistry.h"
#include "reflection/SerializeTypes.h"
#include "serialize/FileStream.h"

#include "serialize/yaml_util.h"
#include <vector>
#include <map>

namespace trace {

	enum class EnumTest
	{
		Zero,
		One,
		Two,
		Three
	};

	struct Pos
	{
		float one = 10.0f;
		int two = 20;
		double three = 30.0;
		std::vector<int> many;

		GET_TYPE_ID
	};

	BEGIN_REGISTER_CLASS(Pos)
		REGISTER_TYPE(Pos);
		REGISTER_MEMBER(Pos, one);
		REGISTER_MEMBER(Pos, two);
		REGISTER_MEMBER(Pos, three);
		REGISTER_MEMBER(Pos, many);
	END_REGISTER_CLASS

	struct Rot : public Pos
	{
		float four;
		int five;
		double size;
		char data[16] = { 0 };
		Pos* pos_test = nullptr;
		std::map<UUID, glm::vec3> all;
		EnumTest enum_test{};
		std::pair<int, float> pair_test;

		GET_TYPE_ID
	};

	BEGIN_REGISTER_CLASS(Rot)
		REGISTER_TYPE_PARENT(Rot, Pos);
		REGISTER_MEMBER(Rot, four);
		REGISTER_MEMBER(Rot, five);
		REGISTER_MEMBER(Rot, size);
		REGISTER_MEMBER(Rot, all);
		REGISTER_MEMBER(Rot, data);
		REGISTER_MEMBER(Rot, pos_test);
		REGISTER_MEMBER(Rot, enum_test);
		REGISTER_MEMBER(Rot, pair_test);
	END_REGISTER_CLASS

	Reflection::RegisterTypeObject<std::vector<Pos*>> vec_pos_type;
	Reflection::RegisterTypeObject<std::map<int, Pos*>> map_pos_type;
	
	void SerializationTest()
	{
		YAML::Emitter emit;

		emit << YAML::BeginMap;
		emit << YAML::Key << "Trace Version" << YAML::Value << "0.0.0.0";
		emit << YAML::Key << "Reflection Version" << YAML::Value << "0.0.0.0";

		Pos pos;
		pos.many = { 6, 7, 8, 9, 10 };

		Rot p_test;
		Rot rot;
		rot.one = 51.0f;
		rot.two = 62;
		rot.three = 63.0;
		rot.four = 64.0f;
		rot.five = 65;
		rot.size = 66.0;
		rot.many = { 67, 68, 69, 70 };
		rot.all = { {71, glm::vec3(1.0f)}, {72, glm::vec3(2.0f)}, {73, glm::vec3(3.0f)} };
		rot.pos_test = &p_test;
		rot.pair_test = std::make_pair(13, 18.0f);
		Pos* rot_ptr = &rot;
		char value[16] = "0123456789ABCDE";
		//rot.data = value;
		memcpy(rot.data, value, 16);
		rot.enum_test = EnumTest::Three;

		void* _ptr = &rot_ptr;

		Pos*& ptr_result = *static_cast<Pos**>(_ptr);
		std::vector<Pos*> vec_ptr = { rot_ptr, &p_test };
		std::map<int, Pos*> map_ptr = { {7, rot_ptr}, {8, &p_test } };
		Reflection::SerializeContainer(map_ptr, &emit, nullptr ,Reflection::SerializationFormat::YAML);
		//FileStream stream("../Reflection_test.bin", FileMode::WRITE);
		//Reflection::SerializeContainer(map_ptr, &stream, nullptr ,Reflection::SerializationFormat::BINARY);

		emit << YAML::EndMap;

		FileHandle out_handle;
		if (FileSystem::open_file("../reflection_test.txt", FileMode::WRITE, out_handle))
		{
			FileSystem::writestring(out_handle, emit.c_str());
			FileSystem::close_file(out_handle);
		}



	}

	void DeserializationTest()
	{
		FileHandle in_handle;
		if (!FileSystem::open_file("../reflection_test.txt", FileMode::READ, in_handle))
		{
			TRC_ERROR("Unable to open file");
			return;
		}
		std::string file_data;
		FileSystem::read_all_lines(in_handle, file_data);
		FileSystem::close_file(in_handle);

		YAML::Node data = YAML::Load(file_data);

		Rot rot;
		std::vector<Pos*> vec_ptr;
		std::map<int, Pos*> map_ptr;
		Reflection::DeserializeContainer(map_ptr, &data, nullptr, Reflection::SerializationFormat::YAML);
		//FileStream stream("../Reflection_test.bin", FileMode::READ);
		//Reflection::DeserializeContainer(map_ptr, &stream, nullptr, Reflection::SerializationFormat::BINARY);

		return;
	}

	using namespace glm;
	REGISTER_TYPE(vec3);

}