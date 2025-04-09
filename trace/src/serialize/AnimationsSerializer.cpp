#include "pch.h"

#include "AnimationsSerializer.h"
#include "core/FileSystem.h"
#include "resource/AnimationsManager.h"
#include "MemoryStream.h"
#include "resource/GenericAssetManager.h"
#include "core/Utils.h"
#include "animation/AnimationPoseNode.h"
#include "scene/UUID.h"
#include "scene/Entity.h"
#include "scene/Scene.h"
#include "reflection/SerializeTypes.h"

#include "yaml_util.h"

namespace trace {

	extern std::filesystem::path GetPathFromUUID(UUID uuid);
	extern UUID GetUUIDFromName(const std::string& name);

	bool AnimationsSerializer::SerializeAnimationClip(Ref<AnimationClip> clip, const std::string& file_path)
	{
		YAML::Emitter emit;

		emit << YAML::BeginMap;
		emit << YAML::Key << "Trace Version" << YAML::Value << "0.0.0.0";
		emit << YAML::Key << "Clip Version" << YAML::Value << "0.0.0.0";

		emit << YAML::Key << "Duration" << YAML::Value << clip->GetDuration();
		emit << YAML::Key << "Sample Rate" << YAML::Value << clip->GetSampleRate();
		emit << YAML::Key << "Clip Type" << YAML::Value << (int)clip->GetType();
		emit << YAML::Key << "Channels" << YAML::Value << YAML::BeginSeq;



		glm::vec4 anim_data; // NOTE: used to serialize animation frame data
		for (auto& uuid : clip->GetTracks())
		{

			emit << YAML::BeginMap;
			emit << YAML::Key << "Handle" << YAML::Value << STRING_FROM_ID(uuid.first);
			emit << YAML::Key << "Tracks" << YAML::Value << YAML::BeginSeq;
			for (auto& track : uuid.second)
			{
				emit << YAML::BeginMap;
				emit << YAML::Key << "Type" << YAML::Value << (int)track.first;
				emit << YAML::Key << "Track Data" << YAML::Value << YAML::BeginSeq;
				for (AnimationFrameData& fd : track.second)
				{
					emit << YAML::BeginMap;
					memcpy(&anim_data, fd.data, 16);
					emit << YAML::Key << "Data" << YAML::Value << anim_data;
					emit << YAML::Key << "Time Point" << YAML::Value << fd.time_point;

					emit << YAML::EndMap;
				}
				emit << YAML::EndSeq;
				emit << YAML::EndMap;
			}
			

			emit << YAML::EndSeq;
			emit << YAML::EndMap;
		}

		emit << YAML::EndSeq;
		RootMotionInfo& root_motion_info = clip->GetRootMotionInfo();
		emit << YAML::Key << "Has Root Motion" << YAML::Value << clip->HasRootMotion();
		emit << YAML::Key << "Y Motion" << YAML::Value << root_motion_info.Y_motion;
		emit << YAML::Key << "XZ Motion" << YAML::Value << root_motion_info.XZ_motion;
		emit << YAML::Key << "Rotation Motion" << YAML::Value << root_motion_info.enable_rotation;
		emit << YAML::Key << "Root Bone Index" << YAML::Value << root_motion_info.root_bone_index;
		emit << YAML::EndMap;

		FileHandle out_handle;
		if (FileSystem::open_file(file_path, FileMode::WRITE, out_handle))
		{
			FileSystem::writestring(out_handle, emit.c_str());
			FileSystem::close_file(out_handle);
		}

		return true;
	}

	bool AnimationsSerializer::SerializeAnimationClip(Ref<AnimationClip> clip, DataStream* stream)
	{
		std::string clip_name = clip->GetName();

		Reflection::Serialize(clip_name, stream, nullptr, Reflection::SerializationFormat::BINARY);
		Reflection::Serialize(*clip.get(), stream, nullptr, Reflection::SerializationFormat::BINARY);


		return true;
	}

