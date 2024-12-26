
#include "AnimationGraphEditor.h"
#include "animation/Animation.h"
#include "resource/AnimationsManager.h"
#include "serialize/AnimationsSerializer.h"
#include "../utils/ImGui_utils.h"
#include "../TraceEditor.h"
#include "core/events/Events.h"

#include "imgui.h"
#include "imgui_stdlib.h"
#include "imnodes/imnodes.h"
#include "portable-file-dialogs.h"


namespace trace {

    static int start_node_id = 0;
    static int start_output_id = 1;
    static int link_id = 0;
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



	bool AnimationGraphEditor::Init()
	{
		ImNodes::GetStyle().LinkLineSegmentsPerLength = 0.0f;

        trace::EventsSystem::get_instance()->AddEventListener(trace::EventType::TRC_BUTTON_RELEASED, BIND_EVENT_FN(AnimationGraphEditor::OnEvent));


		return true;
	}

	void AnimationGraphEditor::Shutdown()
	{
        TraceEditor* editor = TraceEditor::get_instance();
        if (m_currentGraph)
        {
            std::string db_path = editor->GetCurrentProject()->GetProjectCurrentDirectory() + "/InternalAssetsDB/" + m_currentGraph->GetName() + ".ini";
            ImNodes::SaveCurrentEditorStateToIniFile(db_path.c_str());
        }
	}

	void AnimationGraphEditor::Update(float deltaTime)
	{
	}

    void AnimationGraphEditor::OnEvent(Event* p_event)
    {

        switch (p_event->GetEventType())
        {
        case EventType::TRC_BUTTON_RELEASED:
        {
            MouseReleased* release = reinterpret_cast<MouseReleased*>(p_event);

            if (release->GetButton() == Buttons::BUTTON_LEFT)
            {
                rect_id = 0;
            }

            break;
        }
        }

    }

