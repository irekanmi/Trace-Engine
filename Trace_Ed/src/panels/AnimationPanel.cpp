


#include "AnimationPanel.h"
#include "../TraceEditor.h"
#include "scene/Entity.h"
#include "serialize/AnimationsSerializer.h"
#include "resource/AnimationsManager.h"
#include "animation/AnimationEngine.h"

#include "imgui.h"
#include "imgui_neo_sequencer.h"
#include "portable-file-dialogs.h"


namespace trace {

	AnimationPanel::AnimationPanel()
	{
	}
	bool AnimationPanel::Init()
	{
		m_editor = TraceEditor::get_instance();
        m_state = State::PAUSE;

		return true;
	}
	void AnimationPanel::Shutdown()
	{
	}
	void AnimationPanel::Render(float deltaTime)
	{
        Ref<Scene> scene = m_editor->m_currentScene;

        //TODO: Move to Update Function ===============

        if (m_currentClip && scene)
        {
            switch (m_state)
            {
            case State::PLAY:
            {
                m_elapsed += deltaTime;
                AnimationEngine::get_instance()->Animate(m_currentClip, scene.get(), m_elapsed);
                currentFrame = ((m_elapsed / m_currentClip->GetDuration()) * (float)endFrame);
                currentFrame %= endFrame;
                m_elapsed = fmod(m_elapsed, m_currentClip->GetDuration());
                break;
            }
            }
        }

        // ===========================================

		ImGui::Begin("Animation");
        ImGui::Columns(2);
        if (m_currentClip)
        {
            std::string clip_name = m_currentClip->GetName();
            int sample_rate = m_currentClip->GetSampleRate();
            float duration = m_currentClip->GetDuration();

            ImGui::Button(clip_name.c_str());
            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(".trcac"))
                {
                    static char buf[1024] = { 0 };
                    memcpy_s(buf, 1024, payload->Data, payload->DataSize);
                    std::filesystem::path p = buf;
                    Ref<AnimationClip> ac = AnimationsManager::get_instance()->GetClip(p.filename().string());
                    if (ac) m_currentClip = ac;
                    ac = AnimationsSerializer::DeserializeAnimationClip(p.string());
                    if (ac) m_currentClip = ac;
                }
                ImGui::EndDragDropTarget();
            }
            if (ImGui::InputInt("Sample Rate", &sample_rate)) m_currentClip->SetSampleRate(sample_rate);
            if (ImGui::InputFloat("Duration(Seconds)", &duration)) m_currentClip->SetDuration(duration);

            if (ImGui::Button("Play"))
            {
                m_state = State::PLAY;
                play();
            }
            ImGui::SameLine();
            if(ImGui::Button("Pause"))
            {
                m_state = State::PAUSE;
                pause();
            }
            ImGui::SameLine();
            if (ImGui::Button("Record"))
            {
                if (m_state == State::RECORD)
                {
                    m_state = State::PAUSE;
                }
                else m_state = State::RECORD;
            }