	/*
	* Animation Clip
	*  '-> duration
	*  '-> sample_rate
	*  '-> group_count
	*   for each group
	*    '-> UUID
	*    '-> track_count
	*     for each track
	*      '-> channel_type
	*      '-> frame_data_count
	*       for each frame data
	*        '-> Animation Frame data
	*/
	bool AnimationsSerializer::SerializeAnimationClip(Ref<AnimationClip> clip, FileStream& stream, std::vector<std::pair<UUID, AssetHeader>>& map)
	{
		if (!clip)
		{
			TRC_ERROR("Invalid Clip, Function -> {}", __FUNCTION__);
			return false;
		}

		UUID id = GetUUIDFromName(clip->GetName());
		auto it = std::find_if(map.begin(), map.end(), [&id](std::pair<UUID, AssetHeader>& i)
			{
				return i.first == id;
			});

		//if (it == map.end())
		//{
		//	AssetHeader ast_h;
		//	ast_h.offset = stream.GetPosition();
		//	float duration = clip->GetDuration();
		//	stream.Write(duration);// Duration
		//	int sample_rate = clip->GetSampleRate();
		//	stream.Write(sample_rate);// Sample Rate

		//	std::unordered_map<UUID, std::vector<AnimationTrack>>& groups = clip->GetTracks();
		//	int group_count = groups.size();
		//	stream.Write(group_count);
		//	for (auto& i : groups)
		//	{
		//		uint64_t group_id = i.first;
		//		stream.Write(group_id);
		//		std::vector<AnimationTrack>& tracks = i.second;
		//		int track_count = tracks.size();
		//		stream.Write(track_count);
		//		for (auto& track : tracks)
		//		{
		//			int channel_type = (int)track.channel_type;
		//			stream.Write(channel_type);
		//			int frame_data_count = track.channel_data.size();
		//			stream.Write(frame_data_count);
		//			for (AnimationFrameData& fd : track.channel_data)
		//			{
		//				stream.Write<AnimationFrameData>(fd);
		//			}
		//		}
		//	}
		//	ast_h.data_size = stream.GetPosition() - ast_h.offset;

		//	map.push_back(std::make_pair(id, ast_h));


		//}

		return true;
	}

	Ref<AnimationClip> AnimationsSerializer::DeserializeAnimationClip(const std::string& file_path)
	{
		Ref<AnimationClip> result;

		std::filesystem::path p = file_path;
		Ref<AnimationClip> clip = AnimationsManager::get_instance()->GetClip(p.filename().string());
		if (clip)
		{
			TRC_WARN("{} has already been loaded", p.filename().string());
			return clip;
		}

		FileHandle in_handle;
		if (!FileSystem::open_file(file_path, FileMode::READ, in_handle))
		{
			TRC_ERROR("Unable to open file {}", file_path);
			return result;
		}
		std::string file_data;
		FileSystem::read_all_lines(in_handle, file_data);
		FileSystem::close_file(in_handle);

		YAML::Node data = YAML::Load(file_data);
		if (!data["Trace Version"] || !data["Clip Version"])
		{
			TRC_ERROR("These file is not a valid animation clip file {}", file_path);
			return result;
		}

		std::string trace_version = data["Trace Version"].as<std::string>(); // TODO: To be used later
		std::string clip_version = data["Clip Version"].as<std::string>(); // TODO: To be used later
		std::string clip_name = p.filename().string();
		
		
		clip = AnimationsManager::get_instance()->LoadClip_(file_path);

		float duration = data["Duration"].as<float>();
		int rate = data["Sample Rate"].as<int>();
		AnimationClipType type = static_cast<AnimationClipType>(data["Clip Type"].as<int>());
		clip->SetSampleRate(rate);
		clip->SetDuration(duration);
		clip->SetType(type);


		std::unordered_map<StringID, AnimationDataTrack>& clip_tracks = clip->GetTracks();
		glm::vec4 anim_data; // NOTE: used to serialize animation frame data
		for (auto& uuid : data["Channels"])
		{
			std::string id = uuid["Handle"].as<std::string>();
			for (auto& track : uuid["Tracks"])
			{
				//AnimationTrack new_track;
				//new_track.channel_type = (AnimationDataType)track["Type"].as<int>();
				AnimationDataType track_type = (AnimationDataType)track["Type"].as<int>();
				std::vector<AnimationFrameData> track_data;
				for (auto& frame_data : track["Track Data"])
				{
					AnimationFrameData fd;
					anim_data = frame_data["Data"].as<glm::vec4>();
					memcpy(fd.data, &anim_data, 16);
					fd.time_point = frame_data["Time Point"].as<float>();
					//new_track.channel_data.push_back(fd);

					track_data.push_back(fd);
				}
				//clip_tracks[id].push_back(new_track);
				auto& object_tracks = clip_tracks[STR_ID(id)];
				object_tracks[track_type] = std::move(track_data);
			}
		}

		if (data["Has Root Motion"])
		{
			clip->SetRootMotion(data["Has Root Motion"].as<bool>());
			RootMotionInfo& root_motion_info = clip->GetRootMotionInfo();
			root_motion_info.Y_motion = data["Y Motion"].as<bool>();
			root_motion_info.XZ_motion = data["XZ Motion"].as<bool>();
			root_motion_info.enable_rotation = data["Rotation Motion"].as<bool>();
			root_motion_info.root_bone_index = data["Root Bone Index"].as<uint32_t>();
		}

		result = clip;

		return result;
	}

