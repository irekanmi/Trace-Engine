

#include "AnimationPanel.h"
#include "../TraceEditor.h"
#include "scene/Entity.h"
#include "serialize/AnimationsSerializer.h"
#include "resource/AnimationsManager.h"
#include "animation/AnimationEngine.h"
#include "../utils/ImGui_utils.h"
#include "core/events/EventsSystem.h"
#include "core/input/Input.h"
#include "HierachyPanel.h"
#include "core/Utils.h"

#include "imgui.h"
#include "imgui_neo_sequencer.h"
#include "portable-file-dialogs.h"


namespace trace {

	AnimationPanel::AnimationPanel()
	{
	}
	bool AnimationPanel::Init()
	{
        m_state = State::PAUSE;

        trace::EventsSystem::get_instance()->AddEventListener(trace::EventType::TRC_BUTTON_PRESSED, BIND_EVENT_FN(AnimationPanel::OnEvent));
        trace::EventsSystem::get_instance()->AddEventListener(trace::EventType::TRC_BUTTON_RELEASED, BIND_EVENT_FN(AnimationPanel::OnEvent));
        trace::EventsSystem::get_instance()->AddEventListener(trace::EventType::TRC_KEY_RELEASED, BIND_EVENT_FN(AnimationPanel::OnEvent));
        trace::EventsSystem::get_instance()->AddEventListener(trace::EventType::TRC_KEY_PRESSED, BIND_EVENT_FN(AnimationPanel::OnEvent));

		return true;
	}
	void AnimationPanel::Shutdown()
	{
	}
    void AnimationPanel::OnEvent(Event* p_event)
    {
        switch (p_event->GetEventType())
        {
        case TRC_KEY_PRESSED:
        {
            trace::KeyPressed* press = reinterpret_cast<trace::KeyPressed*>(p_event);


            break;
        }
        case TRC_KEY_RELEASED:
        {
            trace::KeyReleased* release = reinterpret_cast<trace::KeyReleased*>(p_event);


            break;
        }
        case TRC_BUTTON_PRESSED:
        {
            trace::MousePressed* press = reinterpret_cast<trace::MousePressed*>(p_event);
            break;
        }
        case TRC_BUTTON_RELEASED:
        {
            trace::MouseReleased* release = reinterpret_cast<trace::MouseReleased*>(p_event);
            break;
        }
        case TRC_MOUSE_MOVE:
        {
            trace::MouseMove* move = reinterpret_cast<trace::MouseMove*>(p_event);
            break;
        }
        case TRC_MOUSE_DB_CLICK:
        {
            trace::MouseDBClick* click = reinterpret_cast<trace::MouseDBClick*>(p_event);
            break;
        }

        }

    }
	void AnimationPanel::Render(float deltaTime)
	{
        bool delete_pressed = InputSystem::get_instance()->GetKeyState(Keys::KEY_DELETE) == KeyState::KEY_RELEASE;
        bool control = InputSystem::get_instance()->GetKey(Keys::KEY_LCONTROL);
        bool key_D = InputSystem::get_instance()->GetKeyState(Keys::KEY_D) == KeyState::KEY_RELEASE;
        TraceEditor* editor = TraceEditor::get_instance();
        HierachyPanel* hierachy_panel = editor->GetHierachyPanel();
        Ref<Scene> scene = editor->GetEditScene();
        float clip_duration = m_currentClip ? m_currentClip->GetDuration() : 0.0f;
        bool has_entity = (current_entity_id != 0);

        AnimationComponent* anim_comp = nullptr;
        if (has_entity)
        {
            Entity current_entity = scene->GetEntity(current_entity_id);
            anim_comp = &current_entity.GetComponent<AnimationComponent>();
        }

        //TODO: Move to Update Function ===============

        if (m_currentClip)
        {
            clip_duration = m_currentClip->GetDuration();
            switch (m_state)
            {
            case State::PLAY:
            {
                m_elapsed += deltaTime;
                if (has_entity)
                {
                    AnimationEngine::get_instance()->Animate(m_currentClip, scene.get(), m_elapsed, anim_comp->entities);
                }
                m_currentFrame = int((m_elapsed / clip_duration) * (float)m_endFrame);
                m_currentFrame %= m_endFrame;
                m_elapsed = fmod(m_elapsed, clip_duration);
                break;
            }
            case State::PAUSE:
            {
                if (m_lastSelectedFrame != m_currentFrame)
                {
                    float t = ((float)m_currentFrame / (float)m_endFrame) * clip_duration;
                    if (has_entity)
                    {
                        AnimationEngine::get_instance()->Animate(m_currentClip, scene.get(), t, anim_comp->entities);
                    }
                    m_lastSelectedFrame = m_currentFrame;
                }
                break;
            }
            }
        }

        // ===========================================

		ImGui::Begin("Animation");
        ImGui::Columns(2);
        if (m_currentClip)
        {
            clip_duration = m_currentClip->GetDuration();
            std::string clip_name = m_currentClip->GetName();
            int sample_rate = m_currentClip->GetSampleRate();
            float duration = clip_duration;

            ImGui::Button(clip_name.c_str());
            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(".trcac"))
                {
                    static char buf[1024] = { 0 };
                    memcpy_s(buf, 1024, payload->Data, payload->DataSize);
                    std::filesystem::path p = buf;
                    Ref<AnimationClip> ac = AnimationsManager::get_instance()->GetClip(p.filename().string());
                    if (!ac)
                    {
                        ac = AnimationsSerializer::DeserializeAnimationClip(p.string());
                    }
                    if(ac)
                    {
                        
                        if (hierachy_panel->GetSelectedEntity())
                        {
                            SetAnimationClip(ac, hierachy_panel->GetSelectedEntity().GetID());
                        }
                        else
                        {
                            SetAnimationClip(ac, 0);
                        }
                    }
                }
                ImGui::EndDragDropTarget();
            }
            if (ImGui::InputInt("Sample Rate", &sample_rate))
            {
                m_currentClip->SetSampleRate(sample_rate);
            }
            if (ImGui::DragFloat("Duration(Seconds)", &duration, 0.25f, 0.005f))
            {
                float min = 1.0f / (float)m_currentClip->GetSampleRate();
                min += 0.005f;
                duration = glm::max(min, duration);
                m_currentClip->SetDuration(duration);
            }

