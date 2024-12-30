
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

	bool AnimationSequencer::Init()
	{
		trace::EventsSystem::get_instance()->AddEventListener(trace::EventType::TRC_BUTTON_RELEASED, BIND_EVENT_FN(AnimationSequencer::OnEvent));

        track_ui_info& anim_track = render_table[Animation::SequenceTrackType::ANIMATION_TRACK];
        anim_track.color = IM_COL32(150, 85, 77, 150);
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
        skeletal_track.color = IM_COL32(77, 85, 150, 150);
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
        TraceEditor* editor = TraceEditor::get_instance();
        Ref<Animation::Sequence> sequence = m_instance.GetSequence();

        ImGui::Begin("Animation Sequencer");
        ImGui::Columns(2);

        std::string name = "None (Animation Sequence)";

        if (sequence)
        {
            name = sequence->GetName();
            ImGui::Text("Animation Sequence: ");
            ImGui::SameLine();
            ImGui::Button(name.c_str());
            Ref<Animation::Sequence> seq = ImGuiDragDropResource<Animation::Sequence>(".trcsq");
            if (seq)
            {
                SetAnimationSequence(seq);
                sequence = seq;
            }

            if (ImGui::Button("Add Track"))
            {
                ImGui::OpenPopup("Add Track");
            }

            if (ImGui::BeginPopup("Add Track"))
            {
                if (ImGui::MenuItem("Animation track"))
                {
                    sequence->GetTracks().push_back(new Animation::AnimationSequenceTrack);//TODO: Use custom allocator
                }
                if (ImGui::MenuItem("Skeletal Animation track"))
                {
                    sequence->GetTracks().push_back(new Animation::SkeletalAnimationTrack);//TODO: Use custom allocator
                }
                ImGui::EndPopup();
            }
            float duration = sequence->GetDuration();
            if (ImGui::DragFloat("Duration", &duration, 0.25f))
            {
                sequence->SetDuration(duration);
                timelineEnd = duration;
            }

            // Playback controls
            if (ImGui::Button("Play")) isPlaying = true;
            ImGui::SameLine();
            if (ImGui::Button("Pause")) isPlaying = false;
            ImGui::SameLine();
            if (ImGui::Button("Stop")) { isPlaying = false; currentTime = timelineStart; }

            if (isPlaying) {
                currentTime += ImGui::GetIO().DeltaTime;
                if (currentTime > timelineEnd) isPlaying = false;
            }

            if (ImGui::Button("Save"))
            {                
                AnimationsSerializer::SerializeSequence(sequence, GetPathFromUUID(GetUUIDFromName(sequence->GetName())).string());
            }

            ImGui::BeginChild("##Selected Item");

            if (selected_clip_index >= 0 && selected_track_index >= 0 && sequence->GetTracks().size() > selected_track_index)
            {
                Animation::SequenceTrack* track = sequence->GetTracks()[selected_track_index];
                if (track->GetTrackChannels().size() > selected_clip_index)
                {
                    Animation::SequenceTrackChannel* channel = track->GetTrackChannels()[selected_clip_index];

                    float channel_start_time = channel->GetStartTime();
                    float channel_duration = channel->GetDuration();
                    if (ImGui::DragFloat("Start Time", &channel_start_time))
                    {
                        channel->SetStartTime(channel_start_time);
                    }
                    if (ImGui::DragFloat("Duration", &channel_duration))
                    {
                        channel->SetDuration(channel_duration);
                    }

                    render_table[track->GetType()].item_on_render(track, selected_clip_index);
                }
            }
            ImGui::EndChild();
        }
        else
        {
            ImGui::Text("Animation Sequence: ");
            ImGui::SameLine();
            ImGui::Button(name.c_str());
            Ref<Animation::Sequence> seq = ImGuiDragDropResource<Animation::Sequence>(".trcsq");
            if (seq)
            {
                SetAnimationSequence(seq);
                sequence = seq;
            }
        }
        ImGui::NextColumn();
        ImGui::BeginChild("##Sequencer");
        static ImVec2 mouse_pos;
        bool is_left_key_pressed = InputSystem::get_instance()->GetButtonState(Buttons::BUTTON_LEFT) == KeyState::KEY_PRESS;

        if (sequence)
        {
            std::vector<Animation::SequenceTrack*>& tracks = sequence->GetTracks();

            float timelineHeight = tracks.size() * 60.0f;
            float headerHeight = 40.0f;
            float trackHeight = 40.0f;
            ImVec2 window_size = ImGui::GetWindowSize();
            ImVec2 window_pos = ImGui::GetWindowPos();
            float window_width = ImGui::GetWindowWidth();
            float draw_offset = 150.0f;

            // Draw timeline header background
            ImDrawList* drawList = ImGui::GetWindowDrawList();
            ImVec2 cursorPos = ImGui::GetCursorScreenPos();
            cursorPos.x += draw_offset;
            ImVec2 headerEnd = ImVec2(cursorPos.x + (timelineEnd - timelineStart) * pixelsPerSecond, cursorPos.y + headerHeight);
            drawList->AddRectFilled(cursorPos, headerEnd, IM_COL32(50, 50, 50, 255));

            // Draw dynamic grid lines with increasing granularity as we zoom in
            int32_t line_index = 0;
            for (float timeStep = 1.0f; timeStep >= minTimeStep; timeStep /= 2.0f) {
                if (pixelsPerSecond * timeStep < 10.0f) break;

                for (float t = timelineStart; t <= timelineEnd; t += timeStep) {
                    float x = cursorPos.x + (t - timelineStart) * pixelsPerSecond;
                    int alpha = (timeStep < 1.0f) ? 100 : 200;
                    ImU32 lineColor = (timeStep == 1.0f) ? IM_COL32(200, 200, 200, alpha) : IM_COL32(150, 150, 150, alpha);
                    drawList->AddLine(ImVec2(x, cursorPos.y), ImVec2(x, cursorPos.y + timelineHeight + headerHeight), lineColor);
                    char buf[128] = {};
                    sprintf_s<128>(buf, "%.2f", t);
                    if (line_index % 2 == 0) {
                        drawList->AddText(ImVec2(x - 2, cursorPos.y + 5), IM_COL32(255, 255, 255, 255), buf);
                    }

                    line_index++;
                }
            }

            // Draw the playhead and handle dragging
            float playheadX = cursorPos.x + (currentTime - timelineStart) * pixelsPerSecond;
            ImVec2 playheadPos(playheadX, cursorPos.y);
            ImVec2 playheadEnd(playheadX, cursorPos.y + timelineHeight + headerHeight);

            drawList->AddLine(playheadPos, playheadEnd, IM_COL32(255, 0, 0, 255), 2.0f);

            ImVec2 mousePos = ImGui::GetIO().MousePos;
            if (ImGui::IsMouseHoveringRect(ImVec2(playheadPos.x - 2.0f, playheadPos.y - 0.0f), ImVec2(playheadEnd.x + 2.0f, playheadEnd.y + 0.0f)) && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                isDraggingPlayhead = true;
            }

            if (isDraggingPlayhead) {
                if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
                    currentTime = timelineStart + (mousePos.x - cursorPos.x) / pixelsPerSecond;
                    currentTime = std::clamp(currentTime, timelineStart, timelineEnd);
                }
                else {
                    isDraggingPlayhead = false;
                }
            }

            // Draw each track and its clips
            float trackY = cursorPos.y + headerHeight + 20.0f;
            int32_t track_id = 1000;
            uint32_t track_index = 0;
            for (auto& track : tracks) {
                track_ui_info& ui_info = render_table[track->GetType()];

                ImGui::SetCursorScreenPos(ImVec2(cursorPos.x - 150.0f, trackY));
                std::string name = "None (Entity)";
                StringID string_id = track->GetStringID();
                if (string_id.value != 0)
                {
                    name = STRING_FROM_ID(string_id);
                }
                ImGui::Button(name.c_str(), ImVec2(draw_offset, headerHeight));
                ImGui::PushID((int)track_id);
                if (ImGui::BeginPopupContextItem("Track Popup"))
                {
                    if (ImGui::MenuItem("Delete"))
                    {
                        sequence->RemoveTrack(track_index);
                    }
                    ImGui::EndPopup();
                }
                ImGui::PopID();
                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Entity"))
                    {
                        UUID uuid = *(UUID*)payload->Data;
                        Entity entity = editor->GetCurrentScene()->GetEntity(uuid);
                        
                        if (string_id.value == 0)
                        {
                            track->SetStringID(entity.GetComponent<TagComponent>().GetStringID());
                            m_instance.DestroyInstance();
                            m_instance.CreateInstance(sequence, editor->GetCurrentScene().get());
                        }
                    }
                    ImGui::EndDragDropTarget();
                }
                ImGui::SetCursorScreenPos(ImVec2(cursorPos.x, trackY - 15.0f));

                int32_t index = 0;
                bool is_hovering_any_clip = false;
                for (auto& clip : track->GetTrackChannels())
                {
                    track_id += 100;
                    float clipX = cursorPos.x + (clip->GetStartTime() - timelineStart) * pixelsPerSecond;
                    clipX = std::clamp(clipX, cursorPos.x, clipX);
                    float clipWidth = clip->GetDuration() * pixelsPerSecond;
                    float end_time = clip->GetStartTime() + clip->GetDuration();
                    float edgeWidth = 5.0f;
                    bool can_modify_start_time = true;
                    if (clip->GetStartTime() < timelineStart)
                    {
                        clipX = cursorPos.x;
                        float difference = (timelineStart - clip->GetStartTime());
                        clipWidth = clip->GetDuration() - difference;
                        clipWidth *= pixelsPerSecond;
                        can_modify_start_time = (difference * pixelsPerSecond) > edgeWidth ? false : true;
                    }
                    ImVec2 clipPos(clipX, trackY);
                    ImVec2 clipSize(clipX + clipWidth, trackY + trackHeight);
                    int32_t left_id = track_id + 10;
                    int32_t right_id = track_id + 20;
                    int32_t clip_id = track_id + 30;

                    if (clipWidth <= 0.0f)
                    {
                        continue;
                    }

                    if (clipX > (window_width + window_pos.x))
                    {
                        continue;
                    }

                    ImVec2 move_offset = ImVec2(0.0f, 0.0f);
                    if (rect_id == clip_id)
                    {
                        move_offset = ImVec2(3.5f, 3.5f);
                    }

                    drawList->AddRectFilled(clipPos + shadowOffset + move_offset, clipSize + shadowOffset + move_offset, shadowColor);
                    drawList->AddRectFilled(clipPos, clipSize, ui_info.color);
                    std::string clip_name;
                    ui_info.item_get_name(track, index, clip_name);
                    if (!clip_name.empty())
                    {
                        drawList->AddText(ImVec2(clipPos.x + 5.0f, clipPos.y + 5.0f), IM_COL32(255, 255, 255, 255), clip_name.c_str());
                    }

                    auto prev_it = track->GetTrackChannels().end();
                    auto next_it = track->GetTrackChannels().end();
                    auto curr_it = track->GetTrackChannels().begin() + index;

                    if (index - 1 >= 0)
                    {
                        prev_it = track->GetTrackChannels().begin() + (index - 1);
                    }
                    if (index + 1 < track->GetTrackChannels().size())
                    {
                        next_it = track->GetTrackChannels().begin() + (index + 1);
                    }

                    // Draggable regions for resizing

                    ImVec2 leftEdge(clipPos.x, clipPos.y);
                    ImVec2 left_edge_size(leftEdge.x + edgeWidth, leftEdge.y + clipSize.y - clipPos.y);
                    ImVec2 rightEdge(clipSize.x - edgeWidth, clipPos.y);
                    ImVec2 right_edge_size(clipSize.x, clipSize.y);

                    bool is_hovering_left_edge = ImGui::IsMouseHoveringRect(leftEdge, left_edge_size);
                    bool is_hovering_right_edge = ImGui::IsMouseHoveringRect(rightEdge, right_edge_size);
                    bool is_hovering_clip = ImGui::IsMouseHoveringRect(clipPos, clipSize);
                    is_hovering_any_clip = is_hovering_any_clip || is_hovering_clip;

                    

                    bool isResizingLeft = is_hovering_left_edge && ImGui::IsMouseDragging(ImGuiMouseButton_Left);
                    bool isResizingRight = is_hovering_right_edge && ImGui::IsMouseDragging(ImGuiMouseButton_Left);

                    if (is_hovering_left_edge && can_modify_start_time)
                    {
                        drawList->AddRectFilled(leftEdge, left_edge_size, IM_COL32(200, 200, 200, 200));
                        if (is_left_key_pressed)
                        {
                            rect_id = left_id;
                        }
                    }
                    else if (is_hovering_right_edge)
                    {
                        drawList->AddRectFilled(rightEdge, right_edge_size, IM_COL32(200, 200, 200, 200));
                        if (is_left_key_pressed)
                        {
                            rect_id = right_id;
                        }
                    }

                    if (rect_id == left_id && can_modify_start_time)
                    {
                        float deltaX = ImGui::GetIO().MouseDelta.x / pixelsPerSecond;
                        clip->SetStartTime(std::clamp(clip->GetStartTime() + deltaX, timelineStart, clip->GetStartTime() + clip->GetDuration() - minClipDuration));
                        clip->SetDuration(std::max(end_time - clip->GetStartTime(), minClipDuration));
                        modified_tracks.emplace(track_index);
                    }
                    if (rect_id == right_id)
                    {
                        float deltaX = ImGui::GetIO().MouseDelta.x / pixelsPerSecond;
                        clip->SetDuration(std::max(clip->GetDuration() + deltaX, minClipDuration));
                    }


                    // Dragging the entire clip
                    ImVec2 move_min = ImVec2(clipPos.x + edgeWidth, clipPos.y);
                    ImVec2 move_max = ImVec2(clipSize.x - edgeWidth, clipSize.y);
                    if (ImGui::IsMouseHoveringRect(move_min, move_max) && is_left_key_pressed)
                    {
                        rect_id = clip_id;
                    }
                    if (ImGui::IsMouseHoveringRect(move_min, move_max) && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                    {
                        selected_clip_index = index;
                        selected_track_index = track_index;
                    }
                    if (rect_id == clip_id)
                    {
                        float deltaX = ImGui::GetIO().MouseDelta.x / pixelsPerSecond;
                        clip->SetStartTime(std::clamp(clip->GetStartTime() + deltaX, timelineStart, timelineEnd - clip->GetDuration()));

                        modified_tracks.emplace(track_index);
                    }

                    if (index - 1 >= 0 && ui_info.can_blend) {
                        auto& prevClip = track->GetTrackChannels()[index - 1];
                        if (prevClip->GetStartTime() + prevClip->GetDuration() > clip->GetStartTime()) {
                            float blendStartX = clipX;
                            float blendEndX = cursorPos.x + (prevClip->GetStartTime() + prevClip->GetDuration() - timelineStart) * pixelsPerSecond;


                            ImVec2 p1(blendStartX, trackY);
                            ImVec2 p2(blendEndX, trackY + trackHeight);
                            drawList->AddRect(p1, p2, blendColor);
                        }
                    }

                    ImGui::PushID((int)clip_id);
                    if (is_hovering_clip && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
                    {
                        ImGui::OpenPopup("Clip Popup");
                        mouse_pos = ImGui::GetMousePos();

                    }
                    if (ImGui::BeginPopup("Clip Popup"))
                    {
                        if (ImGui::MenuItem("Delete"))
                        {
                            track->RemoveChannel(index);
                            modified_tracks.emplace(track_index);
                            sort_tracks(tracks);
                        }
                        ImGui::EndPopup();
                    }
                    ImGui::PopID();

                    index++;
                }
                // Handle right-click for new track popup
                ImVec2 track_min(cursorPos.x, trackY);
                ImVec2 track_max(cursorPos.x + window_size.x - draw_offset, trackY + trackHeight);


                ImGui::PushID((int)track_id);
                if (ImGui::IsMouseHoveringRect(track_min, track_max) && ImGui::IsMouseClicked(ImGuiMouseButton_Right) && !is_hovering_any_clip)
                {
                    ImGui::OpenPopup("Insert Channel Popup");
                    mouse_pos = ImGui::GetMousePos();

                }
                if (ImGui::BeginPopup("Insert Channel Popup"))
                {
                    mouse_pos = ImGui::GetMousePos();
                    float time = mouse_pos.x - cursorPos.x;
                    time /= pixelsPerSecond;
                    time += timelineStart;
                    if (ImGui::MenuItem("Add Channel"))
                    {
                        switch (track->GetType())
                        {
                        case Animation::SequenceTrackType::ANIMATION_TRACK:
                        {
                            Animation::SequenceTrackChannel* chan = new Animation::AnimationChannel;
                            chan->SetStartTime(time);
                            chan->SetDuration(1.0f);
                            track->GetTrackChannels().push_back(chan);//TODO: Use custom allocator
                            break;
                        }
                        case Animation::SequenceTrackType::SKELETAL_ANIMATION_TRACK:
                        {
                            Animation::SequenceTrackChannel* chan = new Animation::SkeletalAnimationChannel;
                            chan->SetStartTime(time);
                            chan->SetDuration(1.0f);
                            track->GetTrackChannels().push_back(chan);//TODO: Use custom allocator
                            break;
                        }
                        }

                        m_instance.DestroyInstance();
                        m_instance.CreateInstance(sequence, editor->GetCurrentScene().get());
                        modified_tracks.emplace(track_index);
                        sort_tracks(tracks);
                    }
                    ImGui::EndPopup();
                }
                ImGui::PopID();

                //Drag and Drop
                ImRect d_r = {};
                d_r.Min = track_min;
                d_r.Max = track_max;
                TraceEditor* editor = TraceEditor::get_instance();
                if (!is_hovering_any_clip)
                {
                    mouse_pos = ImGui::GetMousePos();
                    float time = mouse_pos.x - cursorPos.x;
                    time /= pixelsPerSecond;
                    time += timelineStart;
                    
                    auto result = ui_info.item_on_drag_drop(track, d_r, (ImGuiID)track_id);
                    if (result)
                    {
                        result->SetStartTime(time);
                        m_instance.DestroyInstance();
                        m_instance.CreateInstance(sequence, editor->GetCurrentScene().get());
                        modified_tracks.emplace(track_index);
                    }
                }

                trackY += trackHeight + 10.0f;
                track_id += 1000;
                track_index++;
            }



            if (ImGui::IsWindowHovered() && ImGui::GetIO().MouseWheelH != 0) {
                float zoomFactor = ImGui::GetIO().MouseWheelH > 0 ? 1.1f : 0.9f;
                pixelsPerSecond = std::clamp(pixelsPerSecond * zoomFactor, 50.0f, 1000.0f);
                ImGui::SetItemUsingMouseWheel();
            }

            if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
                float deltaX = ImGui::GetIO().MouseDelta.x / pixelsPerSecond;
                timelineStart = std::max(0.0f, timelineStart - deltaX);
                timelineEnd = std::max(timelineStart + 1.0f, timelineEnd);
            }
        }
        mouse_pos = ImGui::GetMousePos();
        ImGui::EndChild();

        ImGui::Columns(1);
        ImGui::End();
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
        TraceEditor* editor = TraceEditor::get_instance();

        m_instance.DestroyInstance();
        m_instance.CreateInstance(sequence, editor->GetCurrentScene().get());
        timelineStart = 0.0f;
        timelineEnd = sequence->GetDuration();
        selected_clip_index = -1;
        selected_track_index = -1;
    }

}