	Ref<AnimationClip> AnimationsSerializer::DeserializeAnimationClip(DataStream* stream)
	{

		std::string clip_name;
		Reflection::Deserialize(clip_name, stream, nullptr, Reflection::SerializationFormat::BINARY);

		Ref<AnimationClip> clip = AnimationsManager::get_instance()->GetClip(clip_name);
		if (clip)
		{
			TRC_WARN("{} has already been loaded", clip_name);
			return clip;
		}

		clip = AnimationsManager::get_instance()->LoadClip_(clip_name);
		Reflection::Deserialize(*clip.get(), stream, nullptr, Reflection::SerializationFormat::BINARY);

		return clip;
	}

	void AnimationsSerializer::DeserializeAnimationClip(Ref<AnimationClip> clip, FileStream& stream, AssetHeader& header)
	{

		if (!clip)
		{
			TRC_WARN("Invalid Clip");
			return;
		}

		char* data = new char[header.data_size];// TODO: Use custom allocator

		stream.SetPosition(header.offset);
		stream.Read(data, header.data_size);

		MemoryStream mem_stream(data, header.data_size);
		float duration = 0.0f;
		mem_stream.Read<float>(duration);
		clip->SetDuration(duration);

		int sample_rate = 0;
		mem_stream.Read<int>(sample_rate);
		clip->SetSampleRate(sample_rate);


		int group_count = 0;
		mem_stream.Read<int>(group_count);

		//std::unordered_map<UUID, std::vector<AnimationTrack>>& groups = clip->GetTracks();

		//for (int i = 0; i < group_count; i++)
		//{
		//	UUID g_id = 0;
		//	mem_stream.Read<UUID>(g_id);
		//	int track_count = 0;
		//	mem_stream.Read<int>(track_count);
		//	std::vector<AnimationTrack> tracks;
		//	for (int j = 0; j < track_count; j++)
		//	{
		//		AnimationTrack track;
		//		int channel_type = -1;
		//		mem_stream.Read<int>(channel_type);
		//		track.channel_type = (AnimationDataType)channel_type;
		//		int frame_data_count = 0;
		//		mem_stream.Read<int>(frame_data_count);
		//		track.channel_data.resize(frame_data_count);

		//		mem_stream.Read(track.channel_data.data(), frame_data_count * sizeof(AnimationFrameData));
		//		tracks.emplace_back(track);
		//	}

		//	groups[g_id] = std::move(tracks);

		//}

		//delete[] data;// TODO: Use custom allocator


	}

	bool AnimationsSerializer::SerializeAnimationGraph(Ref<AnimationGraph> graph, const std::string& file_path)
	{
		YAML::Emitter emit;

		emit << YAML::BeginMap;
		emit << YAML::Key << "Trace Version" << YAML::Value << "0.0.0.0";
		emit << YAML::Key << "Anim Graph Version" << YAML::Value << "0.0.0.0";

		emit << YAML::Key << "States" << YAML::Value << YAML::BeginSeq;

		for (AnimationState& state : graph->GetStates())
		{
			emit << YAML::BeginMap;

			emit << YAML::Key << "Name" << YAML::Value << state.GetName();
			if(state.GetAnimationClip()) emit << YAML::Key << "Anim Clip" << YAML::Value << GetUUIDFromName(state.GetAnimationClip()->GetName());
			emit << YAML::Key << "Loop" << YAML::Value << state.GetLoop();
			emit << YAML::Key << "Anim State" << YAML::Value << (int)state.GetAnimState();

			emit << YAML::EndSeq;
		}

		emit << YAML::EndSeq;


		emit << YAML::EndMap;

		FileHandle out_handle;
		if (FileSystem::open_file(file_path, FileMode::WRITE, out_handle))
		{
			FileSystem::writestring(out_handle, emit.c_str());
			FileSystem::close_file(out_handle);
		}

		return true;
	}


