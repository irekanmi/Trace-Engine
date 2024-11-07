
#include "AnimationGraphEditor.h"
#include "animation/Animation.h"
#include "resource/AnimationsManager.h"
#include "serialize/AnimationsSerializer.h"
#include "../utils/ImGui_utils.h"
#include "../TraceEditor.h"

#include "imgui.h"
#include "imgui_stdlib.h"
#include "imnodes/imnodes.h"
#include "portable-file-dialogs.h"

namespace trace {

    static int start_node_id = 0;
    static int start_output_id = 1;
    static int link_id = 0;

	bool AnimationGraphEditor::Init()
	{
		ImNodes::GetStyle().LinkLineSegmentsPerLength = 0.0f;

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


float startTime = 0.0f, endTime = 10.0f; // Timeline range
float clipStart = 2.0f, clipEnd = 5.0f;  // Initial clip position
float pixelsPerSecond = 50.0f;

ImGui::Begin("Timeline");

// Draw Timeline Background
ImGui::Text("Timeline");
ImDrawList* drawList = ImGui::GetWindowDrawList();
ImVec2 cursorPos = ImGui::GetCursorScreenPos();
ImVec2 timelineSize(500, 50); // Width and height of the timeline

// Draw a base line
drawList->AddLine(cursorPos, ImVec2(cursorPos.x + timelineSize.x, cursorPos.y), IM_COL32(200, 200, 200, 255), 2.0f);

// Draw keyframes or clips
ImVec2 clipStartPos = ImVec2(cursorPos.x + (clipStart - startTime) * pixelsPerSecond, cursorPos.y);
ImVec2 clipEndPos = ImVec2(cursorPos.x + (clipEnd - startTime) * pixelsPerSecond, cursorPos.y + timelineSize.y);
drawList->AddRectFilled(clipStartPos, clipEndPos, IM_COL32(100, 150, 250, 255));

// Drag functionality (to move the clip horizontally)
if (ImGui::IsItemActive()) {
    ImVec2 delta = ImGui::GetIO().MouseDelta;
    clipStart += delta.x / pixelsPerSecond;
    clipEnd += delta.x / pixelsPerSecond;
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