
#include "AnimationSequencer.h"
#include "core/events/EventsSystem.h"
#include "core/io/Logging.h"
#include "scene/UUID.h"
#include "scene/Entity.h"
#include "scene/Scene.h"
#include "../TraceEditor.h"
#include "core/input/Input.h"
#include "animation/AnimationSequenceTrack.h"
#include "animation/SequenceTrackChannel.h"
#include "../TraceEditor.h"
#include "serialize/AnimationsSerializer.h"
#include "core/Utils.h"
#include "AnimationPanel.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_stdlib.h"
#include "imgui_internal.h"
#include "../utils/ImGui_utils.h"

#include <algorithm>
#include <set>
#include <unordered_map>

namespace trace {

    //- Adding track
    //- Removing track
    //- Removing channels

    extern std::filesystem::path GetPathFromUUID(UUID uuid);
    extern UUID GetUUIDFromName(const std::string& name);

    struct track_ui_info
    {
        ImU32 color;
        bool can_blend = false;
        std::function<void(Animation::SequenceTrack*,uint32_t index)> item_on_render;
        std::function<void(Animation::SequenceTrack*,uint32_t index, std::string& name)> item_get_name;
        std::function<Animation::SequenceTrackChannel*(Animation::SequenceTrack*, ImRect rect, ImGuiID id)> item_on_drag_drop;

    };

    static std::unordered_map<Animation::SequenceTrackType, track_ui_info> render_table;

    static int32_t rect_id = 0;
    static int32_t selected_track_index = -1;
    static int32_t selected_clip_index = -1;

    float timelineStart = 0.0f;
    float timelineEnd = 10.0f;
    float pixelsPerSecond = 100.0f;  // Initial zoom level
    float currentTime = 0.0f;        // Position of the playhead
    float lastTime = 0.0f;
    bool isPlaying = false;
    // Variables for playhead dragging
    bool isDraggingPlayhead = false;


    float minTimeStep = 0.005f; // Minimum time step when fully zoomed in

    float minClipDuration = 0.1f; // Minimum clip duration to prevent it from shrinking too much

    // Blending indicator parameters
    float blendLineSpacing = 10.0f; // Space between diagonal lines in the blending region
    ImU32 blendColor = IM_COL32(255, 0, 0, 200); // Semi-transparent white for blending indicator

    //Shadow parameters
    ImVec2 shadowOffset = ImVec2(3.0f, 3.0f);
    ImU32 shadowColor = IM_COL32(0, 0, 0, 150);

    std::set<uint32_t> modified_tracks;

    static void sort_tracks(std::vector<Animation::SequenceTrack*>& tracks)
    {
        
        for (const uint32_t& index : modified_tracks)
        {
            Animation::SequenceTrack* track = tracks[index];
            std::sort(track->GetTrackChannels().begin(), track->GetTrackChannels().end(), [](Animation::SequenceTrackChannel* a, Animation::SequenceTrackChannel* b)
                {
                    return a->GetStartTime() < b->GetStartTime();
                });
        }

        modified_tracks.clear();
    }
    float get_pixels_per_second(float window_width, float duration)
    {
        return window_width * (1.0f / duration);
    }