	/*
	* Animation Graph
	*  '-> start_index
	*  '-> state_count
	*   for each state
	*    '-> name_size
	*    '-> name_data
	*    '-> clip_id
	*    '-> loop
	*/
	bool AnimationsSerializer::SerializeAnimationGraph(Ref<AnimationGraph> graph, FileStream& stream, std::vector<std::pair<UUID, AssetHeader>>& map)
	{
		if (!graph)
		{
			TRC_ERROR("Invalid Animation Graph, Function -> {}", __FUNCTION__);
			return false;
		}

		UUID id = GetUUIDFromName(graph->GetName());
		auto it = std::find_if(map.begin(), map.end(), [&id](std::pair<UUID, AssetHeader>& i)
			{
				return i.first == id;
			});

		if (it == map.end())
		{
			AssetHeader ast_h;
			ast_h.offset = stream.GetPosition();
			int32_t start_index = graph->GetStartIndex();
			stream.Write(start_index);

			std::vector<AnimationState>& states = graph->GetStates();
			
			int32_t states_count = static_cast<int32_t>(states.size());
			stream.Write(states_count);
			for (auto& s : states)
			{
				int32_t name_size = static_cast<int32_t>(s.GetName().length() + 1);
				stream.Write(name_size);
				stream.Write(s.GetName().data(), name_size);
				if (s.GetAnimationClip())
				{
					uint64_t clip_id = GetUUIDFromName(s.GetAnimationClip()->GetName());
					stream.Write(clip_id);
				}
				else
				{
					uint64_t clip_id = 0;
					stream.Write(clip_id);
				}
				bool loop = s.GetLoop();
				stream.Write(loop);

			}
			ast_h.data_size = stream.GetPosition() - ast_h.offset;

			map.push_back(std::make_pair(id, ast_h));
		}

		return true;
	}

	Ref<AnimationGraph> AnimationsSerializer::DeserializeAnimationGraph(const std::string& file_path)
	{
		Ref<AnimationGraph> result;

		FileHandle in_handle;
		if (!FileSystem::open_file(file_path, FileMode::READ, in_handle))
		{
			TRC_ERROR("Unable to open file {}", file_path);
			return result;
		}
		std::string file_data;
		FileSystem::read_all_lines(in_handle, file_data);
		FileSystem::close_file(in_handle);

		YAML::Node data = YAML::Load(file_data);
		if (!data["Trace Version"] || !data["Anim Graph Version"])
		{
			TRC_ERROR("These file is not a valid animation clip file {}", file_path);
			return result;
		}

		std::filesystem::path p = file_path;

		std::string trace_version = data["Trace Version"].as<std::string>(); // TODO: To be used later
		std::string graph_version = data["Anim Graph Version"].as<std::string>(); // TODO: To be used later
		std::string graph_name = p.filename().string();

		Ref<AnimationGraph> graph = AnimationsManager::get_instance()->GetGraph(graph_name);
		if (graph)
		{
			TRC_WARN("{} has already been loaded", graph_name);
			return graph;
		}
		graph = AnimationsManager::get_instance()->LoadGraph_(file_path);


		std::vector<AnimationState>& graph_states = graph->GetStates();
		for (auto& state : data["States"])
		{
			AnimationState new_state;
			new_state.SetAnimState((uint8_t)state["Anim State"].as<int>());
			new_state.SetLoop(state["Loop"].as<bool>());
			new_state.SetName(state["Name"].as<std::string>());
			if (state["Anim Clip"])
			{
				std::string clip_path = GetPathFromUUID(state["Anim Clip"].as<uint64_t>()).string();
				Ref<AnimationClip> clip = AnimationsSerializer::DeserializeAnimationClip(clip_path);
				new_state.SetAnimationClip(clip);
			}
			graph_states.push_back(new_state);

		}

		result = graph;

		return result;
	}

