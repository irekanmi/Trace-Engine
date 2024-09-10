#include "pch.h"

#include "AnimationsSerializer.h"
#include "core/FileSystem.h"
#include "resource/AnimationsManager.h"
#include "MemoryStream.h"

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
		emit << YAML::Key << "Clip Name" << YAML::Value << clip->GetName();

		emit << YAML::Key << "Duration" << YAML::Value << clip->GetDuration();
		emit << YAML::Key << "Sample Rate" << YAML::Value << clip->GetSampleRate();
		emit << YAML::Key << "Clip Type" << YAML::Value << (int)clip->GetType();
		emit << YAML::Key << "Channels" << YAML::Value << YAML::BeginSeq;

		glm::vec4 anim_data; // NOTE: used to serialize animation frame data
		for (auto& uuid : clip->GetTracks())
		{

			emit << YAML::BeginMap;
			emit << YAML::Key << "Handle" << YAML::Value << uuid.first;
			emit << YAML::Key << "Tracks" << YAML::Value << YAML::BeginSeq;
			for (auto& track : uuid.second)
			{
				emit << YAML::BeginMap;
				emit << YAML::Key << "Type" << YAML::Value << (int)track.channel_type;
				emit << YAML::Key << "Track Data" << YAML::Value << YAML::BeginSeq;
				for (AnimationFrameData& fd : track.channel_data)
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
		if (!data["Trace Version"] || !data["Clip Version"] || !data["Clip Name"])
		{
			TRC_ERROR("These file is not a valid animation clip file {}", file_path);
			return result;
		}

		std::string trace_version = data["Trace Version"].as<std::string>(); // TODO: To be used later
		std::string clip_version = data["Clip Version"].as<std::string>(); // TODO: To be used later
		std::string clip_name = data["Clip Name"].as<std::string>();
		
		Ref<AnimationClip> clip = AnimationsManager::get_instance()->GetClip(clip_name);
		if (clip)
		{
			TRC_WARN("{} has already been loaded", clip_name);
			return clip;
		}
		clip = AnimationsManager::get_instance()->LoadClip_(file_path);

		float duration = data["Duration"].as<float>();
		int rate = data["Sample Rate"].as<int>();
		AnimationClipType type = static_cast<AnimationClipType>(data["Clip Type"].as<int>());
		clip->SetSampleRate(rate);
		clip->SetDuration(duration);
		clip->SetType(type);

		std::unordered_map<std::string, std::vector<AnimationTrack>>& clip_tracks = clip->GetTracks();
		glm::vec4 anim_data; // NOTE: used to serialize animation frame data
		for (auto& uuid : data["Channels"])
		{
			std::string id = uuid["Handle"].as<std::string>();
			for (auto& track : uuid["Tracks"])
			{
				AnimationTrack new_track;
				new_track.channel_type = (AnimationDataType)track["Type"].as<int>();
				for (auto& frame_data : track["Track Data"])
				{
					AnimationFrameData fd;
					anim_data = frame_data["Data"].as<glm::vec4>();
					memcpy(fd.data, &anim_data, 16);
					fd.time_point = frame_data["Time Point"].as<float>();
					new_track.channel_data.push_back(fd);
				}
				clip_tracks[id].push_back(new_track);
			}
		}

		result = clip;

		return result;
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
		emit << YAML::Key << "Anim Graph Name" << YAML::Value << graph->GetName();

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
			
			int states_count = states.size();
			stream.Write(states_count);
			for (auto& s : states)
			{
				int name_size = s.GetName().length() + 1;
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
		if (!data["Trace Version"] || !data["Anim Graph Version"] || !data["Anim Graph Name"])
		{
			TRC_ERROR("These file is not a valid animation clip file {}", file_path);
			return result;
		}

		std::string trace_version = data["Trace Version"].as<std::string>(); // TODO: To be used later
		std::string graph_version = data["Anim Graph Version"].as<std::string>(); // TODO: To be used later
		std::string graph_name = data["Anim Graph Name"].as<std::string>();

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

}