	bool AnimationSequencer::Init()
	{
		trace::EventsSystem::get_instance()->AddEventListener(trace::EventType::TRC_BUTTON_RELEASED, BIND_EVENT_FN(AnimationSequencer::OnEvent));

        track_ui_info& anim_track = render_table[Animation::SequenceTrackType::ANIMATION_TRACK];
        anim_track.color = IM_COL32(150, 85, 77, 200);
        anim_track.can_blend = false;
        anim_track.item_on_render = [](Animation::SequenceTrack* track, uint32_t index)
        {

            Animation::AnimationChannel* anim = (Animation::AnimationChannel*)track->GetTrackChannels()[index];
            std::string name = "None (Animation Clip)";
            if (anim->GetAnimationClip())
            {
                name = anim->GetAnimationClip()->GetName();
            }

            ImGui::Text("Animation Clip: ");
            ImGui::SameLine();
            ImGui::Button(name.c_str());
            Ref<AnimationClip> clip = ImGuiDragDropResource<AnimationClip>(".trcac");
            if (clip)
            {
                anim->SetAnimationClip(clip);
                anim->SetDuration(clip->GetDuration());
            }
        };
        anim_track.item_get_name = [](Animation::SequenceTrack* track, uint32_t index, std::string& name)
        {
            Animation::AnimationChannel* anim = (Animation::AnimationChannel*)track->GetTrackChannels()[index];
            if (anim->GetAnimationClip())
            {
                name = anim->GetAnimationClip()->GetName();
            }
        };
        anim_track.item_on_drag_drop = [](Animation::SequenceTrack* track, ImRect rect, ImGuiID id) -> Animation::SequenceTrackChannel*
        {
            Ref<AnimationClip> clip = ImGuiDragDropResourceCustom<AnimationClip>(rect, id, ".trcac");
            if (clip && clip->GetType() == AnimationClipType::SEQUENCE)
            {
                Animation::AnimationChannel* anim = new Animation::AnimationChannel;
                anim->SetAnimationClip(clip);
                anim->SetDuration(clip->GetDuration());
                track->GetTrackChannels().emplace_back(anim);
                return anim;
            }
            
            return nullptr;
        };

        track_ui_info& skeletal_track = render_table[Animation::SequenceTrackType::SKELETAL_ANIMATION_TRACK];
        skeletal_track.color = IM_COL32(77, 85, 150, 200);
        skeletal_track.can_blend = true;
        skeletal_track.item_on_render = [](Animation::SequenceTrack* track, uint32_t index)
        {

            Animation::SkeletalAnimationChannel* anim = (Animation::SkeletalAnimationChannel*)track->GetTrackChannels()[index];
            std::string name = "None (Animation Clip)";
            if (anim->GetAnimationClip())
            {
                name = anim->GetAnimationClip()->GetName();
            }

            ImGui::Text("Animation Clip: ");
            ImGui::SameLine();
            ImGui::Button(name.c_str());
            Ref<AnimationClip> clip = ImGuiDragDropResource<AnimationClip>(".trcac");
            if (clip)
            {
                anim->SetAnimationClip(clip);
                anim->SetDuration(clip->GetDuration());
            }
        };
        skeletal_track.item_get_name = anim_track.item_get_name;
        skeletal_track.item_on_drag_drop = [](Animation::SequenceTrack* track, ImRect rect, ImGuiID id) -> Animation::SequenceTrackChannel*
        {
            Ref<AnimationClip> clip = ImGuiDragDropResourceCustom<AnimationClip>(rect, id, ".trcac");
            if (clip && clip->GetType() == AnimationClipType::SKELETAL_ANIMATIOM)
            {
                Animation::SkeletalAnimationChannel* anim = new Animation::SkeletalAnimationChannel;
                anim->SetAnimationClip(clip);
                anim->SetDuration(clip->GetDuration());
                track->GetTrackChannels().emplace_back(anim);
                return anim;
            }

            return nullptr;
        };

        track_ui_info& active_track = render_table[Animation::SequenceTrackType::ACTIVATION_TRACK];
        active_track.color = IM_COL32(85, 150, 77, 200);
        active_track.can_blend = false;
        active_track.item_on_render = [](Animation::SequenceTrack* track, uint32_t index)
        {
           
        };
        active_track.item_get_name = [](Animation::SequenceTrack* track, uint32_t index, std::string& name)
        {
        };
        active_track.item_on_drag_drop = [](Animation::SequenceTrack* track, ImRect rect, ImGuiID id) -> Animation::SequenceTrackChannel*
        {
            return nullptr;
        };

		return true;
	}

	void AnimationSequencer::Shutdown()
	{
	}

	void AnimationSequencer::Update(float deltaTime)
	{
        

	}