	void AnimationsSerializer::DeserializeAnimationGraph(Ref<AnimationGraph> graph, FileStream& stream, AssetHeader& header)
	{

		if (!graph)
		{
			TRC_WARN("Invalid Animation Graph");
			return;
		}

		char* data = new char[header.data_size];// TODO: Use custom allocator

		stream.SetPosition(header.offset);
		stream.Read(data, header.data_size);

		MemoryStream mem_stream(data, header.data_size);
		int32_t start_index = -1;
		mem_stream.Read(start_index);
		graph->SetStartIndex(start_index);

		int state_count = 0;
		mem_stream.Read<int>(state_count);

		std::vector<AnimationState>& states = graph->GetStates();
		char buf[512] = {0};// NOTE: Assuming that name_size would not be greater than 512
		for (int i = 0; i < state_count; i++)
		{
			AnimationState state;
			int name_size = 0;
			mem_stream.Read<int>(name_size);
			mem_stream.Read(buf, name_size);
			std::string& name = state.GetName();
			name = buf;

			UUID clip_id = 0;
			mem_stream.Read<UUID>(clip_id);
			Ref<AnimationClip> clip = AnimationsManager::get_instance()->LoadClip_Runtime(clip_id);
			state.SetAnimationClip(clip);

			bool loop = false;
			mem_stream.Read<bool>(loop);
			state.SetLoop(loop);

			states.emplace_back(state);

		}

		delete[] data;// TODO: Use custom allocator
	}

	// Skeleton -----------------------------------------------

	bool AnimationsSerializer::SerializeSkeleton(Ref<Animation::Skeleton> skeleton, const std::string& file_path)
	{
		if (!skeleton)
		{
			return false;
		}

		YAML::Emitter emit;

		emit << YAML::BeginMap;
		emit << YAML::Key << "Trace Version" << YAML::Value << "0.0.0.0";
		emit << YAML::Key << "Skeleton Version" << YAML::Value << "0.0.0.0";
		emit << YAML::Key << "Root Node" << YAML::Value << skeleton->GetRootNode();
		if (skeleton->GetHumanoidRig())
		{
			emit << YAML::Key << "Humanoid Rig" << YAML::Value << GetUUIDFromName(skeleton->GetHumanoidRig()->GetName());
		}

		emit << YAML::Key << "Bones" << YAML::Value << YAML::BeginSeq;


		for (Animation::Bone& bone : skeleton->GetBones())
		{

			emit << YAML::BeginMap;
			emit << YAML::Key << "Bone Name" << YAML::Value << bone.GetBoneName();
			emit << YAML::Key << "Bind Pose" << YAML::Value << bone.GetBindPose();
			emit << YAML::Key << "Bone Offset" << YAML::Value << bone.GetBoneOffset();
			emit << YAML::Key << "Parent Index" << YAML::Value << bone.GetParentIndex();
			emit << YAML::EndMap;
		}

		emit << YAML::EndSeq;
		emit << YAML::EndMap;

		FileHandle out_handle;
		if (FileSystem::open_file(file_path, FileMode::WRITE, out_handle))
		{
			FileSystem::writestring(out_handle, emit.c_str());
			FileSystem::close_file(out_handle);
		}

		return true;
	}

	bool AnimationsSerializer::SerializeSkeleton(Ref<Animation::Skeleton> skeleton, DataStream* stream)
	{
		if (!skeleton)
		{
			return false;
		}

		std::string skeleton_name = skeleton->GetName();
		Reflection::Serialize(skeleton_name, stream, nullptr, Reflection::SerializationFormat::BINARY);
		Reflection::Serialize(*skeleton.get(), stream, nullptr, Reflection::SerializationFormat::BINARY);


		return true;
	}