            if (ImGui::Button("Save"))
            {
                AnimationsSerializer::SerializeAnimationClip(m_currentClip, m_currentClip->m_path.string());
            }
        }
        else
        {
            float column_width = ImGui::GetColumnWidth();
            ImGui::Button("None(Animation Clip)", {column_width, 0.0f});
            if (ImGui::BeginItemTooltip())
            {
                ImGui::Text("Drag and drop the animation clip on the button");
                ImGui::EndTooltip();
            }
            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(".trcac"))
                {
                    static char buf[1024] = { 0 };
                    memcpy_s(buf, 1024, payload->Data, payload->DataSize);
                    std::filesystem::path p = buf;
                    Ref<AnimationClip> ac = AnimationsManager::get_instance()->GetClip(p.filename().string());
                    if (ac) m_currentClip = ac;
                    ac = AnimationsSerializer::DeserializeAnimationClip(p.string());
                    if (ac) m_currentClip = ac;
                    generate_tracks();
                }
                ImGui::EndDragDropTarget();
            }

            ImGui::SetCursorPosX(0.27f * column_width);
            if (ImGui::Button("Create Animation Clip"))
            {
                std::string result = pfd::save_file("New Animation", "", { "Trace Animation Clip", "*.trcac" }).result();
                if (!result.empty())
                {
                    std::filesystem::path path = result;
                    if (std::filesystem::exists(path))
                    {
                        m_currentClip = AnimationsSerializer::DeserializeAnimationClip(path.string());
                    }
                    else
                    {
                        if (path.extension() != ".trcac") path = std::filesystem::path(result += ".trcac");
                        Ref<AnimationClip> clip = AnimationsManager::get_instance()->CreateClip(path.filename().string());
                        AnimationsSerializer::SerializeAnimationClip(clip, path.string());
                        clip.free();
                    }
                    
                }
            }

        }

        ImGui::NextColumn();

        if (m_currentClip)
        {
            int sample_rate = m_currentClip->GetSampleRate();
            float duration = m_currentClip->GetDuration();

            startFrame = 0;
            endFrame = int((float)sample_rate * duration);
        }

        if (ImGui::BeginNeoSequencer("Sequencer", &currentFrame, &startFrame, &endFrame, { 0, 0 },
            ImGuiNeoSequencerFlags_EnableSelection |
            ImGuiNeoSequencerFlags_Selection_EnableDragging |
            ImGuiNeoSequencerFlags_AlwaysShowHeader|
            ImGuiNeoSequencerFlags_Selection_EnableDeletion))
        {
            
            if (m_currentClip && scene)
            {
                Ref<Scene> scene = m_editor->m_currentScene;
                float duration = m_currentClip->GetDuration();


                for (auto& channel : m_currentClip->GetTracks())
                {
                    std::string& name = scene->GetEntity(channel.first).GetComponent<TagComponent>()._tag;
                    static bool do_open = false;
                    if (ImGui::BeginNeoGroup(name.c_str(), &do_open))
                    {
                        for (AnimationTrack& track : channel.second)
                        {

                            bool open = true;
                            if (ImGui::BeginNeoTimelineEx(get_animation_data_type_string(track.channel_type), &open, ImGuiNeoTimelineFlags_::ImGuiNeoTimelineFlags_AllowFrameChanging))
                            {

                                for (int& index : m_currentTracks[channel.first][track.channel_type])
                                {
                                    ImGui::NeoKeyframe(&index);
                                }

                                ImGui::EndNeoTimeLine();
                            }

                        }
                        ImGui::EndNeoGroup();
                    }

                    
                    
                }


            }



            ImGui::EndNeoSequencer();
        }
		
        ImGui::Columns(1);
		ImGui::End();

	}
    void AnimationPanel::SetAnimationClip(Ref<AnimationClip> clip)
    {
        if (clip != m_currentClip) m_currentTracks.clear();
        m_currentClip = clip;
        generate_tracks();
    }
    bool AnimationPanel::Recording()
    {
        return m_state == State::RECORD;
    }
    void AnimationPanel::SetFrameData(UUID id, AnimationDataType type, void* data, int size)
    {
        if (!m_currentClip) return;
        std::vector<AnimationTrack>& tracks = m_currentClip->GetTracks()[id];
        auto it = std::find_if(tracks.begin(), tracks.end(), [&](AnimationTrack& t) {
            return t.channel_type == type;
            });

        int sample_rate = m_currentClip->GetSampleRate();
        float duration = m_currentClip->GetDuration();
        float current_time = ((float)currentFrame / (float)endFrame) * duration;

        if (it != tracks.end())
        {
            AnimationFrameData fd;
            fd.time_point = current_time;
            memcpy(fd.data, data, size);
            for (int i = it->channel_data.size() - 1; i >= 0; i--)
            {
                AnimationFrameData* current_fd = &it->channel_data[i];
                AnimationFrameData* prev = i != 0 ? &it->channel_data[i - 1] : nullptr;
                AnimationFrameData* next = i != it->channel_data.size() - 1 ? &it->channel_data[i + 1] : nullptr;


                if (current_time == current_fd->time_point)
                {
                    *current_fd = fd;

                    break;
                }
                else if (current_time < current_fd->time_point)
                {
                    if (prev && (current_time > prev->time_point) && (current_time != prev->time_point))
                    {
                        it->channel_data.insert(it->channel_data.begin() + i, fd);
                        break;
                    }
                    if (!prev)
                    {
                        it->channel_data.insert(it->channel_data.begin() + i, fd);
                        break;
                    }

                    
                }
                else if (current_time > current_fd->time_point)
                {
                    if (!next)
                    {
                        it->channel_data.push_back(fd);
                        break;
                    }
                }
                
                
            }

        }
        else
        {
            AnimationTrack new_track;
            new_track.channel_type = type;
            AnimationFrameData fd;
            fd.time_point = current_time;
            memcpy(fd.data, data, size);
            new_track.channel_data.push_back(fd);

            tracks.push_back(new_track);
        }

        generate_tracks();

    }
    void AnimationPanel::generate_tracks()
    {
        if (!m_currentClip) return;

        int sample_rate = m_currentClip->GetSampleRate();
        float duration = m_currentClip->GetDuration();

        endFrame = int((float)sample_rate * duration);

        for (auto& channel : m_currentClip->GetTracks())
        {
            for (AnimationTrack& track : channel.second)
            {
                m_currentTracks[channel.first][track.channel_type].clear();
                for (AnimationFrameData& fd : track.channel_data)
                {
                    int index = int((fd.time_point / duration) * (float)endFrame);
                    m_currentTracks[channel.first][track.channel_type].push_back(index);
                }

            }
        }

    }
    void AnimationPanel::play()
    {
        m_elapsed = ((float)currentFrame / (float)endFrame) * m_currentClip->GetDuration();
    }
    void AnimationPanel::pause()
    {
        m_elapsed = 0.0f;
    }
}