	void AnimationSequencer::Render(float deltaTime)
	{
        //TraceEditor* editor = TraceEditor::get_instance();
        //Ref<Animation::Sequence> sequence = m_instance.GetSequence();

        //ImGui::Begin("Animation Sequencer");
        //ImGui::Columns(2);

        //std::string name = "None (Animation Sequence)";
        //float sequence_duration = 10.0f;

        //if (sequence)
        //{
        //    name = sequence->GetName();
        //    sequence_duration = sequence->GetDuration();
        //    ImGui::Text("Animation Sequence: ");
        //    ImGui::SameLine();
        //    ImGui::Button(name.c_str());
        //    Ref<Animation::Sequence> seq = ImGuiDragDropResource<Animation::Sequence>(".trcsq");
        //    if (seq)
        //    {
        //        SetAnimationSequence(seq);
        //        sequence = seq;
        //    }

        //    if (ImGui::Button("Add Track"))
        //    {
        //        ImGui::OpenPopup("Add Track");
        //    }

        //    if (ImGui::BeginPopup("Add Track"))
        //    {
        //        if (ImGui::MenuItem("Animation track"))
        //        {
        //            sequence->GetTracks().push_back(new Animation::AnimationSequenceTrack);//TODO: Use custom allocator
        //        }
        //        if (ImGui::MenuItem("Skeletal Animation track"))
        //        {
        //            sequence->GetTracks().push_back(new Animation::SkeletalAnimationTrack);//TODO: Use custom allocator
        //        }
        //        if (ImGui::MenuItem("Activation track"))
        //        {
        //            sequence->GetTracks().push_back(new Animation::ActivationTrack);//TODO: Use custom allocator
        //        }
        //        ImGui::EndPopup();
        //    }
        //    float duration = sequence->GetDuration();
        //    if (ImGui::DragFloat("Duration", &duration, 0.25f))
        //    {
        //        sequence->SetDuration(duration);
        //        timelineEnd = duration;
        //    }

        //    // Playback controls
        //    if (ImGui::Button("Play"))
        //    {
        //        isPlaying = true;
        //    }
        //    ImGui::SameLine();
        //    if (ImGui::Button("Pause"))
        //    {
        //        isPlaying = false;
        //    }
        //    ImGui::SameLine();
        //    if (ImGui::Button("Stop")) 
        //    { 
        //        isPlaying = false;
        //    }

        //    /*if (!isPlaying && lastTime != currentTime && editor->GetEditorState() != EditorState::ScenePlay)
        //    {
        //        m_instance.UpdateElaspedTime(editor->GetCurrentScene().get(), currentTime);
        //        lastTime = currentTime;
        //    }

        //    if (isPlaying && editor->GetEditorState() != EditorState::ScenePlay)
        //    {
        //        currentTime += deltaTime;
        //        currentTime = fmod(currentTime, sequence->GetDuration());
        //        m_instance.UpdateElaspedTime(editor->GetCurrentScene().get(), currentTime);
        //    }*/
        //    

        //    if (ImGui::Button("Save"))
        //    {                
        //        AnimationsSerializer::SerializeSequence(sequence, GetPathFromUUID(GetUUIDFromName(sequence->GetName())).string());
        //    }

        //    ImGui::BeginChild("##Selected Item");

        //    if (selected_clip_index >= 0 && selected_track_index >= 0 && sequence->GetTracks().size() > selected_track_index)
        //    {
        //        Animation::SequenceTrack* track = sequence->GetTracks()[selected_track_index];
        //        if (track->GetTrackChannels().size() > selected_clip_index)
        //        {
        //            Animation::SequenceTrackChannel* channel = track->GetTrackChannels()[selected_clip_index];

        //            float channel_start_time = channel->GetStartTime();
        //            float channel_duration = channel->GetDuration();
        //            if (ImGui::DragFloat("Start Time", &channel_start_time))
        //            {
        //                channel->SetStartTime(channel_start_time);
        //            }
        //            if (ImGui::DragFloat("Duration", &channel_duration))
        //            {
        //                channel->SetDuration(channel_duration);
        //            }

        //            render_table[track->GetType()].item_on_render(track, selected_clip_index);
        //        }
        //    }
        //    ImGui::EndChild();
        //}
        //else
        //{
        //    ImGui::Text("Animation Sequence: ");
        //    ImGui::SameLine();
        //    ImGui::Button(name.c_str());
        //    Ref<Animation::Sequence> seq = ImGuiDragDropResource<Animation::Sequence>(".trcsq");
        //    if (seq)
        //    {
        //        SetAnimationSequence(seq);
        //        sequence = seq;
        //    }
        //}
        //ImGui::NextColumn();
        //ImGui::BeginChild("##Sequencer");
        //ImGui::SetWindowFontScale(0.85f);
        //static ImVec2 mouse_pos;
        //bool is_left_key_pressed = InputSystem::get_instance()->GetButtonState(Buttons::BUTTON_LEFT) == KeyState::KEY_PRESS;

        //float headerHeight = 40.0f;
        //float trackHeight = 25.0f;
        //ImVec2 window_size = ImGui::GetWindowSize();
        //float timelineHeight = window_size.y * 2.0f;
        //ImVec2 window_pos = ImGui::GetWindowPos();
        //float window_width = ImGui::GetWindowWidth();
        //float draw_offset = 150.0f;

        //// Draw timeline header background
        //ImDrawList* drawList = ImGui::GetWindowDrawList();
        //ImVec2 cursorPos = ImGui::GetCursorScreenPos();
        //cursorPos.x += draw_offset;
        //ImVec2 headerEnd = ImVec2(cursorPos.x + (timelineEnd - timelineStart) * pixelsPerSecond, cursorPos.y + headerHeight);
        //drawList->AddRectFilled(cursorPos, headerEnd, IM_COL32(50, 50, 50, 255));
        //if (ImGui::IsMouseHoveringRect(cursorPos, headerEnd) && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && rect_id == 0)
        //{
        //    float time = mouse_pos.x - cursorPos.x;
        //    time /= pixelsPerSecond;
        //    time += timelineStart;
        //    currentTime = time;
        //}

        //// Draw dynamic grid lines with increasing granularity as we zoom in
        //float step = 4.0f;
        //if (pixelsPerSecond <= 30.0f)
        //{
        //    step = 2.0f;
        //}
        //else if (pixelsPerSecond > 30.0f && pixelsPerSecond < 60.0f)
        //{
        //    step = 1.0f;
        //}
        //else if (pixelsPerSecond > 60.0f && pixelsPerSecond < 120.0f)
        //{
        //    step = 0.5f;
        //}
        //else if (pixelsPerSecond > 120.0f && pixelsPerSecond < 750.0f)
        //{
        //    step = 0.25f;
        //}
        //else if (pixelsPerSecond > 750.0f && pixelsPerSecond < 2500.0f)
        //{
        //    step = 0.05f;
        //}
        //else if (pixelsPerSecond > 2500.0f && pixelsPerSecond < 5000.0f)
        //{
        //    step = 0.01f;
        //}
        //else if (pixelsPerSecond >= 5000.f)
        //{
        //    step = 0.005f;
        //}

        //for (float t = 0.0f; t <= timelineEnd; t += (step * 2.0f)) {
        //    if (t < timelineStart)
        //    {
        //        continue;
        //    }
        //    float x = cursorPos.x + (t - timelineStart) * pixelsPerSecond;
        //    if (x > (window_width + window_pos.x))
        //    {
        //        continue;
        //    }
        //    int alpha = 200;
        //    ImU32 lineColor = IM_COL32(200, 200, 200, alpha);
        //    drawList->AddLine(ImVec2(x, cursorPos.y), ImVec2(x, cursorPos.y + timelineHeight + headerHeight), lineColor);
        //    

        //}

        //for (float t = 0.0f; t <= timelineEnd; t += (step)) {
        //    if (t < timelineStart)
        //    {
        //        continue;
        //    }
        //    float x = cursorPos.x + (t - timelineStart) * pixelsPerSecond;
        //    if (x > (window_width + window_pos.x))
        //    {
        //        continue;
        //    }
        //    int alpha = 100;
        //    ImU32 lineColor = IM_COL32(150, 150, 150, alpha);
        //    drawList->AddLine(ImVec2(x, cursorPos.y), ImVec2(x, cursorPos.y + timelineHeight + headerHeight), lineColor);
        //    
        //    char buf[128] = {};
        //    if (step < 0.01f)
        //    {
        //        sprintf_s<128>(buf, "%.3f", t);
        //    }
        //    else
        //    {
        //        sprintf_s<128>(buf, "%.2f", t);
        //    }
        //    drawList->AddText(ImVec2(x - 7, cursorPos.y + 5), IM_COL32(255, 255, 255, 255), buf);


        //}


        //// Draw the playhead and handle dragging
        //float playheadX = cursorPos.x + (currentTime - timelineStart) * pixelsPerSecond;
        //ImVec2 playheadPos(playheadX, cursorPos.y + headerHeight);
        //ImVec2 playheadEnd(playheadX, cursorPos.y + timelineHeight + headerHeight);

        //ImVec2 play_head_start = ImVec2(playheadX - 5.0f, cursorPos.y + (headerHeight / 2.0f));
        //ImVec2 play_head_end = ImVec2(playheadX + 5.0f, cursorPos.y + headerHeight);
        //ImU32 head_color = IM_COL32(255, 0, 0, 100);
        //int32_t play_head_id = 25;
        //bool is_hovering_play_head = ImGui::IsMouseHoveringRect(play_head_start, play_head_end);
        //if (is_hovering_play_head)
        //{
        //    head_color = IM_COL32(255, 0, 0, 255);
        //}
        //if (playheadX > (cursorPos.x))
        //{
        //    drawList->AddRectFilled(play_head_start, play_head_end, head_color, 2.0f);
        //}

        //if (is_hovering_play_head && is_left_key_pressed) 
        //{
        //    rect_id = play_head_id;
        //}
        //if (rect_id == play_head_id)
        //{
        //    currentTime = timelineStart + (mouse_pos.x - cursorPos.x) / pixelsPerSecond;
        //    currentTime = std::clamp(currentTime, timelineStart, timelineEnd);
        //}

        //if (sequence)
        //{
        //    mouse_pos = ImGui::GetMousePos();
        //    float mouse_time = mouse_pos.x - cursorPos.x;
        //    mouse_time /= pixelsPerSecond;
        //    mouse_time += timelineStart;
        //    std::vector<Animation::SequenceTrack*>& tracks = sequence->GetTracks();             
        //    

        //    // Draw each track and its clips
        //    float min_pixels_per_second = get_pixels_per_second(window_width - draw_offset, sequence->GetDuration());
        //    float trackY = cursorPos.y + headerHeight + 20.0f;
        //    int32_t track_id = 1000;
        //    uint32_t track_index = 0;
        //    for (auto& track : tracks) {
        //        track_ui_info& ui_info = render_table[track->GetType()];

        //        ImGui::SetCursorScreenPos(ImVec2(cursorPos.x - 150.0f, trackY));
        //        std::string name = "None (Entity)";
        //        StringID string_id = track->GetStringID();
        //        Entity entity;
        //        if (string_id.value != 0)
        //        {
        //            name = STRING_FROM_ID(string_id);
        //            entity = editor->GetCurrentScene()->GetEntityByName(string_id);
        //        }
        //        ImGui::Button(name.c_str(), ImVec2(draw_offset, trackHeight));
        //        ImGui::PushID((int)track_id);
        //        if (ImGui::BeginPopupContextItem("Track Popup"))
        //        {
        //            if (ImGui::MenuItem("Delete"))
        //            {
        //                sequence->RemoveTrack(track_index);
        //            }
        //            ImGui::EndPopup();
        //        }
        //        ImGui::PopID();
        //        if (ImGui::BeginDragDropTarget())
        //        {
        //            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Entity"))
        //            {
        //                UUID uuid = *(UUID*)payload->Data;
        //                Entity entity = editor->GetCurrentScene()->GetEntity(uuid);
        //                
        //                if (string_id.value == 0)
        //                {
        //                    track->SetStringID(entity.GetComponent<TagComponent>().GetStringID());
        //                    m_instance.DestroyInstance();
        //                    m_instance.CreateInstance(sequence, editor->GetCurrentScene().get());
        //                }
        //            }
        //            ImGui::EndDragDropTarget();
        //        }
        //        ImGui::SetCursorScreenPos(ImVec2(cursorPos.x, trackY - 15.0f));

        //        int32_t index = 0;
        //        bool is_hovering_any_clip = false;
        //        for (auto& clip : track->GetTrackChannels())
        //        {
        //            track_id += 100;
        //            float clipX = cursorPos.x + (clip->GetStartTime() - timelineStart) * pixelsPerSecond;
        //            clipX = std::clamp(clipX, cursorPos.x, clipX);
        //            float clipWidth = clip->GetDuration() * pixelsPerSecond;
        //            float end_time = clip->GetStartTime() + clip->GetDuration();
        //            float edgeWidth = 5.0f;
        //            bool can_modify_start_time = true;
        //            if (clip->GetStartTime() < timelineStart)
        //            {
        //                clipX = cursorPos.x;
        //                float difference = (timelineStart - clip->GetStartTime());
        //                clipWidth = clip->GetDuration() - difference;
        //                clipWidth *= pixelsPerSecond;
        //                can_modify_start_time = (difference * pixelsPerSecond) > edgeWidth ? false : true;
        //            }
        //            ImVec2 clipPos(clipX, trackY);
        //            ImVec2 clipSize(clipX + clipWidth, trackY + trackHeight);
        //            int32_t left_id = track_id + 10;
        //            int32_t right_id = track_id + 20;
        //            int32_t clip_id = track_id + 30;

        //            if (clipWidth <= 0.0f)
        //            {
        //                index++;
        //                continue;
        //            }

        //            if (clipX > (window_width + window_pos.x))
        //            {
        //                index++;
        //                continue;
        //            }

        //            ImVec2 move_offset = ImVec2(0.0f, 0.0f);
        //            if (rect_id == clip_id)
        //            {
        //                move_offset = ImVec2(3.5f, 3.5f);
        //            }

        //            drawList->AddRectFilled(clipPos + shadowOffset + move_offset, clipSize + shadowOffset + move_offset, shadowColor);
        //            drawList->AddRectFilled(clipPos, clipSize, ui_info.color);

        //            auto prev_it = track->GetTrackChannels().end();
        //            auto next_it = track->GetTrackChannels().end();
        //            auto curr_it = track->GetTrackChannels().begin() + index;

        //            if (index - 1 >= 0)
        //            {
        //                prev_it = track->GetTrackChannels().begin() + (index - 1);
        //            }
        //            if (index + 1 < track->GetTrackChannels().size())
        //            {
        //                next_it = track->GetTrackChannels().begin() + (index + 1);
        //            }

        //            // Draggable regions for resizing

        //            ImVec2 leftEdge(clipPos.x, clipPos.y);
        //            ImVec2 left_edge_size(leftEdge.x + edgeWidth, leftEdge.y + clipSize.y - clipPos.y);
        //            ImVec2 rightEdge(clipSize.x - edgeWidth, clipPos.y);
        //            ImVec2 right_edge_size(clipSize.x, clipSize.y);

        //            bool is_hovering_left_edge = ImGui::IsMouseHoveringRect(leftEdge, left_edge_size);
        //            bool is_hovering_right_edge = ImGui::IsMouseHoveringRect(rightEdge, right_edge_size);
        //            bool is_hovering_clip = ImGui::IsMouseHoveringRect(clipPos, clipSize);
        //            is_hovering_any_clip = is_hovering_any_clip || is_hovering_clip;

        //            

        //            bool isResizingLeft = is_hovering_left_edge && ImGui::IsMouseDragging(ImGuiMouseButton_Left);
        //            bool isResizingRight = is_hovering_right_edge && ImGui::IsMouseDragging(ImGuiMouseButton_Left);

        //            if (is_hovering_left_edge && can_modify_start_time)
        //            {
        //                drawList->AddRectFilled(leftEdge, left_edge_size, IM_COL32(200, 200, 200, 200));
        //                if (is_left_key_pressed)
        //                {
        //                    rect_id = left_id;
        //                }
        //            }
        //            else if (is_hovering_right_edge)
        //            {
        //                drawList->AddRectFilled(rightEdge, right_edge_size, IM_COL32(200, 200, 200, 200));
        //                if (is_left_key_pressed)
        //                {
        //                    rect_id = right_id;
        //                }
        //            }

        //            if (rect_id == left_id && can_modify_start_time)
        //            {
        //                float deltaX = ImGui::GetIO().MouseDelta.x / pixelsPerSecond;
        //                clip->SetStartTime(std::clamp(clip->GetStartTime() + deltaX, timelineStart, clip->GetStartTime() + clip->GetDuration() - minClipDuration));
        //                clip->SetDuration(std::max(end_time - clip->GetStartTime(), minClipDuration));
        //                modified_tracks.emplace(track_index);
        //            }
        //            if (rect_id == right_id)
        //            {
        //                float deltaX = ImGui::GetIO().MouseDelta.x / pixelsPerSecond;
        //                clip->SetDuration(std::max(clip->GetDuration() + deltaX, minClipDuration));
        //            }


        //            // Dragging the entire clip
        //            ImVec2 move_min = ImVec2(clipPos.x + edgeWidth, clipPos.y);
        //            ImVec2 move_max = ImVec2(clipSize.x - edgeWidth, clipSize.y);
        //            if (ImGui::IsMouseHoveringRect(move_min, move_max) && is_left_key_pressed)
        //            {
        //                rect_id = clip_id;
        //            }
        //            if (ImGui::IsMouseHoveringRect(move_min, move_max) && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
        //            {
        //                selected_clip_index = index;
        //                selected_track_index = track_index;
        //            }
        //            if (rect_id == clip_id)
        //            {
        //                float deltaX = ImGui::GetIO().MouseDelta.x / pixelsPerSecond;
        //                clip->SetStartTime(std::clamp(clip->GetStartTime() + deltaX, timelineStart, timelineEnd - clip->GetDuration()));

        //                modified_tracks.emplace(track_index);
        //            }

        //            if (index - 1 >= 0 && ui_info.can_blend && clipX > cursorPos.x ) {
        //                auto& prevClip = track->GetTrackChannels()[index - 1];
        //                if (prevClip->GetStartTime() + prevClip->GetDuration() > clip->GetStartTime()) {
        //                    float blendStartX = clipX;
        //                    float blendEndX = cursorPos.x + (prevClip->GetStartTime() + prevClip->GetDuration() - timelineStart) * pixelsPerSecond;


        //                    ImVec2 p1(blendStartX, trackY);
        //                    ImVec2 p2(blendEndX, trackY + trackHeight);
        //                    drawList->AddRect(p1, p2, blendColor);
        //                }
        //            }

        //            ImGui::PushID((int)clip_id);
        //            if (is_hovering_clip && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
        //            {
        //                ImGui::OpenPopup("Clip Popup");
        //                mouse_pos = ImGui::GetMousePos();

        //            }
        //            if (ImGui::BeginPopup("Clip Popup"))
        //            {
        //                if (ImGui::MenuItem("Delete"))
        //                {
        //                    track->RemoveChannel(index);
        //                    modified_tracks.emplace(track_index);
        //                    sort_tracks(tracks);
        //                }
        //                switch (track->GetType())
        //                {
        //                case Animation::SequenceTrackType::ANIMATION_TRACK:
        //                case Animation::SequenceTrackType::SKELETAL_ANIMATION_TRACK:
        //                {
        //                    Animation::AnimationChannel* chan = (Animation::AnimationChannel*)clip;
        //                    if (ImGui::MenuItem("Edit Animation Clip") && chan->GetAnimationClip() && entity)
        //                    {
        //                        editor->GetAnimationPanel()->SetAnimationClip(chan->GetAnimationClip(), entity.GetID());
        //                    }
        //                    
        //                    break;
        //                }
        //                }
        //                ImGui::EndPopup();
        //            }
        //            ImGui::PopID();

        //            index++;
        //        }
        //        // Handle right-click for new track popup
        //        ImVec2 track_min(cursorPos.x, trackY);
        //        ImVec2 track_max(cursorPos.x + window_size.x - draw_offset, trackY + trackHeight);


        //        ImGui::PushID((int)track_id);
        //        if (ImGui::IsMouseHoveringRect(track_min, track_max) && ImGui::IsMouseClicked(ImGuiMouseButton_Right) && !is_hovering_any_clip)
        //        {
        //            ImGui::OpenPopup("Insert Channel Popup");
        //            mouse_pos = ImGui::GetMousePos();

        //        }
        //        if (ImGui::BeginPopup("Insert Channel Popup"))
        //        {
        //            mouse_pos = ImGui::GetMousePos();
        //            float time = mouse_pos.x - cursorPos.x;
        //            time /= pixelsPerSecond;
        //            time += timelineStart;
        //            if (ImGui::MenuItem("Add Channel"))
        //            {
        //                switch (track->GetType())
        //                {
        //                case Animation::SequenceTrackType::ANIMATION_TRACK:
        //                {
        //                    Animation::SequenceTrackChannel* chan = new Animation::AnimationChannel;
        //                    chan->SetStartTime(time);
        //                    chan->SetDuration(1.0f);
        //                    track->GetTrackChannels().push_back(chan);//TODO: Use custom allocator
        //                    break;
        //                }
        //                case Animation::SequenceTrackType::SKELETAL_ANIMATION_TRACK:
        //                {
        //                    Animation::SequenceTrackChannel* chan = new Animation::SkeletalAnimationChannel;
        //                    chan->SetStartTime(time);
        //                    chan->SetDuration(1.0f);
        //                    track->GetTrackChannels().push_back(chan);//TODO: Use custom allocator
        //                    break;
        //                }
        //                case Animation::SequenceTrackType::ACTIVATION_TRACK:
        //                {
        //                    Animation::SequenceTrackChannel* chan = new Animation::ActivationChannel;
        //                    chan->SetStartTime(time);
        //                    chan->SetDuration(1.0f);
        //                    track->GetTrackChannels().push_back(chan);//TODO: Use custom allocator
        //                    break;
        //                }
        //                }

        //                m_instance.DestroyInstance();
        //                m_instance.CreateInstance(sequence, editor->GetCurrentScene().get());
        //                modified_tracks.emplace(track_index);
        //                sort_tracks(tracks);
        //            }
        //            
        //            ImGui::EndPopup();
        //        }
        //        ImGui::PopID();

        //        //Drag and Drop
        //        ImRect d_r = {};
        //        d_r.Min = track_min;
        //        d_r.Max = track_max;
        //        TraceEditor* editor = TraceEditor::get_instance();
        //        if (!is_hovering_any_clip)
        //        {
        //            mouse_pos = ImGui::GetMousePos();
        //            float time = mouse_pos.x - cursorPos.x;
        //            time /= pixelsPerSecond;
        //            time += timelineStart;
        //            
        //            auto result = ui_info.item_on_drag_drop(track, d_r, (ImGuiID)track_id);
        //            if (result)
        //            {
        //                result->SetStartTime(time);
        //                m_instance.DestroyInstance();
        //                m_instance.CreateInstance(sequence, editor->GetCurrentScene().get());
        //                modified_tracks.emplace(track_index);
        //            }
        //        }

        //        trackY += trackHeight + 3.0f;
        //        track_id += 1000;
        //        track_index++;
        //    }



        //    if (ImGui::IsWindowHovered() && ImGui::GetIO().MouseWheelH != 0) 
        //    {
        //        float zoomFactor = ImGui::GetIO().MouseWheelH > 0 ? 1.1f : 0.9f;
        //        pixelsPerSecond = std::clamp(pixelsPerSecond * zoomFactor, min_pixels_per_second, 7000.0f);
        //        float deltaX = ImGui::GetIO().MouseDelta.x / pixelsPerSecond;
        //        float diff = mouse_time - timelineStart;
        //        float scale = ImGui::GetIO().MouseWheelH > 0 ? 0.1f : -0.1f;
        //        timelineStart += diff * scale;
        //        timelineStart = std::max(0.0f, timelineStart);

        //    }

        //    if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
        //        float deltaX = ImGui::GetIO().MouseDelta.x / pixelsPerSecond;
        //        timelineStart = std::max(0.0f, timelineStart - deltaX);
        //        timelineEnd = std::max(timelineStart + 1.0f, timelineEnd);
        //    }
        //}
        //mouse_pos = ImGui::GetMousePos();
        //
        //if (playheadX > (cursorPos.x))
        //{
        //    drawList->AddLine(playheadPos, playheadEnd, IM_COL32(225, 225, 255, 175), 2.0f);
        //}


        //ImGui::EndChild();

        //ImGui::Columns(1);
        //ImGui::End();
	}

    void AnimationSequencer::OnEvent(Event* p_event)
    {
        switch (p_event->GetEventType())
        {
        case EventType::TRC_BUTTON_RELEASED:
        {
            MouseReleased* release = reinterpret_cast<MouseReleased*>(p_event);

            Ref<Animation::Sequence> seq = m_instance.GetSequence();
            if (release->GetButton() == Buttons::BUTTON_LEFT)
            {
                if (seq)
                {
                    sort_tracks(seq->GetTracks());
                }
                rect_id = 0;
            }

            break;
        }
        }
    }

    void AnimationSequencer::SetAnimationSequence(Ref<Animation::Sequence> sequence)
    {
        /*TraceEditor* editor = TraceEditor::get_instance();

        m_instance.DestroyInstance();
        m_instance.CreateInstance(sequence, editor->GetCurrentScene().get());
        timelineStart = 0.0f;
        timelineEnd = sequence->GetDuration();
        selected_clip_index = -1;
        selected_track_index = -1;*/
    }

}