	Ref<Animation::Skeleton> AnimationsSerializer::DeserializeSkeleton(const std::string& file_path)
	{
		Ref<Animation::Skeleton> result;

		std::filesystem::path p = file_path;
		Ref<Animation::Skeleton> skeleton = GenericAssetManager::get_instance()->Get<Animation::Skeleton>(p.filename().string());
		if (skeleton)
		{
			TRC_WARN("{} has already been loaded", p.filename().string());
			return skeleton;
		}

		FileHandle in_handle;
		if (!FileSystem::open_file(file_path, FileMode::READ, in_handle))
		{
			TRC_ERROR("Unable to open file {}", file_path);
			return result;
		}
		std::string file_data;
		FileSystem::read_all_lines(in_handle, file_data);
		FileSystem::close_file(in_handle);

		YAML::Node data = YAML::Load(file_data);
		if (!data["Trace Version"] || !data["Skeleton Version"])
		{
			TRC_ERROR("These file is not a valid animation clip file {}", file_path);
			return result;
		}

		std::string trace_version = data["Trace Version"].as<std::string>(); // TODO: To be used later
		std::string skeleton_version = data["Skeleton Version"].as<std::string>(); // TODO: To be used later
		std::string skeleton_name = p.filename().string();
		std::string root_node = data["Root Node"].as<std::string>();

		

		std::vector<Animation::Bone> bones;
		for (auto& bone : data["Bones"])
		{
			std::string bone_name = bone["Bone Name"].as<std::string>();
			glm::mat4 bind_pose = bone["Bind Pose"].as<glm::mat4>();
			glm::mat4 bone_offset = bone["Bone Offset"].as<glm::mat4>();

			Animation::Bone bone_ins;
			bone_ins.Create(bone_name, bind_pose, bone_offset);
			if (bone["Parent Index"])
			{
				int32_t parent_index = bone["Parent Index"].as<int32_t>();
				bone_ins.SetParentIndex(parent_index);
			}

			bones.push_back(bone_ins);
		}

		result = GenericAssetManager::get_instance()->CreateAssetHandle_<Animation::Skeleton>(file_path);
		result->Create(skeleton_name, root_node,bones);

		if (data["Humanoid Rig"])
		{
			UUID file_id = data["Humanoid Rig"].as<UUID>();
			if (file_id != 0)
			{
				std::string file_path = GetPathFromUUID(file_id).string();
				Ref<Animation::HumanoidRig> rig = DeserializeHumanoidRig(file_path);
				result->SetHumanoidRig(rig);
			}
		}

		return result;
	}

	Ref<Animation::Skeleton> AnimationsSerializer::DeserializeSkeleton(DataStream* stream)
	{

		std::string skeleton_name;
		Reflection::Deserialize(skeleton_name, stream, nullptr, Reflection::SerializationFormat::BINARY);
		Ref<Animation::Skeleton> skeleton = GenericAssetManager::get_instance()->Get<Animation::Skeleton>(skeleton_name);
		if (skeleton)
		{
			TRC_WARN("{} has already been loaded", skeleton_name);
			return skeleton;
		}
		skeleton = GenericAssetManager::get_instance()->CreateAssetHandle_<Animation::Skeleton>(skeleton_name);
		Reflection::Deserialize(*skeleton.get(), stream, nullptr, Reflection::SerializationFormat::BINARY);

		return skeleton;
	}


	// Animation Graph ----------------------------------------------

	bool AnimationsSerializer::SerializeAnimGraph(Ref<Animation::Graph> graph, const std::string& file_path)
	{
		if (!graph)
		{
			return false;
		}

		YAML::Emitter emit;

		emit << YAML::BeginMap;
		emit << YAML::Key << "Trace Version" << YAML::Value << "0.0.0.0";
		emit << YAML::Key << "Anim_Graph Version" << YAML::Value << "0.0.0.0";

		Reflection::Serialize(*graph.get(), &emit, nullptr, Reflection::SerializationFormat::YAML);


		emit << YAML::EndMap;

		YAML::save_emitter_data(emit, file_path);


		return true;
	}


	bool AnimationsSerializer::SerializeAnimGraph(Ref<Animation::Graph> graph, DataStream* stream)
	{
		if (!graph)
		{
			TRC_TRACE("{}", __FUNCTION__);
			return false;
		}
		std::string graph_name = graph->GetName();
		Reflection::Serialize(graph_name, stream, nullptr, Reflection::SerializationFormat::BINARY);
		Reflection::Serialize(*graph.get(), stream, nullptr, Reflection::SerializationFormat::BINARY);
		return true;
	}

