
#include "AnimationSequencer.h"
#include "core/events/EventsSystem.h"
#include "core/io/Logging.h"
#include "scene/UUID.h"
#include "scene/Entity.h"
#include "scene/Scene.h"
#include "../TraceEditor.h"
#include "core/input/Input.h"

#include "imgui.h"
#include "imgui_stdlib.h"
#include "imgui_internal.h"

#include <algorithm>
#include <set>

namespace trace {

    static int32_t rect_id = 0;

    struct Keyframe {
        float time;
    };

    struct Clip {
        float startTime;
        float duration;
        std::string name;
        std::vector<Keyframe> keyframes;
    };

    struct Track {
        std::string name;
        std::vector<Clip> clips;
        float height = 40.0f;  // Track height
    };

    std::vector<Track> tracks = {
        { "Animation Track", { {0.0f, 2.0f, "Walk", {{0.5f}, {1.5f}}}, {3.0f, 2.0f, "Run", {{3.5f}, {4.5f}}} } },
        { "Audio Track", { {1.0f, 2.5f, "Audio Clip", {{1.5f}}} } },
        { "sdjkbh Track", { {0.0f, 2.0f, "Walk", {{0.5f}, {1.5f}}}, {3.0f, 2.0f, "Run", {{3.5f}, {4.5f}}} } },
        { "Event Track", { {5.0f, 1.0f, "Event Trigger", {{5.2f}}} } },
        { "goal Track", { {0.0f, 2.0f, "Walk", {{0.5f}, {1.5f}}}, {3.0f, 2.0f, "Run", {{3.5f}, {4.5f}}} } },
        { "sldivj Track", { {0.0f, 2.0f, "Walk", {{0.5f}, {1.5f}}}, {3.0f, 2.0f, "Run", {{3.5f}, {4.5f}}} } },
        { "Event Track", { {5.0f, 1.0f, "Event Trigger", {{5.2f}}} } }
    };

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

    std::set<uint32_t> modified_tracks;

    static void sort_tracks()
    {
        for (const uint32_t& index : modified_tracks)
        {
            Track& track = tracks[index];
            std::sort(track.clips.begin(), track.clips.end(), [](Clip& a, Clip& b)
                {
                    return a.startTime < b.startTime;
                });
        }

        modified_tracks.clear();
    }

