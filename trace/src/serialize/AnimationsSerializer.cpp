#include "pch.h"

#include "AnimationsSerializer.h"
#include "core/FileSystem.h"
#include "resource/AnimationsManager.h"

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
		clip->SetSampleRate(rate);
		clip->SetDuration(duration);

		std::unordered_map<UUID, std::vector<AnimationTrack>>& clip_tracks = clip->GetTracks();
		glm::vec4 anim_data; // NOTE: used to serialize animation frame data
		for (auto& uuid : data["Channels"])
		{
			UUID id = uuid["Handle"].as<uint64_t>();
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

}