	Ref<Animation::Graph> AnimationsSerializer::DeserializeAnimGraph(const std::string& file_path)
	{
		Ref<Animation::Graph> result;

		std::filesystem::path p = file_path;
		Ref<Animation::Graph> graph = GenericAssetManager::get_instance()->Get<Animation::Graph>(p.filename().string());
		if (graph)
		{
			TRC_WARN("{} has already been loaded", p.filename().string());
			return graph;
		}

		YAML::Node data;
		if (!YAML::load_yaml_data(file_path, data))
		{
			return result;
		}

		if (!data["Trace Version"] || !data["Anim_Graph Version"])
		{
			TRC_ERROR("These file is not a valid animation clip file {}", file_path);
			return result;
		}

		std::string trace_version = data["Trace Version"].as<std::string>(); // TODO: To be used later
		std::string graph_version = data["Anim_Graph Version"].as<std::string>(); // TODO: To be used later
		std::string graph_name = p.filename().string();

		result = GenericAssetManager::get_instance()->CreateAssetHandle_<Animation::Graph>(file_path);

		Reflection::Deserialize(*result.get(), &data, nullptr, Reflection::SerializationFormat::YAML);


		return result;
	}

	Ref<Animation::Graph> AnimationsSerializer::DeserializeAnimGraph(DataStream* stream)
	{
		std::string graph_name;
		Reflection::Deserialize(graph_name, stream, nullptr, Reflection::SerializationFormat::BINARY);
		Ref<Animation::Graph> graph = GenericAssetManager::get_instance()->Get<Animation::Graph>(graph_name);
		if (graph)
		{
			TRC_WARN("{} has already been loaded", graph_name);
			return graph;
		}

		graph = GenericAssetManager::get_instance()->CreateAssetHandle_<Animation::Graph>(graph_name);
		Reflection::Deserialize(*graph.get(), stream, nullptr, Reflection::SerializationFormat::BINARY);

		return graph;
	}

	// Sequence -------------------------------------------------------
	bool AnimationsSerializer::SerializeSequence(Ref<Animation::Sequence> sequence, const std::string& file_path)
	{
		if (!sequence)
		{
			return false;
		}

		YAML::Emitter emit;

		emit << YAML::BeginMap;
		emit << YAML::Key << "Trace Version" << YAML::Value << "0.0.0.0";
		emit << YAML::Key << "Anim_Sequence Version" << YAML::Value << "0.0.0.0";

		Reflection::Serialize(*sequence.get(), &emit, nullptr, Reflection::SerializationFormat::YAML);


		emit << YAML::EndMap;

		YAML::save_emitter_data(emit, file_path);

		return true;
	}

	bool AnimationsSerializer::SerializeSequence(Ref<Animation::Sequence> sequence, DataStream* stream)
	{
		if (!sequence)
		{
			return false;
		}

		std::string sequence_name = sequence->GetName();
		Reflection::Serialize(sequence_name, stream, nullptr, Reflection::SerializationFormat::BINARY);
		Reflection::Serialize(*sequence.get(), stream, nullptr, Reflection::SerializationFormat::BINARY);

		return true;
	}

	Ref<Animation::Sequence> AnimationsSerializer::DeserializeSequence(const std::string& file_path)
	{
		Ref<Animation::Sequence> result;

		std::filesystem::path p = file_path;
		Ref<Animation::Sequence> sequence = GenericAssetManager::get_instance()->Get<Animation::Sequence>(p.filename().string());
		if (sequence)
		{
			TRC_WARN("{} has already been loaded", p.filename().string());
			return sequence;
		}

		YAML::Node data;
		if (!YAML::load_yaml_data(file_path, data))
		{
			return result;
		}

		if (!data["Trace Version"] || !data["Anim_Sequence Version"])
		{
			TRC_ERROR("These file is not a valid animation clip file {}", file_path);
			return result;
		}

		std::string trace_version = data["Trace Version"].as<std::string>(); // TODO: To be used later
		std::string sequence_version = data["Anim_Sequence Version"].as<std::string>(); // TODO: To be used later
		std::string sequence_name = p.filename().string();

		result = GenericAssetManager::get_instance()->CreateAssetHandle_<Animation::Sequence>(file_path);

		Reflection::Deserialize(*result.get(), &data, nullptr, Reflection::SerializationFormat::YAML);


		return result;
	}