            const char* type_string[] = { "Sequence", "Skeletal Animation" };
            const char* current_type = type_string[(int)m_currentClip->GetType()];
            if (ImGui::BeginCombo("Animation Clip Type", current_type))
            {
                for (int i = 0; i < 2; i++)
                {
                    bool selected = (current_type == type_string[i]);
                    if (ImGui::Selectable(type_string[i], selected))
                    {
                        m_currentClip->SetType((AnimationClipType)i);
                    }

                    if (selected)
                    {
                        ImGui::SetItemDefaultFocus();
                    }
                }

                ImGui::EndCombo();
            }

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
                else
                {
                    m_state = State::RECORD;
                }
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
                    if (!ac)
                    {
                        ac = AnimationsSerializer::DeserializeAnimationClip(p.string());
                    }
                    if (ac)
                    {

                        if (hierachy_panel->GetSelectedEntity())
                        {
                            SetAnimationClip(ac, hierachy_panel->GetSelectedEntity().GetID());
                        }
                        else
                        {
                            SetAnimationClip(ac, 0);
                        }
                    }
                    
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
                        if (path.extension() != ".trcac")
                        {
                            path = std::filesystem::path(result += ".trcac");
                        }
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

            m_startFrame = 0;
            m_endFrame = int((float)sample_rate * duration);
        }

        if (ImGui::BeginNeoSequencer("Sequencer", &m_currentFrame, &m_startFrame, &m_endFrame, { 0, 0 },
            ImGuiNeoSequencerFlags_EnableSelection |
            ImGuiNeoSequencerFlags_Selection_EnableDragging |
            ImGuiNeoSequencerFlags_AlwaysShowHeader|
            ImGuiNeoSequencerFlags_Selection_EnableDeletion))
        {
            
            if (m_currentClip && scene)
            {
                Ref<Scene> scene = editor->GetCurrentScene();
                float duration = m_currentClip->GetDuration();


                for (auto& channel : m_currentClip->GetTracks())
                {


                    const std::string& name = STRING_FROM_ID(channel.first);
                    static bool do_open = false;

                    if (!has_entity)
                    {
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.85f, 0.22f, 0.19f, 0.58f));
                    }
                    if (ImGui::BeginNeoGroup(name.c_str(), &do_open))
                    {
                        int track_index = 0;
                        static std::vector<int> tracks_to_delete{}; //TODO: Create a temp small vector
                        for (auto& track : channel.second)
                        {
                            bool open = true;
                            bool modified = false;

                            ImGuiID timeline_ID = ImGui::GetID(name.c_str());
                            ImGui::PushID(timeline_ID);
                            if (ImGui::BeginNeoTimelineEx(get_animation_data_type_string(track.first), &open, ImGuiNeoTimelineFlags_::ImGuiNeoTimelineFlags_AllowFrameChanging))
                            {
                                if (ImGui::IsNeoTimelineSelected())
                                {
                                    if (delete_pressed)
                                    {
                                        TRC_TRACE("Track: {} has been deleted", get_animation_data_type_string(track.first));
                                        tracks_to_delete.push_back(track_index);
                                    }
                                }
                                std::vector<FrameIndex>& f_idx = m_currentTracks[channel.first][track.first];
                                int frame_size = f_idx.size();
                                static std::vector<int> frame_to_delete{}; //TODO: Create a temp small vector
                                static std::vector<int> frame_to_duplicate{}; //TODO: Create a temp small vector
                                for (int i = 0; i < frame_size; i++)
                                {
                                    FrameIndex& fi = f_idx[i];
                                    ImGui::NeoKeyframe(&fi.index);
                                    float time_point = ((float)fi.index / (float)m_endFrame) * clip_duration;
                                    if (ImGui::NeoIsDraggingSelection() && ImGui::IsNeoKeyframeSelected())
                                    {
                                        track.second[fi.current_fd_index].time_point = time_point;
                                        bool begin = fi.current_fd_index == 0;
                                        bool end = fi.current_fd_index == (track.second.size() - 1);
                                        if (!begin && track.second[fi.current_fd_index - 1].time_point > time_point)
                                        {
                                            AnimationFrameData temp = track.second[fi.current_fd_index];
                                            track.second[fi.current_fd_index] = track.second[fi.current_fd_index - 1];
                                            track.second[fi.current_fd_index - 1] = temp;

                                            auto it = std::find_if(f_idx.begin(), f_idx.end(), [&fi](FrameIndex& a) {
                                                return a.current_fd_index == (fi.current_fd_index - 1);
                                                });
                                            it->current_fd_index++;
                                            fi.current_fd_index--;
                                        }
                                        if (!end && track.second[fi.current_fd_index + 1].time_point < time_point)
                                        {
                                            AnimationFrameData temp = track.second[fi.current_fd_index];
                                            track.second[fi.current_fd_index] = track.second[fi.current_fd_index + 1];
                                            track.second[fi.current_fd_index + 1] = temp;

                                            auto it = std::find_if(f_idx.begin(), f_idx.end(), [&fi](FrameIndex& a) {
                                                return a.current_fd_index == (fi.current_fd_index + 1);
                                                });
                                            it->current_fd_index--;
                                            fi.current_fd_index++;
                                        }
                                    }
                                    if (ImGui::IsNeoKeyframeHovered() && !ImGui::NeoIsDraggingSelection())
                                    {
                                        ImGui::SetNextWindowPos(ImGui::GetNeoKeyFramePos());
                                        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
                                        std::string str_id = "##NeoKeyFrame" + std::to_string(fi.current_fd_index) + name + get_animation_data_type_string(track.first);
                                        ImGui::Begin(str_id.c_str(), &open,
                                            ImGuiWindowFlags_NoDocking |
                                            ImGuiWindowFlags_NoNav |
                                            ImGuiWindowFlags_AlwaysAutoResize |
                                            ImGuiWindowFlags_NoTitleBar
                                        );
                                        animation_data_ui(track.first, track.second[fi.current_fd_index].data);
                                        ImGui::End();
                                        ImGui::PopStyleVar(1);
                                    }

                                    if (delete_pressed && ImGui::IsNeoKeyframeSelected())
                                    {
                                        frame_to_delete.push_back(i);
                                    }
                                    if (control && key_D && ImGui::IsNeoKeyframeSelected())
                                    {
                                        frame_to_duplicate.push_back(i);
                                    }
                                }
                                int delete_count = 0;//NOTE: Used to know the number of deleted frames
                                for (int i = 0; i < frame_to_delete.size(); i++)
                                {
                                    int& delete_index = frame_to_delete[i];
                                    delete_index -= delete_count;
                                    FrameIndex& fi = f_idx[delete_index];
                                   
                                    for (int j = delete_index + 1; j < f_idx.size(); j++)
                                    {
                                        f_idx[j].current_fd_index--;
                                    }

                                    track.second.erase(track.second.begin() + fi.current_fd_index);
                                    f_idx.erase(f_idx.begin() + delete_index);
                                    delete_count++;
                                }

                                int duplicate_count = 0;//NOTE: Used to know the number of duplicated frames
                                for (int i = 0; i < frame_to_duplicate.size(); i++)
                                {
                                    int& duplicate_index = frame_to_duplicate[i];
                                    duplicate_index += duplicate_count;
                                    FrameIndex& fi = f_idx[duplicate_index];
                                    FrameIndex fi_temp = fi;
                                    fi_temp.current_fd_index++;

                                    for (int j = duplicate_index + 1; j < f_idx.size(); j++)
                                    {
                                        f_idx[j].current_fd_index++;
                                    }

                                    AnimationFrameData anim_data = track.second[fi.current_fd_index];

                                    track.second.insert(track.second.begin() + fi.current_fd_index, anim_data);
                                    f_idx.insert(f_idx.begin() + duplicate_index, fi_temp);
                                    duplicate_count++;
                                }

                                frame_to_delete.clear();
                                frame_to_duplicate.clear();

                                ImGui::EndNeoTimeLine();
                            }
                            ImGui::PopID();
                            track_index++;

                            int delete_count = 0;//NOTE: Used to know the number of deleted frames
                            /*for (int i = 0; i < tracks_to_delete.size(); i++)
                            {
                                int& delete_index = tracks_to_delete[i];
                                delete_index -= delete_count;

                                channel.second.erase(channel.second.begin() + delete_index);

                            }*/

                            tracks_to_delete.clear();
                        }
                        ImGui::EndNeoGroup();
                    }
                    if (!has_entity)
                    {
                        ImGui::PopStyleColor();
                    }
                    
                    
                }


            }



            ImGui::EndNeoSequencer();
        }
		
        ImGui::Columns(1);
		ImGui::End();

	}
    void AnimationPanel::SetAnimationClip(Ref<AnimationClip> clip, UUID entity)
    {
        TraceEditor* editor = TraceEditor::get_instance();
        Ref<Scene> edit_scene = editor->GetEditScene();
        Entity object = edit_scene->GetEntity(entity);

        if (object && object.HasComponent<AnimationComponent>())
        {
            AnimationComponent& anim_comp = object.GetComponent<AnimationComponent>();

            if (anim_comp.GetAnimationGraph() && anim_comp.GetAnimationGraph()->HasAnimationClip(clip))
            {
                anim_comp.InitializeEntities(edit_scene.get(), object.GetID(), true);
                current_entity_id = object.GetID();
            }
        }

        if (m_currentClip && !clip->Compare(m_currentClip.get()))
        {
            m_currentTracks.clear();
        }
        m_currentClip = clip;
        generate_tracks();
        m_lastSelectedFrame = -1;

        m_startFrame = 0;
        int sample_rate = m_currentClip->GetSampleRate();
        float duration = m_currentClip->GetDuration();

        m_endFrame = int((float)sample_rate * duration);

        
    }
    bool AnimationPanel::Recording()
    {
        return m_state == State::RECORD;
    }
    void AnimationPanel::SetFrameData(UUID id, AnimationDataType type, void* data, int size)
    {
        /*if (!m_currentClip)
        {
            return;
        }
        TraceEditor* editor = TraceEditor::get_instance();

        Ref<Scene> scene = editor->GetEditScene();
        if (!scene)
        {
            return;
        }

        if (current_entity_id == 0)
        {
            return;
        }

        Entity entity = scene->GetEntity(id);
        Entity current_entity = scene->GetEntity(current_entity_id);

        switch (m_currentClip->GetType())
        {
        case AnimationClipType::SEQUENCE:
        {
            break;
        }
        case AnimationClipType::SKELETAL_ANIMATIOM:
        {
            if (entity == current_entity)
            {

            }
            else if (!scene->IsParent(current_entity, entity))
            {
                return;
            }
            break;
        }
        }

        std::vector<AnimationTrack>& tracks = m_currentClip->GetTracks()[entity.GetComponent<TagComponent>().GetTag()];
        auto it = std::find_if(tracks.begin(), tracks.end(), [&](AnimationTrack& t) {
            return t.channel_type == type;
            });

        int sample_rate = m_currentClip->GetSampleRate();
        float duration = m_currentClip->GetDuration();
        float current_time = ((float)m_currentFrame / (float)m_endFrame) * duration;

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

        generate_tracks();*/

    }
    void AnimationPanel::OnSceneLoad()
    {
        m_currentClip = Ref<AnimationClip>();
        current_entity_id = 0;
        m_currentTracks.clear();
    }
    void AnimationPanel::generate_tracks()
    {
        /*if (!m_currentClip)
        {
            return;
        }

        int sample_rate = m_currentClip->GetSampleRate();
        float duration = m_currentClip->GetDuration();

        m_endFrame = int((float)sample_rate * duration);

        for (auto& channel : m_currentClip->GetTracks())
        {
            for (AnimationTrack& track : channel.second)
            {
                m_currentTracks[channel.first][track.channel_type].clear();
                int i = 0;
                for (AnimationFrameData& fd : track.channel_data)
                {
                    FrameIndex fi;
                    fi.index = int((fd.time_point / duration) * (float)m_endFrame);
                    fi.current_fd_index = i;
                    m_currentTracks[channel.first][track.channel_type].push_back(fi);
                    ++i;
                }

            }
            
        }*/

    }
    void AnimationPanel::play()
    {
        m_elapsed = ((float)m_currentFrame / (float)m_endFrame) * m_currentClip->GetDuration();
    }
    void AnimationPanel::pause()
    {
        m_elapsed = 0.0f;
    }
    void AnimationPanel::animation_data_ui(AnimationDataType type, void* data)
    {

        switch (type)
        {
        case AnimationDataType::NONE:
        {
            ImGui::Text("None");

            break;
        }
        case AnimationDataType::POSITION:
        {
            glm::vec3 value;
            memcpy(&value, data, sizeof(glm::vec3));
            DrawVec3( value, "Position", 50.0f);

            break;
        }
        case AnimationDataType::ROTATION:
        {
            glm::quat value;
            memcpy(&value, data, sizeof(glm::quat));
            glm::vec3 euler = glm::degrees(glm::eulerAngles(value));
            DrawVec3( euler, "Rotation", 50.0f);

            break;
        }
        case AnimationDataType::SCALE:
        {
            glm::vec3 value;
            memcpy(&value, data, sizeof(glm::vec3));
            DrawVec3(value, "Scale", 50.0f);

            break;
        }
        case AnimationDataType::TEXT_INTENSITY:
        {
            float value;
            memcpy(&value, data, sizeof(float));
            ImGui::DragFloat("##Text Intensity", &value);

            break;
        }
        case AnimationDataType::LIGHT_INTENSITY:
        {
            float value;
            memcpy(&value, data, sizeof(float));
            ImGui::DragFloat("##Light Intensity", &value);

            break;
        }
        case AnimationDataType::IMAGE_COLOR:
        {
            uint32_t value;
            memcpy(&value, data, sizeof(uint32_t));
            ImVec4 color = ImGui::ColorConvertU32ToFloat4(value); 

            ImGui::ColorEdit4("##Base Color", &color.x, ImGuiColorEditFlags_Uint8);
            break;
        }
        }

    }
}