	void AnimationGraphEditor::Render(float deltaTime)
	{
        TraceEditor* editor = TraceEditor::get_instance();

		//ImGui::Begin("Animation Graph Editor");
		/*ImGui::Columns(2);

		if (m_currentGraph)
		{
            std::string graph_name = m_currentGraph->GetName();
            

            ImGui::Button(graph_name.c_str());
            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(".trcag"))
                {
                    static char buf[1024] = { 0 };
                    memcpy_s(buf, 1024, payload->Data, payload->DataSize);
                    SetAnimationGraph(buf);
                }
                ImGui::EndDragDropTarget();
            }
            

            
            if (ImGui::Button("Save"))
            {
                AnimationsSerializer::SerializeAnimationGraph(m_currentGraph, m_currentGraph->m_path.string());
            }
            
            if (m_selectedState)
            {
                ImGui::InvisibleButton("Seperator", {10.0f, GetLineHeight()});
                std::string& state_name = m_selectedState->GetName();
                ImGui::InputText("State Name", &state_name);

                bool loop = m_selectedState->GetLoop();
                if (ImGui::Checkbox("Loop", &loop)) m_selectedState->SetLoop(loop);

                Ref<AnimationClip> clip = m_selectedState->GetAnimationClip();
                std::string clip_name = clip ? clip->GetName() : "None(Animation Clip)";


                ImGui::Text("Animation Clip ");
                ImGui::SameLine();
                ImGui::Button(clip_name.c_str());
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
                        if (ac)
                        {
                            m_selectedState->SetAnimationClip(ac);
                        }
                        else
                        {
                            ac = AnimationsSerializer::DeserializeAnimationClip(p.string());
                            if (ac)
                            {
                                m_selectedState->SetAnimationClip(ac);
                            }
                        }

                    }
                    ImGui::EndDragDropTarget();
                }

            }


		}
		else
		{
            float column_width = ImGui::GetColumnWidth();
            ImGui::Button("None(Animation Graph)", { column_width, 0.0f });
            if (ImGui::BeginItemTooltip())
            {
                ImGui::Text("Drag and drop the animation graph on the button");
                ImGui::EndTooltip();
            }
            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(".trcag"))
                {
                    static char buf[1024] = { 0 };
                    memcpy_s(buf, 1024, payload->Data, payload->DataSize);
                    SetAnimationGraph(buf);

                }
                ImGui::EndDragDropTarget();
            }

            ImGui::SetCursorPosX(0.27f * column_width);
            if (ImGui::Button("Create Animation Graph"))
            {
                std::string result = pfd::save_file("New Animation Graph", "", { "Trace Animation Graph", "*.trcag" }).result();
                if (!result.empty())
                {
                    std::filesystem::path path = result;
                    if (std::filesystem::exists(path))
                    {
                        SetAnimationGraph(result);
                    }
                    else
                    {
                        if (path.extension() != ".trcag") path = std::filesystem::path(result += ".trcag");
                        Ref<AnimationGraph> graph = AnimationsManager::get_instance()->CreateGraph(path.filename().string());
                        AnimationsSerializer::SerializeAnimationGraph(graph, path.string());
                        graph.free();
                    }

                }
            }
		}

		ImGui::NextColumn();

		ImNodes::BeginNodeEditor();
        

		if (m_currentGraph)
		{
            

            ImNodes::BeginNode(start_node_id);

            ImNodes::BeginNodeTitleBar();
            ImGui::Text("Start");
            ImNodes::EndNodeTitleBar();

            ImNodes::BeginOutputAttribute(start_output_id);
            ImNodes::EndOutputAttribute();

            ImNodes::EndNode();


            std::vector<AnimationState>& states = m_currentGraph->GetStates();
            for (int i = 0; i < states.size(); i++)
            {
                

                AnimationState& _s = states[i];
                ImGuiID state_id = i + 1;
                ImNodes::BeginNode(state_id);

                ImNodes::BeginNodeTitleBar();
                ImGui::Text(_s.GetName().c_str());
                ImNodes::EndNodeTitleBar();

                ImNodes::PushColorStyle(ImNodesCol_Pin, IM_COL32(60, 198, 65, 128));
                ImNodes::PushColorStyle(ImNodesCol_PinHovered, IM_COL32(60, 198, 65, 255));
                ImNodes::BeginInputAttribute(state_id << 16);
                ImNodes::EndInputAttribute();
                ImNodes::PopColorStyle();
                ImNodes::PopColorStyle();

                ImNodes::PushColorStyle(ImNodesCol_Pin, IM_COL32(198, 60, 65, 128));
                ImNodes::PushColorStyle(ImNodesCol_PinHovered, IM_COL32(198, 60, 65, 255));
                ImNodes::BeginOutputAttribute(state_id << 4);
                ImNodes::EndOutputAttribute();
                ImNodes::PopColorStyle();
                ImNodes::PopColorStyle();

                float progress = _s.GetAnimationClip() ? _s.GetElaspedTime() / _s.GetAnimationClip()->GetDuration() : 0.0f;
                ImNodes::BeginStaticAttribute(state_id << 8);
                ImGui::ProgressBar(progress, {200.0f, 10.0f});
                ImNodes::EndStaticAttribute();

                ImNodes::EndNode();

                
            }

            for (auto& link : m_links)
            {
                ImNodes::Link(link.id, link.from, link.to);
            }


            const bool open_popup = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) &&
                ImNodes::IsEditorHovered() &&
                ImGui::IsMouseClicked(ImGuiMouseButton_Right);

            if (!ImGui::IsAnyItemHovered() && open_popup)
            {
                ImGui::OpenPopup("add node");
            }

		}

        

        if (ImGui::BeginPopup("add node"))
        {
            if (ImGui::MenuItem("Create New State"))
            {
                create_new_state();
            }
            ImGui::EndPopup();
        }

        ImNodes::MiniMap();
		ImNodes::EndNodeEditor();

        if(m_currentGraph)
        {
            int num_selected_nodes = ImNodes::NumSelectedNodes();
            if (num_selected_nodes == 1)
            {
                static std::vector<int> selected_nodes;
                selected_nodes.resize(static_cast<size_t>(num_selected_nodes));
                ImNodes::GetSelectedNodes(selected_nodes.data());
                int index = selected_nodes[0] - 1;
                m_selectedState = (start_node_id != selected_nodes[0]) ?  &m_currentGraph->GetStates()[index] : m_selectedState;
                m_selectedState = new_graph ? nullptr : m_selectedState;
                new_graph = false;
            }
        };

		ImGui::Columns(1);*/


        ImGui::Begin("Timeline Editor");

        float timelineHeight = tracks.size() * 60.0f;
        float headerHeight = 40.0f;

        // Draw timeline header background
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 cursorPos = ImGui::GetCursorScreenPos();
        cursorPos.x += 150.0f;
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
        for (auto& track : tracks) {
            //drawList->AddRectFilled(ImVec2(cursorPos.x, trackY - 20.0f), ImVec2(cursorPos.x + (timelineEnd - timelineStart) * pixelsPerSecond, trackY + track.height), IM_COL32(35, 35, 35, 255));
            ImGui::SetCursorScreenPos(ImVec2(cursorPos.x - 150.0f, trackY));
            ImGui::Button(track.name.c_str(), ImVec2(150.0f, headerHeight));
            ImGui::SetCursorScreenPos(ImVec2(cursorPos.x, trackY - 15.0f));
            //ImGui::Text("%s", track.name.c_str());
            //drawList->AddText(ImVec2(cursorPos.x - 150.0f, trackY + 5.0f), IM_COL32(255, 255, 255, 255), track.name.c_str());

            for (auto& clip : track.clips) {
                track_id += 100;
                float clipX = cursorPos.x + (clip.startTime - timelineStart) * pixelsPerSecond;
                clipX = std::clamp(clipX, cursorPos.x, clipX);
                float clipWidth = clip.duration * pixelsPerSecond;
                float end_time = clip.startTime + clip.duration;
                ImVec2 clipPos(clipX, trackY);
                ImVec2 clipSize(clipX + clipWidth, trackY + track.height);
                
                if (clipWidth <= 0.0f)
                {
                    continue;
                }

                drawList->AddRectFilled(clipPos, clipSize, IM_COL32(100, 150, 250, 255));
                drawList->AddText(ImVec2(clipPos.x + 5.0f, clipPos.y + 5.0f), IM_COL32(255, 255, 255, 255), clip.name.c_str());

                for (auto& keyframe : clip.keyframes) {
                    float keyframeX = clipX + (keyframe.time - clip.startTime) * pixelsPerSecond;
                    drawList->AddCircleFilled(ImVec2(keyframeX, trackY + track.height / 2.0f), 5.0f, IM_COL32(255, 255, 0, 255));
                }

                // Draggable regions for resizing
                float edgeWidth = pixelsPerSecond > 25.0f ? 0.1f * pixelsPerSecond : 5.0f;
                ImVec2 leftEdge(clipPos.x, clipPos.y);
                ImVec2 left_edge_size(leftEdge.x + edgeWidth, leftEdge.y + clipSize.y - clipPos.y);
                ImVec2 rightEdge(clipSize.x - edgeWidth, clipPos.y);
                ImVec2 right_edge_size(clipSize.x, clipSize.y);

                bool is_hovering_left_edge = ImGui::IsMouseHoveringRect(leftEdge, left_edge_size);
                bool is_hovering_right_edge = ImGui::IsMouseHoveringRect(rightEdge, right_edge_size);
                int32_t left_id = track_id + 10;
                int32_t right_id = track_id + 20;
                int32_t clip_id = track_id + 30;

                bool isResizingLeft = is_hovering_left_edge && ImGui::IsMouseDragging(ImGuiMouseButton_Left);
                bool isResizingRight = is_hovering_right_edge && ImGui::IsMouseDragging(ImGuiMouseButton_Left);

                if (is_hovering_left_edge)
                {
                    drawList->AddRectFilled(leftEdge, left_edge_size, IM_COL32(200, 200, 200, 200));

                }
                else if (is_hovering_right_edge)
                {
                    drawList->AddRectFilled(rightEdge, right_edge_size, IM_COL32(200, 200, 200, 200));
                }

                if (isResizingLeft || rect_id == left_id) 
                {
                    rect_id = (rect_id == 0) ? left_id : rect_id;
                    if (rect_id == left_id)
                    {
                        float deltaX = ImGui::GetIO().MouseDelta.x / pixelsPerSecond;
                        clip.startTime = std::clamp(clip.startTime + deltaX, timelineStart, clip.startTime + clip.duration - minClipDuration);
                        clip.duration = std::max(end_time - clip.startTime, minClipDuration);
                    }
                }
                else if (isResizingRight || rect_id == right_id)
                {
                    rect_id = (rect_id == 0) ? right_id : rect_id;
                    if (rect_id == right_id)
                    {
                        float deltaX = ImGui::GetIO().MouseDelta.x / pixelsPerSecond;
                        clip.duration = std::max(clip.duration + deltaX, minClipDuration);
                    }
                }

                // Dragging the entire clip
                if (ImGui::IsMouseHoveringRect(ImVec2( clipPos.x + edgeWidth, clipPos.y), ImVec2(clipSize.x - edgeWidth, clipSize.y )) && ImGui::IsMouseDragging(ImGuiMouseButton_Left) && !isResizingLeft && !isResizingRight) 
                {
                    rect_id = (rect_id == 0) ? clip_id : rect_id;
                    if (rect_id == clip_id)
                    {
                        float deltaX = ImGui::GetIO().MouseDelta.x / pixelsPerSecond;
                        clip.startTime = std::clamp(clip.startTime + deltaX, timelineStart, timelineEnd - clip.duration);
                    }
                }
            }
            trackY += track.height + 10.0f;
            track_id += 1000;
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

        if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) &&  ImGui::IsMouseDragging(ImGuiMouseButton_Right)) {
            float deltaX = ImGui::GetIO().MouseDelta.x / pixelsPerSecond;
            timelineStart = std::max(0.0f, timelineStart - deltaX);
            timelineEnd = std::max(timelineStart + 1.0f, timelineEnd);
        }