	bool AnimationSequencer::Init()
	{
		trace::EventsSystem::get_instance()->AddEventListener(trace::EventType::TRC_BUTTON_RELEASED, BIND_EVENT_FN(AnimationSequencer::OnEvent));
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
        
        ImGui::Begin("Animation Sequencer");

        static ImVec2 mouse_pos;
        bool is_left_key_pressed = InputSystem::get_instance()->GetButtonState(Buttons::BUTTON_LEFT) == KeyState::KEY_PRESS;
        

        float timelineHeight = tracks.size() * 60.0f;
        float headerHeight = 40.0f;
        ImVec2 window_size = ImGui::GetWindowSize();
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
            //drawList->AddRectFilled(ImVec2(cursorPos.x, trackY - 20.0f), ImVec2(cursorPos.x + (timelineEnd - timelineStart) * pixelsPerSecond, trackY + track.height), IM_COL32(35, 35, 35, 255));
            ImGui::SetCursorScreenPos(ImVec2(cursorPos.x - 150.0f, trackY));
            ImGui::Button(track.name.c_str(), ImVec2(150.0f, headerHeight));
            ImGui::SetCursorScreenPos(ImVec2(cursorPos.x, trackY - 15.0f));

            int32_t index = 0;
            bool is_hovering_any_clip = false;
            for (auto& clip : track.clips) {
                track_id += 100;
                float clipX = cursorPos.x + (clip.startTime - timelineStart) * pixelsPerSecond;
                clipX = std::clamp(clipX, cursorPos.x, clipX);
                float clipWidth = clip.duration * pixelsPerSecond;
                float end_time = clip.startTime + clip.duration;
                float edgeWidth = pixelsPerSecond > 25.0f ? 0.1f * pixelsPerSecond : 5.0f;
                bool can_modify_start_time = true;
                if (clip.startTime < timelineStart)
                {
                    clipX = cursorPos.x;
                    float difference = (timelineStart - clip.startTime);
                    clipWidth = clip.duration - difference;
                    clipWidth *= pixelsPerSecond;
                    can_modify_start_time = (difference * pixelsPerSecond) > edgeWidth ? false : true;
                }
                ImVec2 clipPos(clipX, trackY);
                ImVec2 clipSize(clipX + clipWidth, trackY + track.height);

                if (clipWidth <= 0.0f)
                {
                    continue;
                }

                if (clipX > (window_size.x + draw_offset))
                {
                    continue;
                }

                drawList->AddRectFilled(clipPos, clipSize, IM_COL32(100, 150, 250, 255));
                drawList->AddText(ImVec2(clipPos.x + 5.0f, clipPos.y + 5.0f), IM_COL32(255, 255, 255, 255), clip.name.c_str());
                
                auto prev_it = track.clips.end();
                auto next_it = track.clips.end();
                auto curr_it = track.clips.begin() + index;

                if (index - 1 >= 0)
                {
                    prev_it = track.clips.begin() + (index - 1);
                }
                if (index + 1 < track.clips.size())
                {
                    next_it = track.clips.begin() + (index + 1);
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
                int32_t left_id = track_id + 10;
                int32_t right_id = track_id + 20;
                int32_t clip_id = track_id + 30;

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
                    clip.startTime = std::clamp(clip.startTime + deltaX, timelineStart, clip.startTime + clip.duration - minClipDuration);
                    clip.duration = std::max(end_time - clip.startTime, minClipDuration);
                    modified_tracks.emplace(track_index);
                }
                if (rect_id == right_id)
                {
                    float deltaX = ImGui::GetIO().MouseDelta.x / pixelsPerSecond;
                    clip.duration = std::max(clip.duration + deltaX, minClipDuration);
                }


                


                // Dragging the entire clip
                ImVec2 move_min = ImVec2(clipPos.x + edgeWidth, clipPos.y);
                ImVec2 move_max = ImVec2(clipSize.x - edgeWidth, clipSize.y);
                if (ImGui::IsMouseHoveringRect(move_min, move_max) && is_left_key_pressed)
                {
                    rect_id = clip_id;
                }
                if (rect_id == clip_id)
                {
                    float deltaX = ImGui::GetIO().MouseDelta.x / pixelsPerSecond;
                    clip.startTime = std::clamp(clip.startTime + deltaX, timelineStart, timelineEnd - clip.duration);

                    modified_tracks.emplace(track_index);
                }

                if (index - 1 >= 0) {
                    auto& prevClip = track.clips[index - 1];
                    if (prevClip.startTime + prevClip.duration > clip.startTime) {
                        //float blendStartX = cursorPos.x + (prevClip.startTime - timelineStart) * pixelsPerSecond;
                        float blendStartX = clipX;
                        //float blendEndX = std::min(clipX + clipWidth, cursorPos.x + (prevClip.startTime + prevClip.duration - timelineStart) * pixelsPerSecond);
                        float blendEndX = cursorPos.x + (prevClip.startTime + prevClip.duration - timelineStart) * pixelsPerSecond;

                        
                        ImVec2 p1(blendStartX, trackY);
                        ImVec2 p2(blendEndX, trackY + track.height);
                        drawList->AddRect(p1, p2, blendColor);
                    }
                }

                

                index++;
            }
            // Handle right-click for new track popup
            ImVec2 track_min(cursorPos.x, trackY);
            ImVec2 track_max(cursorPos.x + window_size.x - draw_offset, trackY + track.height);
            

            ImGui::PushID((int)track_id);
            if (ImGui::IsMouseHoveringRect(track_min, track_max) && ImGui::IsMouseClicked(ImGuiMouseButton_Right) && !is_hovering_any_clip)
            {
                ImGui::OpenPopup("Insert Track Popup");
                mouse_pos = ImGui::GetMousePos();
                
            }
            if (ImGui::BeginPopup("Insert Track Popup"))
            {
                ImGui::Text("Insert New Track");
                ImGui::Separator();

                static char newTrackName[128] = "";
                ImGui::InputText("Track Name", newTrackName, IM_ARRAYSIZE(newTrackName));

                if (ImGui::Button("Insert")) {
                    if (strlen(newTrackName) > 0) {
                        // Insert new track at the clicked position
                        float time = mouse_pos.x - cursorPos.x;
                        time /= pixelsPerSecond;
                        time += timelineStart;
                        Clip new_clip;
                        new_clip.duration = 1.0f;
                        new_clip.name = newTrackName;
                        new_clip.startTime = time;
                        track.clips.push_back(new_clip);
                        modified_tracks.emplace(track_index);
                        sort_tracks();
                        memset(newTrackName, 0, sizeof(newTrackName)); // Clear input
                        ImGui::CloseCurrentPopup();
                    }
                }
                ImGui::EndPopup();
            }
            ImGui::PopID();

            //Drag and Drop
            ImRect d_r = {};
            d_r.Min = track_min;
            d_r.Max = track_max;
            TraceEditor* editor = TraceEditor::get_instance();
            if (ImGui::BeginDragDropTargetCustom(d_r, (int)track_id) && !is_hovering_any_clip)
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Entity"))
                {
                    UUID uuid = *(UUID*)payload->Data;
                    Entity entity = editor->GetCurrentScene()->GetEntity(uuid);
                    mouse_pos = ImGui::GetMousePos();
                    float time = mouse_pos.x - cursorPos.x;
                    time /= pixelsPerSecond;
                    time += timelineStart;
                    Clip new_clip;
                    new_clip.duration = 1.0f;
                    new_clip.name = entity.GetComponent<TagComponent>().GetTag();
                    new_clip.startTime = time;
                    track.clips.push_back(new_clip);
                    modified_tracks.emplace(track_index);
                    sort_tracks();
                }
                ImGui::EndDragDropTarget();
            }