	Ref<Animation::Sequence> AnimationsSerializer::DeserializeSequence(DataStream* stream)
	{
		std::string sequence_name;
		Reflection::Deserialize(sequence_name, stream, nullptr, Reflection::SerializationFormat::BINARY);
		Ref<Animation::Sequence> sequence = GenericAssetManager::get_instance()->Get<Animation::Sequence>(sequence_name);
		if (sequence)
		{
			TRC_WARN("{} has already been loaded", sequence_name);
			return sequence;
		}


		sequence = GenericAssetManager::get_instance()->CreateAssetHandle_<Animation::Sequence>(sequence_name);

		Reflection::Deserialize(*sequence.get(), stream, nullptr, Reflection::SerializationFormat::BINARY);

		return sequence;
	}

	// HumanoidRig -------------------------------------------------------
	bool AnimationsSerializer::SerializeHumanoidRig(Ref<Animation::HumanoidRig> humanoid_rig, const std::string& file_path)
	{
		if (!humanoid_rig)
		{
			return false;
		}

		YAML::Emitter emit;

		emit << YAML::BeginMap;
		emit << YAML::Key << "Trace Version" << YAML::Value << "0.0.0.0";
		emit << YAML::Key << "HumanoidRig Version" << YAML::Value << "0.0.0.0";

		Reflection::Serialize(*humanoid_rig.get(), &emit, nullptr, Reflection::SerializationFormat::YAML);


		emit << YAML::EndMap;

		YAML::save_emitter_data(emit, file_path);

		return true;
	}

	bool AnimationsSerializer::SerializeHumanoidRig(Ref<Animation::HumanoidRig> humanoid_rig, DataStream* stream)
	{
		if (!humanoid_rig)
		{
			return false;
		}

		std::string humanoid_rig_name = humanoid_rig->GetName();
		Reflection::Serialize(humanoid_rig_name, stream, nullptr, Reflection::SerializationFormat::BINARY);
		Reflection::Serialize(*humanoid_rig.get(), stream, nullptr, Reflection::SerializationFormat::BINARY);

		return true;
	}

	Ref<Animation::HumanoidRig> AnimationsSerializer::DeserializeHumanoidRig(const std::string& file_path)
	{
		Ref<Animation::HumanoidRig> result;

		std::filesystem::path p = file_path;
		Ref<Animation::HumanoidRig> humanoid_rig = GenericAssetManager::get_instance()->Get<Animation::HumanoidRig>(p.filename().string());
		if (humanoid_rig)
		{
			TRC_WARN("{} has already been loaded", p.filename().string());
			return humanoid_rig;
		}

		YAML::Node data;
		if (!YAML::load_yaml_data(file_path, data))
		{
			return result;
		}

		if (!data["Trace Version"] || !data["HumanoidRig Version"])
		{
			TRC_ERROR("These file is not a valid animation clip file {}", file_path);
			return result;
		}

		std::string trace_version = data["Trace Version"].as<std::string>(); // TODO: To be used later
		std::string humanoid_rig_version = data["HumanoidRig Version"].as<std::string>(); // TODO: To be used later
		std::string humanoid_rig_name = p.filename().string();

		result = GenericAssetManager::get_instance()->CreateAssetHandle_<Animation::HumanoidRig>(file_path);

		Reflection::Deserialize(*result.get(), &data, nullptr, Reflection::SerializationFormat::YAML);


		return result;
	}

	Ref<Animation::HumanoidRig> AnimationsSerializer::DeserializeHumanoidRig(DataStream* stream)
	{
		std::string humanoid_rig_name;
		Reflection::Deserialize(humanoid_rig_name, stream, nullptr, Reflection::SerializationFormat::BINARY);
		Ref<Animation::HumanoidRig> humanoid_rig = GenericAssetManager::get_instance()->Get<Animation::HumanoidRig>(humanoid_rig_name);
		if (humanoid_rig)
		{
			TRC_WARN("{} has already been loaded", humanoid_rig_name);
			return humanoid_rig;
		}


		humanoid_rig = GenericAssetManager::get_instance()->CreateAssetHandle_<Animation::HumanoidRig>(humanoid_rig_name);

		Reflection::Deserialize(*humanoid_rig.get(), stream, nullptr, Reflection::SerializationFormat::BINARY);

		return humanoid_rig;
	}


}