        ImGui::End();







		//ImGui::End();


	}

	void AnimationGraphEditor::SetAnimationGraph(Ref<AnimationGraph> graph)
	{
        m_selectedState = nullptr;
        new_graph = true;
        m_links.clear();
        

        TraceEditor* editor = TraceEditor::get_instance();
        if (m_currentGraph)
        {
            std::string db_path = editor->GetCurrentProject()->GetProjectCurrentDirectory() + "/InternalAssetsDB/" + m_currentGraph->GetName() + ".ini";
            ImNodes::SaveCurrentEditorStateToIniFile(db_path.c_str());
        }

        ImNodes::DestroyContext();
        ImNodesContext* new_context = ImNodes::CreateContext();
        ImNodes::SetCurrentContext(new_context);

        if (graph)
        {
            std::string db_path = editor->GetCurrentProject()->GetProjectCurrentDirectory() + "/InternalAssetsDB/" + graph->GetName() + ".ini";
            if (std::filesystem::exists(db_path))
                ImNodes::LoadCurrentEditorStateFromIniFile(db_path.c_str());
        }

        m_currentGraph = graph;
        std::vector<AnimationState>& states = m_currentGraph->GetStates();
        if (states.empty()) return;
        m_links.clear();

        Link link;
        link.id = link_id++;
        link.from = start_output_id; 
        link.to = (m_currentGraph->GetStartIndex() + 1) << 16;
        m_links.push_back(link);

        /*int i = 0;
        for (auto& s : states)
        {


            i++;
        }*/

	}

    void AnimationGraphEditor::SetAnimationGraph(std::string path)
    {
        std::filesystem::path p = path;
        Ref<AnimationGraph> ag = AnimationsManager::get_instance()->GetGraph(p.filename().string());
        if (ag)
        {
            SetAnimationGraph(ag);
        }
        else
        {
            ag = AnimationsSerializer::DeserializeAnimationGraph(p.string());
            if (ag)
            {
                SetAnimationGraph(ag);
            }
        }
    }

    void AnimationGraphEditor::create_new_state()
    {
        if (!m_currentGraph) return;

        std::vector<AnimationState>& states = m_currentGraph->GetStates();
        static int new_name_id = 0;
        if (states.empty())
        {

            AnimationState new_state;
            std::string state_name = "New State" + std::to_string(++new_name_id);
            new_state.SetName(state_name);
            states.push_back(new_state);
            
            ImGuiID state_id = 1;
            Link link;
            link.from = start_output_id;
            link.to = state_id << 16;
            link.id = link_id++;
            m_links.push_back(link);

            return;
        }

        AnimationState new_state;
        std::string state_name = "New State" + std::to_string(++new_name_id);
        new_state.SetName(state_name);
        states.push_back(new_state);

    }

}