            trackY += track.height + 10.0f;
            track_id += 1000;
            track_index++;
        }

        // Playback and zoom controls
        if (ImGui::Button("Play")) isPlaying = true;
        ImGui::SameLine();
        if (ImGui::Button("Pause")) isPlaying = false;
        ImGui::SameLine();
        if (ImGui::Button("Stop")) { isPlaying = false; currentTime = timelineStart; }

        if (isPlaying) {
            currentTime += ImGui::GetIO().DeltaTime;
            if (currentTime > timelineEnd) isPlaying = false;
        }

        if (ImGui::IsWindowHovered() && ImGui::GetIO().MouseWheel != 0) {
            float zoomFactor = ImGui::GetIO().MouseWheel > 0 ? 1.1f : 0.9f;
            pixelsPerSecond = std::clamp(pixelsPerSecond * zoomFactor, 50.0f, 1000.0f);
        }

        if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
            float deltaX = ImGui::GetIO().MouseDelta.x / pixelsPerSecond;
            timelineStart = std::max(0.0f, timelineStart - deltaX);
            timelineEnd = std::max(timelineStart + 1.0f, timelineEnd);
        }        

        mouse_pos = ImGui::GetMousePos();
        ImGui::End();
	}

    void AnimationSequencer::OnEvent(Event* p_event)
    {
        switch (p_event->GetEventType())
        {
        case EventType::TRC_BUTTON_RELEASED:
        {
            MouseReleased* release = reinterpret_cast<MouseReleased*>(p_event);

            if (release->GetButton() == Buttons::BUTTON_LEFT)
            {
                sort_tracks();
                rect_id = 0;
            }

            break;
        }
        }
    }

}