
#include "AnimationGraphEditor.h"
#include "animation/Animation.h"
#include "resource/AnimationsManager.h"
#include "resource/GenericAssetManager.h"
#include "serialize/AnimationsSerializer.h"
#include "../utils/ImGui_utils.h"
#include "../TraceEditor.h"
#include "core/events/Events.h"
#include "external_utils.h"
#include "reflection/TypeHash.h"
#include "serialize/FileStream.h"
#include "reflection/SerializeTypes.h"
#include "core/defines.h"

#include "imgui.h"
#include "imgui_stdlib.h"
#include "imnodes/imnodes.h"
#include "portable-file-dialogs.h"


namespace trace {

    static int start_node_id = 0x0A0B0C;
    static int start_output_id = 1;
    static int start_link_id = 0xFF00FF0F;
    static int _link_id = 0x0A;
    static int link_id = 0;
    static int output_start_index = 64;
    static bool is_window_focused = false;
    static bool is_delete_pressed = false;

    uint32_t value_color[(int)Animation::ValueType::Max] =
    {
        IM_COL32(255, 204, 153, 128),
        IM_COL32(51, 102, 255, 128),
        IM_COL32(51, 204, 204, 128),
        IM_COL32(153, 204, 0, 128),
        IM_COL32(255, 204, 0, 128),
    };

    uint32_t value_color_hovered[(int)Animation::ValueType::Max] =
    {
        IM_COL32(255, 204, 153, 255),
        IM_COL32(51, 102, 255, 255),
        IM_COL32(51, 204, 204, 255),
        IM_COL32(153, 204, 0, 255),
        IM_COL32(255, 204, 0, 255),
    };

    std::unordered_map<uint64_t, std::string> type_names =
    {
            {
                Reflection::TypeID<Animation::FinalOutputNode>(),
                "Final Output"
            },
            {
                Reflection::TypeID<Animation::GetParameterNode>(),
                "Get Parameter"
            },
            {
                Reflection::TypeID<Animation::StateMachine>(),
                "State Machine"
            },
            {
                Reflection::TypeID<Animation::StateNode>(),
                "State"
            },
            {
                Reflection::TypeID<Animation::TransitionNode>(),
                "Transition"
            },
            {
                Reflection::TypeID<Animation::AnimationSampleNode>(),
                "Animation Sample"
            },
            {
                Reflection::TypeID<Animation::IfNode>(),
                "If Node"
            },
            {
                Reflection::TypeID<Animation::RetargetAnimationNode>(),
                "Retarget Animation Node"
            },
            {
                Reflection::TypeID<Animation::WarpAnimationNode>(),
                "Warp Animation Node"
            }
    };

	bool AnimationGraphEditor::Init()
	{
		//ImNodes::GetStyle().LinkLineSegmentsPerLength = 0.0f;

        trace::EventsSystem::get_instance()->AddEventListener(trace::EventType::TRC_BUTTON_RELEASED, BIND_EVENT_FN(AnimationGraphEditor::OnEvent));
        trace::EventsSystem::get_instance()->AddEventListener(trace::EventType::TRC_KEY_PRESSED, BIND_EVENT_FN(AnimationGraphEditor::OnEvent));
        trace::EventsSystem::get_instance()->AddEventListener(trace::EventType::TRC_KEY_RELEASED, BIND_EVENT_FN(AnimationGraphEditor::OnEvent));

        type_node_render =
        {
            {
                Reflection::TypeID<Animation::FinalOutputNode>(),
                [&](Animation::Node* node)
                {
                    Animation::FinalOutputNode* output = (Animation::FinalOutputNode*)node;
                    int32_t node_index = m_graphNodeIndex[output->GetUUID()];
                    ImNodes::BeginNode(node_index);

                    ImNodes::BeginNodeTitleBar();
                    ImGui::Text("Final Output Pose");
                    ImNodes::EndNodeTitleBar();

                    Animation::NodeInput& input_0 = node->GetInputs()[0];

                    ImNodes::PushColorStyle(ImNodesCol_Pin, value_color[(int)input_0.type]);
                    ImNodes::PushColorStyle(ImNodesCol_PinHovered, value_color_hovered[(int)input_0.type]);
                    ImNodes::BeginInputAttribute( (1 << 24) | node_index , ImNodesPinShape_Quad);
                    ImGui::Text("Pose");
                    ImNodes::EndInputAttribute();
                    ImNodes::PopColorStyle();
                    ImNodes::PopColorStyle();

                    ImNodes::EndNode();
                }
            },
            {
                Reflection::TypeID<Animation::GetParameterNode>(),
                [&](Animation::Node* node)
                {
                    Animation::GetParameterNode* param = (Animation::GetParameterNode*)node;
                    int32_t node_index = m_graphNodeIndex[param->GetUUID()];
                    ImNodes::BeginNode(node_index);

                    ImNodes::BeginNodeTitleBar();
                    ImGui::Text("Get Paramater");
                    ImNodes::EndNodeTitleBar();

                    Animation::NodeOutput& output_0 = node->GetOutputs()[0];

                    int32_t param_index = param->GetParameterIndex();
                    if (param_index != -1)
                    {
                        Animation::Parameter& parameter = m_currentGraph->GetParameters()[param_index];
                        ImNodes::PushColorStyle(ImNodesCol_Pin, value_color[(int)output_0.type]);
                        ImNodes::PushColorStyle(ImNodesCol_PinHovered, value_color_hovered[(int)output_0.type]);
                        ImNodes::BeginOutputAttribute(((output_start_index + output_0.value_index) << 24) | node_index);
                        ImGui::Text(parameter.first.c_str());
                        ImNodes::EndOutputAttribute();
                        ImNodes::PopColorStyle();
                        ImNodes::PopColorStyle();
                    }

                    

                    ImNodes::EndNode();
                }
            },
            {
                Reflection::TypeID<Animation::StateMachine>(),
                [&](Animation::Node* node)
                {
                    Animation::StateMachine* state_machine = (Animation::StateMachine*)node;
                    int32_t node_index = m_graphNodeIndex[state_machine->GetUUID()];
                    ImNodes::BeginNode(node_index);

                    ImNodes::BeginNodeTitleBar();
                    ImGui::Text("State Machine");
                    ImNodes::EndNodeTitleBar();

                    Animation::NodeOutput& output_0 = node->GetOutputs()[0];

                    ImNodes::PushColorStyle(ImNodesCol_Pin, value_color[(int)output_0.type]);
                    ImNodes::PushColorStyle(ImNodesCol_PinHovered, value_color_hovered[(int)output_0.type]);
                    ImNodes::BeginOutputAttribute(((output_start_index + output_0.value_index) << 24) | node_index );
                    ImGui::Text("Pose");
                    ImNodes::EndOutputAttribute();
                    ImNodes::PopColorStyle();
                    ImNodes::PopColorStyle();

                    ImNodes::EndNode();
                }
            },
            {
                Reflection::TypeID<Animation::StateNode>(),
                [&](Animation::Node* node)
                {
                    Animation::StateNode* state = (Animation::StateNode*)node;
                    int32_t node_index = m_graphNodeIndex[state->GetUUID()];
                    ImNodes::BeginNode(node_index);

                    std::string& name = STRING_FROM_ID(state->GetName());
                    ImNodes::BeginNodeTitleBar();
                    ImGui::Text(name.c_str());
                    ImNodes::EndNodeTitleBar();

                    Animation::NodeInput& input_0 = node->GetInputs()[0];

                    ImNodes::PushColorStyle(ImNodesCol_Pin, value_color[(int)input_0.type]);
                    ImNodes::PushColorStyle(ImNodesCol_PinHovered, value_color_hovered[(int)input_0.type]);
                    ImNodes::BeginInputAttribute((1 << 24) | node_index , ImNodesPinShape_Quad);
                    ImGui::Text("Pose");
                    ImNodes::EndInputAttribute();
                    ImNodes::PopColorStyle();
                    ImNodes::PopColorStyle();

                    ImNodes::EndNode();
                }
            },
            {
                Reflection::TypeID<Animation::TransitionNode>(),
                [&](Animation::Node* node)
                {
                    Animation::TransitionNode* transition = (Animation::TransitionNode*)node;
                    int32_t node_index = m_graphNodeIndex[transition->GetUUID()];
                    ImNodes::BeginNode(node_index);

                    ImNodes::BeginNodeTitleBar();
                    ImGui::Text("Transition Node");
                    ImNodes::EndNodeTitleBar();

                    Animation::NodeInput& input_0 = node->GetInputs()[0];

                    ImNodes::PushColorStyle(ImNodesCol_Pin, value_color[(int)input_0.type]);
                    ImNodes::PushColorStyle(ImNodesCol_PinHovered, value_color_hovered[(int)input_0.type]);
                    ImNodes::BeginInputAttribute((1 << 24) | node_index);
                    ImGui::Text("Condition");
                    ImNodes::EndInputAttribute();
                    ImNodes::PopColorStyle();
                    ImNodes::PopColorStyle();

                    ImNodes::EndNode();
                }
            },
            {
                Reflection::TypeID<Animation::AnimationSampleNode>(),
                [&](Animation::Node* node)
                {
                    Animation::AnimationSampleNode* sample_node = (Animation::AnimationSampleNode*)node;
                    int32_t node_index = m_graphNodeIndex[sample_node->GetUUID()];
                    ImNodes::BeginNode(node_index);

                    ImNodes::BeginNodeTitleBar();
                    ImGui::Text("Animation Sample Node");
                    ImNodes::EndNodeTitleBar();

                    Animation::NodeOutput& output_0 = node->GetOutputs()[0];

                    int32_t anim_index = sample_node->GetAnimClipIndex();
                    std::string clip_name = "None(Animation Clip)";

                    if (anim_index != -1)
                    {
                        Ref<AnimationClip> clip = m_currentGraph->GetAnimationDataSet()[anim_index];
                        clip_name = clip->GetName();
                    }
                    ImNodes::PushColorStyle(ImNodesCol_Pin, value_color[(int)output_0.type]);
                    ImNodes::PushColorStyle(ImNodesCol_PinHovered, value_color_hovered[(int)output_0.type]);
                    ImNodes::BeginOutputAttribute(((output_start_index + output_0.value_index) << 24) | node_index);
                    ImGui::Text(clip_name.c_str());
                    ImNodes::EndOutputAttribute();
                    ImNodes::PopColorStyle();
                    ImNodes::PopColorStyle();

                    

                    ImNodes::EndNode();
                }
            },
            {
                Reflection::TypeID<Animation::IfNode>(),
                [&](Animation::Node* node)
                {
                    Animation::IfNode* sample_node = (Animation::IfNode*)node;
                    int32_t node_index = m_graphNodeIndex[sample_node->GetUUID()];
                    ImNodes::BeginNode(node_index);

                    ImNodes::BeginNodeTitleBar();
                    ImGui::Text("If Node");
                    ImNodes::EndNodeTitleBar();

                    Animation::NodeInput& input_0 = node->GetInputs()[0];

                    ImNodes::PushColorStyle(ImNodesCol_Pin, value_color[(int)input_0.type]);
                    ImNodes::PushColorStyle(ImNodesCol_PinHovered, value_color_hovered[(int)input_0.type]);
                    ImNodes::BeginInputAttribute((1 << 24) | node_index);
                    ImGui::Text("Condition");
                    ImNodes::EndInputAttribute();
                    ImNodes::PopColorStyle();
                    ImNodes::PopColorStyle();

                    Animation::NodeOutput& output_0 = node->GetOutputs()[0];


                    ImNodes::PushColorStyle(ImNodesCol_Pin, value_color[(int)output_0.type]);
                    ImNodes::PushColorStyle(ImNodesCol_PinHovered, value_color_hovered[(int)output_0.type]);
                    ImNodes::BeginOutputAttribute(((output_start_index + output_0.value_index) << 24) | node_index);
                    ImGui::Text("True");
                    ImNodes::EndOutputAttribute();
                    ImNodes::PopColorStyle();
                    ImNodes::PopColorStyle();

                    Animation::NodeOutput& output_1 = node->GetOutputs()[1];


                    ImNodes::PushColorStyle(ImNodesCol_Pin, value_color[(int)output_1.type]);
                    ImNodes::PushColorStyle(ImNodesCol_PinHovered, value_color_hovered[(int)output_1.type]);
                    ImNodes::BeginOutputAttribute(((output_start_index + output_1.value_index) << 24) | node_index);
                    ImGui::Text("False");
                    ImNodes::EndOutputAttribute();
                    ImNodes::PopColorStyle();
                    ImNodes::PopColorStyle();



                    ImNodes::EndNode();
                }
            },
            {
                Reflection::TypeID<Animation::RetargetAnimationNode>(),
                [&](Animation::Node* node)
                {
                    Animation::RetargetAnimationNode* sample_node = (Animation::RetargetAnimationNode*)node;
                    int32_t node_index = m_graphNodeIndex[sample_node->GetUUID()];
                    ImNodes::BeginNode(node_index);

                    ImNodes::BeginNodeTitleBar();
                    ImGui::Text("Retarget Animation Node");
                    ImNodes::EndNodeTitleBar();

                    Animation::NodeOutput& output_0 = node->GetOutputs()[0];

                    Ref<AnimationClip> clip = sample_node->GetAnimationClip();
                    std::string clip_name = "None(Animation Clip)";

                    if (clip)
                    {
                        clip_name = clip->GetName();
                    }
                    ImNodes::PushColorStyle(ImNodesCol_Pin, value_color[(int)output_0.type]);
                    ImNodes::PushColorStyle(ImNodesCol_PinHovered, value_color_hovered[(int)output_0.type]);
                    ImNodes::BeginOutputAttribute(((output_start_index + output_0.value_index) << 24) | node_index);
                    ImGui::Text(clip_name.c_str());
                    ImNodes::EndOutputAttribute();
                    ImNodes::PopColorStyle();
                    ImNodes::PopColorStyle();



                    ImNodes::EndNode();
                }
            },
            {
                Reflection::TypeID<Animation::WarpAnimationNode>(),
                [&](Animation::Node* node)
                {
                    Animation::WarpAnimationNode* sample_node = (Animation::WarpAnimationNode*)node;
                    int32_t node_index = m_graphNodeIndex[sample_node->GetUUID()];
                    ImNodes::BeginNode(node_index);

                    ImNodes::BeginNodeTitleBar();
                    ImGui::Text("Warp Animation Node");
                    ImNodes::EndNodeTitleBar();

                    Animation::NodeOutput& output_0 = node->GetOutputs()[0];

                    Ref<AnimationClip> clip = sample_node->GetAnimationClip();
                    std::string clip_name = "None(Animation Clip)";

                    if (clip)
                    {
                        clip_name = clip->GetName();
                    }
                    ImNodes::PushColorStyle(ImNodesCol_Pin, value_color[(int)output_0.type]);
                    ImNodes::PushColorStyle(ImNodesCol_PinHovered, value_color_hovered[(int)output_0.type]);
                    ImNodes::BeginOutputAttribute(((output_start_index + output_0.value_index) << 24) | node_index);
                    ImGui::Text(clip_name.c_str());
                    ImNodes::EndOutputAttribute();
                    ImNodes::PopColorStyle();
                    ImNodes::PopColorStyle();



                    ImNodes::EndNode();
                }
            }

        };

        node_selected_render =
        {
            {
                Reflection::TypeID<Animation::GetParameterNode>(),
                [&](Animation::Node* node)
                {
                    Animation::GetParameterNode* param = (Animation::GetParameterNode*)node;
                    int32_t node_index = m_graphNodeIndex[param->GetUUID()];
                    
                    std::string parameter_name = "None(Parameter)";
                    int32_t param_index = param->GetParameterIndex();
                    if (param_index != -1)
                    {
                        Animation::Parameter& parameter = m_currentGraph->GetParameters()[param_index];
                        parameter_name = parameter.first;
                    }
                    ImGui::Text("Parameter Name: ");
                    ImGui::SameLine();
                    if (ImGui::Button(parameter_name.c_str()))
                    {
                        ImGui::OpenPopup("Parameters Drop Down");
                    }

                    param_index = this->paramters_drop_down(node_index);

                    if (param_index != -1)
                    {
                        param->SetParameterIndex(param_index, m_currentGraph.get());
                    }
                    
                }
            },
            {
                Reflection::TypeID<Animation::StateNode>(),
                [&](Animation::Node* node)
                {
                    static bool change_name = false;
                    TraceEditor* editor = TraceEditor::get_instance();
                    Animation::StateNode* state = (Animation::StateNode*)node;
                    int32_t node_index = m_graphNodeIndex[state->GetUUID()];
                    std::string& name = STRING_FROM_ID(state->GetName());
                    
                    ImGui::Text("Name: ");
                    ImGui::SameLine();
                    if (ImGui::Button(name.c_str()))
                    {
                        change_name = true;
                    }

                    if (change_name)
                    {
                        std::string res;
                        if (editor->InputTextPopup("State Name", res))
                        {
                            if (!res.empty())
                            {
                                state->SetName(STR_ID(res));
                            }
                        }
                        else
                        {
                            change_name = false;
                        }
                    }



                }
            },
            {
                Reflection::TypeID<Animation::TransitionNode>(),
                [&](Animation::Node* node)
                {
                    Animation::TransitionNode* transition = (Animation::TransitionNode*)node;
                    int32_t node_index = m_graphNodeIndex[transition->GetUUID()];
                    
                    float duration = transition->GetDuration();

                    if (ImGui::DragFloat("Duration", &duration, 0.05f))
                    {
                        transition->SetDuration(duration);
                    }

                }
            },
            {
                Reflection::TypeID<Animation::AnimationSampleNode>(),
                [&](Animation::Node* node)
                {
                    Animation::AnimationSampleNode* sample_node = (Animation::AnimationSampleNode*)node;
                    int32_t node_index = m_graphNodeIndex[sample_node->GetUUID()];

                    int32_t anim_index = sample_node->GetAnimClipIndex();
                    std::string clip_name = "None(Animation Clip)";
                    if (anim_index != -1)
                    {
                        Ref<AnimationClip> clip = m_currentGraph->GetAnimationDataSet()[anim_index];
                        clip_name = clip->GetName();
                    }

                    ImGui::Text("Animation Clip: ");
                    ImGui::SameLine();

                    if (ImGui::Button(clip_name.c_str()))
                    {
                        ImGui::OpenPopup("Animations Drop Down");
                    }

                    anim_index = this->animations_drop_down(node_index);
                    if (anim_index != -1)
                    {
                        sample_node->SetAnimationClip(anim_index, m_currentGraph.get());
                    }

                    ImGui::Text("Loop Clip: ");
                    ImGui::SameLine();
                    bool loop = sample_node->GetLooping();
                    if (ImGui::Checkbox("##Looping", &loop))
                    {
                        sample_node->SetLooping(loop);
                    }

                }
            },
            {
                Reflection::TypeID<Animation::RetargetAnimationNode>(),
                [&](Animation::Node* node)
                {
                    Animation::RetargetAnimationNode* sample_node = (Animation::RetargetAnimationNode*)node;
                    int32_t node_index = m_graphNodeIndex[sample_node->GetUUID()];

                    Ref<AnimationClip> clip = sample_node->GetAnimationClip();
                    std::string clip_name = "None(Animation Clip)";
                    if (clip)
                    {
                        clip_name = clip->GetName();
                    }

                    ImGui::Text("Animation Clip: ");
                    ImGui::SameLine();

                    if (ImGui::Button(clip_name.c_str()))
                    {
                        
                    }

                    if (Ref<AnimationClip> new_clip = ImGuiDragDropResource<AnimationClip>(ANIMATION_CLIP_FILE_EXTENSION))
                    {
                        sample_node->SetAnimationClip(new_clip);
                    }

                    Ref<Animation::Skeleton> skeleton = sample_node->GetSkeleton();
                    std::string skeleton_name = "None(Skeleton)";
                    if (skeleton)
                    {
                        skeleton_name = skeleton->GetName();
                    }

                    ImGui::Text("Skeleton: ");
                    ImGui::SameLine();

                    if (ImGui::Button(skeleton_name.c_str()))
                    {

                    }

                    if (Ref<Animation::Skeleton> new_skeleton = ImGuiDragDropResource<Animation::Skeleton>(SKELETON_FILE_EXTENSION))
                    {
                        sample_node->SetSkeleton(new_skeleton);
                    }

                    

                }
            },
            {
                Reflection::TypeID<Animation::WarpAnimationNode>(),
                [&](Animation::Node* node)
                {
                    Animation::WarpAnimationNode* sample_node = (Animation::WarpAnimationNode*)node;
                    int32_t node_index = m_graphNodeIndex[sample_node->GetUUID()];

                    Ref<AnimationClip> clip = sample_node->GetAnimationClip();
                    std::string clip_name = "None(Animation Clip)";
                    if (clip)
                    {
                        clip_name = clip->GetName();
                    }

                    ImGui::Text("Animation Clip: ");
                    ImGui::SameLine();

                    if (ImGui::Button(clip_name.c_str()))
                    {

                    }

                    if (Ref<AnimationClip> new_clip = ImGuiDragDropResource<AnimationClip>(ANIMATION_CLIP_FILE_EXTENSION))
                    {
                        sample_node->SetAnimationClip(new_clip);
                    }

                }
            }
        };

		return true;
	}

	void AnimationGraphEditor::Shutdown()
	{
        TraceEditor* editor = TraceEditor::get_instance();
        if (m_currentGraph)
        {
            if (m_currentNode)
            {
                free_current_node();
            }
            /*std::string db_path = editor->GetCurrentProject()->GetProjectCurrentDirectory() + "/InternalAssetsDB/" + m_currentGraph->GetName() + ".ini";
            ImNodes::SaveCurrentEditorStateToIniFile(db_path.c_str());*/

            UUID asset_id = GetUUIDFromName(m_currentGraph->GetName());
            std::string file_path = GetPathFromUUID(asset_id).string();
            AnimationsSerializer::SerializeAnimGraph(m_currentGraph, file_path);
        }
	}

	void AnimationGraphEditor::Update(float deltaTime)
	{
	}

    void AnimationGraphEditor::OnEvent(Event* p_event)
    {
        if (!is_window_focused)
        {
            return;
        }

        switch (p_event->GetEventType())
        {
        case EventType::TRC_BUTTON_RELEASED:
        {
            MouseReleased* release = reinterpret_cast<MouseReleased*>(p_event);

            if (release->GetButton() == Buttons::BUTTON_LEFT)
            {
                
            }

            break;
        }
        case EventType::TRC_KEY_RELEASED:
        {
            HandleKeyReleased(p_event);
            break;
        }
        case EventType::TRC_KEY_PRESSED:
        {
            HandleKeyPressed(p_event);
            break;
        }
        }

    }

    void AnimationGraphEditor::HandleKeyReleased(Event* p_event)
    {

        KeyReleased* release = reinterpret_cast<KeyReleased*>(p_event);

        switch (release->GetKeyCode())
        {
        case KEY_DELETE:
        {
            is_delete_pressed = false;
            break;
        }
        }
    }

    void AnimationGraphEditor::HandleKeyPressed(Event* p_event)
    {
        KeyPressed* pressed = reinterpret_cast<KeyPressed*>(p_event);

        switch (pressed->GetKeyCode())
        {
        case KEY_DELETE:
        {
            is_delete_pressed = true;
            break;
        }
        }
    }

	void AnimationGraphEditor::Render(float deltaTime)
	{
        TraceEditor* editor = TraceEditor::get_instance();

		ImGui::Begin("Animation Graph Editor");
		ImGui::Columns(2);

		if (m_currentGraph)
		{
            std::string graph_name = m_currentGraph->GetName();
            

            ImGui::Button(graph_name.c_str());
            Ref<Animation::Graph> result = ImGuiDragDropResource<Animation::Graph>(ANIMATION_GRAPH_FILE_EXTENSION);
            if (result)
            {
                SetAnimationGraph(result);
            }

            if (ImGui::TreeNode("Graph Data"))
            {
                render_graph_data();
                ImGui::TreePop();
            }
            

            
            if (ImGui::Button("Save"))
            {
                UUID asset_id = GetUUIDFromName(m_currentGraph->GetName());
                std::string file_path = GetPathFromUUID(asset_id).string();
                AnimationsSerializer::SerializeAnimGraph(m_currentGraph, file_path);
            }

            if (m_selectedNode)
            {
                ImGui::InvisibleButton("Seperator", { 10.0f, GetLineHeight() });
                auto it = node_selected_render.find(m_selectedNode->GetTypeID());
                if (it != node_selected_render.end())
                {
                    it->second(m_selectedNode);
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
            Ref<Animation::Graph> result = ImGuiDragDropResource<Animation::Graph>(".trcag");
            if (result)
            {
                SetAnimationGraph(result);
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
                        Ref<Animation::Graph> graph = GenericAssetManager::get_instance()->CreateAssetHandle_<Animation::Graph>(path.string());
                        AnimationsSerializer::SerializeAnimGraph(graph, path.string());
                        SetAnimationGraph(graph);
                    }

                }
            }
		}

		ImGui::NextColumn();
        if (m_currentGraph)
        {
            for (uint32_t i = 0; i < m_currentGraphNodePath.size(); i++)
            {

                Animation::Node* node = m_currentGraphNodePath[i];
                if (ImGui::Button(type_names[node->GetTypeID()].c_str()))
                {
                    set_current_node(node);
                    m_currentGraphNodePath.resize(i);
                    break;
                }
                ImGui::SameLine();
                ImGui::Text("/");
                if (i != (m_currentGraphNodePath.size() - 1))
                {
                    ImGui::SameLine();
                }

            }
        }
		ImNodes::BeginNodeEditor();
        

		if (m_currentGraph)
		{
            
            const bool open_popup = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) &&
                ImNodes::IsEditorHovered() &&
                ImGui::IsMouseClicked(ImGuiMouseButton_Right);

            if (!ImGui::IsAnyItemHovered() && open_popup)
            {
                ImGui::OpenPopup("add node");
            }

            if (m_currentNode->GetTypeID() == Reflection::TypeID<Animation::StateMachine>())
            {
                render_state_machine();
            }
            else
            {

                auto it = type_node_render.find(m_currentNode->GetTypeID());
                if (it != type_node_render.end())
                {
                    it->second(m_currentNode);
                }

                for (UUID& id : m_currentNodeChildren)
                {
                    Animation::Node* node = m_currentGraph->GetNode(id);
                    auto child_it = type_node_render.find(node->GetTypeID());
                    if (child_it != type_node_render.end())
                    {
                        child_it->second(node);
                    }
                }

                for (auto& link : m_currentNodeLinks)
                {
                    ImNodes::PushColorStyle(ImNodesCol_Link, value_color[(int)link.value_type]);
                    ImNodes::PushColorStyle(ImNodesCol_LinkHovered, value_color_hovered[(int)link.value_type]);
                    ImNodes::PushColorStyle(ImNodesCol_LinkSelected, value_color_hovered[(int)link.value_type]);
                    ImNodes::Link(link.id, link.from, link.to);
                    ImNodes::PopColorStyle();
                    ImNodes::PopColorStyle();
                    ImNodes::PopColorStyle();
                }
            }

            if (m_currentNode->GetTypeID() == Reflection::TypeID<Animation::StateMachine>())
            {
                handle_state_machine_pop_up();
            }
            else
            {
                if (ImGui::BeginPopup("add node"))
                {
                    if (ImGui::MenuItem("State Machine"))
                    {
                        UUID node_id = m_currentGraph->CreateNode<Animation::StateMachine>();
                        add_new_node(node_id);
                    }
                    if (ImGui::MenuItem("Animation Sample"))
                    {
                        UUID node_id = m_currentGraph->CreateNode<Animation::AnimationSampleNode>();
                        add_new_node(node_id);
                    }
                    if (ImGui::MenuItem("Get Parameter"))
                    {
                        UUID node_id = m_currentGraph->CreateNode<Animation::GetParameterNode>();
                        add_new_node(node_id);
                    }
                    if (ImGui::MenuItem("If Node"))
                    {
                        UUID node_id = m_currentGraph->CreateNode<Animation::IfNode>();
                        add_new_node(node_id);
                    }
                    if (ImGui::MenuItem("Retarget Node"))
                    {
                        UUID node_id = m_currentGraph->CreateNode<Animation::RetargetAnimationNode>();
                        add_new_node(node_id);
                    }
                    if (ImGui::MenuItem("Warp Node"))
                    {
                        UUID node_id = m_currentGraph->CreateNode<Animation::WarpAnimationNode>();
                        add_new_node(node_id);
                    }
                    ImGui::EndPopup();
                }
            }
		}


        ImNodes::MiniMap();

        is_window_focused = ImGui::IsWindowFocused() && ImGui::IsWindowHovered();
		ImNodes::EndNodeEditor();

        if(m_currentGraph)
        {            

            if (m_currentNode->GetTypeID() == Reflection::TypeID<Animation::StateMachine>())
            {
                handle_state_machine_actions();

            }
            else
            {
                int num_selected_nodes = ImNodes::NumSelectedNodes();
                if (num_selected_nodes > 0)
                {
                    static std::vector<int> selected_nodes;
                    selected_nodes.resize(static_cast<size_t>(num_selected_nodes));
                    ImNodes::GetSelectedNodes(selected_nodes.data());
                    int32_t index = selected_nodes[0];
                    UUID node_id = m_graphIndex[index];
                    m_selectedNode = m_currentGraph->GetNode(node_id);

                    if (is_delete_pressed)
                    {
                        m_selectedNode = nullptr;
                        for (int32_t& i : selected_nodes)
                        {
                            node_id = m_graphIndex[i];
                            Animation::Node* node = m_currentGraph->GetNode(node_id);
                            if (node == m_currentNode)
                            {
                                continue;
                            }
                            m_currentGraph->DestroyNode(node_id);
                            delete_node(node_id);

                            ImNodes::ClearNodeSelection(i);
                        }
                    }
                }

                if (m_selectedNode)
                {
                    int32_t node_index = m_graphNodeIndex[m_selectedNode->GetUUID()];
                    if (ImNodes::IsNodeHovered(&node_index) && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                    {
                        set_current_node(m_selectedNode);
                        m_selectedNode = nullptr;
                    }
                }

                int32_t start_attr, end_attr;
                if (ImNodes::IsLinkCreated(&start_attr, &end_attr))
                {
                    int32_t mask = ~(~0 << 24);
                    int32_t from_node_index = start_attr & mask;
                    int32_t to_node_index = end_attr & mask;

                    int32_t from_value_index = ((~mask & start_attr) >> 24) - output_start_index;
                    int32_t to_index = ((~mask & end_attr) >> 24) - 1;

                    Animation::Node* from_node = m_currentGraph->GetNode(m_graphIndex[from_node_index]);
                    Animation::Node* to_node = m_currentGraph->GetNode(m_graphIndex[to_node_index]);

                    Animation::NodeInput& input = to_node->GetInputs()[to_index];
                    Animation::NodeOutput& output = from_node->GetOutputs()[from_value_index];
                    if (input.type == output.type && input.node_id == 0)
                    {
                        input.node_id = from_node->GetUUID();
                        input.value_index = from_value_index;

                        Link link = {};
                        link.from = start_attr;
                        link.to = end_attr;
                        link.id = static_cast<int32_t>(m_currentNodeLinks.size());
                        m_currentNodeLinks.push_back(link);
                    }


                }

                int num_selected_links = ImNodes::NumSelectedLinks();
                if (num_selected_links > 0)
                {
                    static std::vector<int> selected_links;
                    selected_links.resize(static_cast<size_t>(num_selected_links));
                    ImNodes::GetSelectedLinks(selected_links.data());
                    int32_t link_id = selected_links[0];
                    
                    if (is_delete_pressed)
                    {
                        for (int32_t& _id : selected_links)
                        {
                            link_id = _id;
                            Link& link = m_currentNodeLinks[link_id];
                            int32_t mask = ~(~0 << 24);
                            int32_t from_node_index = link.from & mask;
                            int32_t to_node_index = link.to & mask;

                            int32_t from_value_index = ((~mask & link.from) >> 24) - output_start_index;
                            int32_t to_index = ((~mask & link.to) >> 24) - 1;

                            Animation::Node* from_node = m_currentGraph->GetNode(m_graphIndex[from_node_index]);
                            Animation::Node* to_node = m_currentGraph->GetNode(m_graphIndex[to_node_index]);

                            Animation::NodeInput& input = to_node->GetInputs()[to_index];
                            input.node_id = 0;
                            input.value_index = -1;

                            m_currentNodeLinks[link_id] = m_currentNodeLinks.back();
                            m_currentNodeLinks.pop_back();
                            ImNodes::ClearLinkSelection(_id);
                        }
                        
                    }
                }

                

                
            }
        };

        

		ImGui::Columns(1);
        



		ImGui::End();


	}

	void AnimationGraphEditor::SetAnimationGraph(Ref<Animation::Graph> graph)
	{
        if (!graph)
        {
            return;
        }

        TraceEditor* editor = TraceEditor::get_instance();
        if (m_currentGraph)
        {
            free_current_node();

            std::string db_path = editor->GetCurrentProject()->GetProjectCurrentDirectory() + "/InternalAssetsDB/" + m_currentGraph->GetName() + ".agbin";
            FileStream stream(db_path, FileMode::WRITE);
            Reflection::Serialize(m_graphCurrentIndex, &stream, nullptr, Reflection::SerializationFormat::BINARY);
            Reflection::Serialize(m_graphIndex, &stream, nullptr, Reflection::SerializationFormat::BINARY);
            Reflection::Serialize(m_graphNodeIndex, &stream, nullptr, Reflection::SerializationFormat::BINARY);
        }

        m_currentGraph = graph;
        new_graph = true;
        m_graphCurrentIndex = 0;
        m_graphIndex.clear();
        m_graphNodeIndex.clear();
        

        std::string db_path = editor->GetCurrentProject()->GetProjectCurrentDirectory() + "/InternalAssetsDB/" + m_currentGraph->GetName() + ".agbin";
        if (std::filesystem::exists(db_path))
        {
            FileStream stream(db_path, FileMode::READ);
            Reflection::Deserialize(m_graphCurrentIndex, &stream, nullptr, Reflection::SerializationFormat::BINARY);
            Reflection::Deserialize(m_graphIndex, &stream, nullptr, Reflection::SerializationFormat::BINARY);
            Reflection::Deserialize(m_graphNodeIndex, &stream, nullptr, Reflection::SerializationFormat::BINARY);
        }
        else
        {
            generate_graph_node_id();
        }

      

        set_current_node(m_currentGraph->GetRootNode());



	}

    void AnimationGraphEditor::SetAnimationGraph(std::string path)
    {
        std::filesystem::path p = path;
        Ref<Animation::Graph> ag = AnimationsSerializer::DeserializeAnimGraph(path);
        if (ag)
        {
            SetAnimationGraph(ag);
        }

    }

    void AnimationGraphEditor::generate_current_node_children()
    {
        if (!m_currentGraph)
        {
            return;
        }

        if (!m_currentNode)
        {
            return;
        }

        m_currentNodeChildren.clear();
        m_currentNodeLinks.clear();

        switch (m_currentNode->GetTypeID())
        {
        case Reflection::TypeID<Animation::FinalOutputNode>():
        case Reflection::TypeID<Animation::GetParameterNode>():
        case Reflection::TypeID<Animation::StateNode>():
        case Reflection::TypeID<Animation::TransitionNode>():
        case Reflection::TypeID<Animation::AnimationSampleNode>():
        case Reflection::TypeID<Animation::IfNode>():
        {
            Animation::Node* current_node = m_currentNode;
            for (uint32_t i = 0; i < current_node->GetInputs().size(); i++)
            {
                Animation::NodeInput& input = current_node->GetInputs()[i];
                if (input.node_id != 0)
                {
                    m_currentNodeChildren.push_back(input.node_id);
                    add_child_node(m_currentGraph->GetNode(input.node_id));
                }
            }
            break;
        }

        case Reflection::TypeID<Animation::StateMachine>():
        {
            Animation::StateMachine* state_machine = (Animation::StateMachine*)m_currentNode;
            for (UUID& state_id : state_machine->GetStates())
            {
                m_currentNodeChildren.push_back(state_id);
            }

            

            break;
        }

        }

    }

    void AnimationGraphEditor::generate_current_node_links()
    {
        if (!m_currentGraph)
        {
            return;
        }

        if (!m_currentNode)
        {
            return;
        }

        m_currentNodeLinks.clear();

        switch (m_currentNode->GetTypeID())
        {
        case Reflection::TypeID<Animation::FinalOutputNode>():
        case Reflection::TypeID<Animation::GetParameterNode>():
        case Reflection::TypeID<Animation::StateNode>():
        case Reflection::TypeID<Animation::TransitionNode>():
        case Reflection::TypeID<Animation::AnimationSampleNode>():
        case Reflection::TypeID<Animation::IfNode>():
        {
            Animation::Node* current_node = m_currentNode;
            for (uint32_t i = 0; i < current_node->GetInputs().size(); i++)
            {
                Animation::NodeInput& input = current_node->GetInputs()[i];
                if (input.node_id != 0)
                {
                    Link link = {};
                    link.to = ((i + 1) << 24) | m_graphNodeIndex[current_node->GetUUID()];
                    link.from = ((input.value_index + output_start_index) << 24) | m_graphNodeIndex[input.node_id];
                    link.id = static_cast<int32_t>(m_currentNodeLinks.size());
                    link.value_type = input.type;
                    m_currentNodeLinks.push_back(link);

                    add_child_links(m_currentGraph->GetNode(input.node_id));
                }
            }
            break;
        }

        case Reflection::TypeID<Animation::StateMachine>():
        {
            Animation::StateMachine* state_machine = (Animation::StateMachine*)m_currentNode;
            for (UUID& state_id : state_machine->GetStates())
            {
                Animation::StateNode* state = (Animation::StateNode*)m_currentGraph->GetNode(state_id);
                for (UUID& transition_id : state->GetTransitions())
                {
                    Animation::TransitionNode* transition = (Animation::TransitionNode*)m_currentGraph->GetNode(transition_id);
                    if (transition->GetFromState() != state_id)
                    {
                        continue;
                    }
                    Link link = {};
                    link.from = ((_link_id + 2) << 24) | m_graphNodeIndex[state_id];
                    int32_t to_node = ((_link_id + 1) << 24) | m_graphNodeIndex[transition->GetTargetState()];
                    link.to = to_node;
                    link.id = m_graphNodeIndex[transition->GetUUID()];
                    m_currentNodeLinks.push_back(link);
                }
            }
            Animation::EntryNode* entry_node = (Animation::EntryNode*)m_currentGraph->GetNode(state_machine->GetEntryNode());
            if (!entry_node->GetInputs().empty())
            {
                Animation::NodeInput& input = entry_node->GetInputs().back();
                if (input.node_id != 0)
                {
                    Link link = {};
                    link.from = start_output_id;
                    int32_t to_node = ((_link_id + 1) << 24) | m_graphNodeIndex[input.node_id];
                    link.to = to_node;
                    link.id = start_link_id;
                    m_currentNodeLinks.push_back(link);
                }
            }
            



            break;
        }

        }

    }

    void AnimationGraphEditor::generate_graph_node_id()
    {
        if (!m_currentGraph)
        {
            return;
        }

        m_graphCurrentIndex = 1;
        for (auto& [id, node] : m_currentGraph->GetNodes())
        {
            m_graphNodeIndex[id] = m_graphCurrentIndex;
            m_graphIndex[m_graphCurrentIndex] = id;

            ++m_graphCurrentIndex;

        }

        
    }

    void AnimationGraphEditor::add_child_node(Animation::Node* current_node)
    {
        TRC_ASSERT(current_node != nullptr, "Invalid node pointer Function: {}", __FUNCTION__);
        for (uint32_t i = 0; i < current_node->GetInputs().size(); i++)
        {
            Animation::NodeInput& input = current_node->GetInputs()[i];
            if (input.node_id != 0)
            {
                m_currentNodeChildren.push_back(input.node_id);
                add_child_node(m_currentGraph->GetNode(input.node_id));
            }
        }
    }

    void AnimationGraphEditor::add_child_links(Animation::Node* current_node)
    {
        TRC_ASSERT(current_node != nullptr, "Invalid node pointer Function: {}", __FUNCTION__);
        for (uint32_t i = 0; i < current_node->GetInputs().size(); i++)
        {
            Animation::NodeInput& input = current_node->GetInputs()[i];
            if (input.node_id != 0)
            {
                Link link = {};
                link.to = ((i + 1) << 24) | m_graphNodeIndex[current_node->GetUUID()];
                link.from = ((input.value_index + output_start_index) << 24) | m_graphNodeIndex[input.node_id];
                link.id = static_cast<int32_t>(m_currentNodeLinks.size());
                link.value_type = input.type;
                m_currentNodeLinks.push_back(link);
                add_child_links(m_currentGraph->GetNode(input.node_id));
            }
        }
    }

    void AnimationGraphEditor::set_current_node(Animation::Node* current_node)
    {
        if (m_currentNode)
        {
            m_currentGraphNodePath.push_back(m_currentNode);
            free_current_node();
            
        }

        m_currentNode = current_node;

        m_currentNodeChildren.clear();
        TraceEditor* editor = TraceEditor::get_instance();
        std::string node_db_path = editor->GetCurrentProject()->GetProjectCurrentDirectory() + "/InternalAssetsDB/" + m_currentGraph->GetName() + std::to_string(uint64_t(m_currentNode->GetUUID())) + ".trnodes";
        if (std::filesystem::exists(node_db_path))
        {
            FileStream stream(node_db_path, FileMode::READ);
            Reflection::DeserializeContainer(m_currentNodeChildren, &stream, nullptr, Reflection::SerializationFormat::BINARY);
            generate_current_node_links();
        }
        else
        {
            generate_current_node_children();
            generate_current_node_links();

        }


        ImNodes::DestroyContext();
        ImNodesContext* new_context = ImNodes::CreateContext();
        ImNodes::SetCurrentContext(new_context);


        std::string db_path = editor->GetCurrentProject()->GetProjectCurrentDirectory() + "/InternalAssetsDB/" + m_currentGraph->GetName() + std::to_string(uint64_t(m_currentNode->GetUUID())) + ".ini";
        if (std::filesystem::exists(db_path))
        {
            ImNodes::LoadCurrentEditorStateFromIniFile(db_path.c_str());
        }

    }

    void AnimationGraphEditor::free_current_node()
    {
        
        TraceEditor* editor = TraceEditor::get_instance();

        std::string node_db_path = editor->GetCurrentProject()->GetProjectCurrentDirectory() + "/InternalAssetsDB/" + m_currentGraph->GetName() + std::to_string(uint64_t(m_currentNode->GetUUID())) + ".trnodes";
        FileStream stream(node_db_path, FileMode::WRITE);
        Reflection::SerializeContainer(m_currentNodeChildren, &stream, nullptr, Reflection::SerializationFormat::BINARY);

        std::string db_path = editor->GetCurrentProject()->GetProjectCurrentDirectory() + "/InternalAssetsDB/" + m_currentGraph->GetName() + std::to_string(uint64_t(m_currentNode->GetUUID())) + ".ini";
        ImNodes::SaveCurrentEditorStateToIniFile(db_path.c_str());

        m_currentNodeChildren.clear();
        m_currentNodeChildren.clear();
        m_currentNode = nullptr;
        m_selectedNode = nullptr;
    }

    void AnimationGraphEditor::delete_node(UUID node_id)
    {
        if (!m_currentGraph)
        {
            return;
        }

        int32_t node_index = m_graphNodeIndex[node_id];
        for (Link& link : m_currentNodeLinks)
        {
            int32_t mask = ~(~0 << 24);
            int32_t to_index = link.to & mask;
            int32_t from_index = link.from & mask;
            if (node_index == from_index)
            {
                UUID to_id = m_graphIndex[to_index];
                Animation::Node* to_node = m_currentGraph->GetNode(to_id);
                for (Animation::NodeInput& input : to_node->GetInputs())
                {
                    if (input.node_id == node_id)
                    {
                        input.node_id = 0;
                    }
                }
            }
        }
        

        node_index = -1;
        auto it = std::find_if(m_currentNodeChildren.begin(), m_currentNodeChildren.end(), [&node_id, &node_index](UUID& val)
            {
                ++node_index;
                return val == node_id;
            });

        
        m_currentNodeChildren[node_index] = m_currentNodeChildren.back();
        m_currentNodeChildren.pop_back();

        m_graphNodeIndex.erase(node_id);
        m_graphIndex.erase(node_index);

        generate_current_node_links();
        
    }

    int32_t AnimationGraphEditor::paramters_drop_down(int32_t id)
    {
        int32_t result = -1;

        if (ImGui::BeginPopup("Parameters Drop Down"))
        {
            for (int32_t i = 0; i < m_currentGraph->GetParameters().size(); i++)
            {
                Animation::Parameter& param = m_currentGraph->GetParameters()[i];
                if (ImGui::MenuItem(param.first.c_str()))
                {
                    result = i;
                    break;
                }
            }
            ImGui::EndPopup();
        }

        return result;
    }

    int32_t AnimationGraphEditor::animations_drop_down(int32_t id)
    {
        int32_t result = -1;

        if (ImGui::BeginPopup("Animations Drop Down"))
        {
            for (int32_t i = 0; i < m_currentGraph->GetAnimationDataSet().size(); i++)
            {
               Ref<AnimationClip>& clip = m_currentGraph->GetAnimationDataSet()[i];
                if (ImGui::MenuItem(clip->GetName().c_str()))
                {
                    result = i;
                    break;
                }
            }
            ImGui::EndPopup();
        }

        return result;
    }

    void AnimationGraphEditor::render_graph_data()
    {
        Ref<Animation::Skeleton> skeleton = m_currentGraph->GetSkeleton();
        std::string skeleton_name = "None(Skeleton)";
        if (skeleton)
        {
            skeleton_name = skeleton->GetName();
        }

        ImGui::Text("Skeleton: ");
        ImGui::SameLine();
        ImGui::Button(skeleton_name.c_str());
        skeleton = ImGuiDragDropResource<Animation::Skeleton>(".trcsk");
        if (skeleton)
        {
            m_currentGraph->SetSkeleton(skeleton);
        }

        ImGui::Text("Animations: ");
        std::vector<Ref<AnimationClip>>& clips = m_currentGraph->GetAnimationDataSet();
        for (uint32_t i = 0; i < clips.size(); i++)
        {
            std::string clip_name = "None(Animation Clip)";
            Ref<AnimationClip> clip = clips[i];
            if (clip)
            {
                clip_name = clip->GetName();
            }
            ImGui::Button(clip_name.c_str());
            clip = ImGuiDragDropResource<AnimationClip>(".trcac");
            if (clip)
            {
                clips[i] = clip;
            }
            
        }
        if (ImGui::Button("Add Animation"))
        {
            clips.push_back(Ref<AnimationClip>());
        }

        ImGui::Text("Parameters: ");
        std::vector<Animation::Parameter>& parameters = m_currentGraph->GetParameters();
        for (uint32_t i = 0; i < parameters.size(); i++)
        {
            Animation::Parameter& param = parameters[i];
            ImGui::PushID((int)i);
            ImGui::InputText("Parameter Name", &param.first);

            char* parameter_type_string[] =
            {
                "Float",
                "Bool",
                "Int"
            };

            if (ImGui::BeginCombo("Parameter Type", parameter_type_string[(int)param.second]))
            {
                for (uint32_t j = 0; j < (uint32_t)Animation::ParameterType::Max; j++)
                {
                    bool selected = (param.second == (Animation::ParameterType)j);
                    if (ImGui::Selectable(parameter_type_string[j], selected))
                    {
                        param.second = (Animation::ParameterType)j;
                    }

                    if (selected)
                    {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }
            ImGui::PopID();
            

        }
        if (ImGui::Button("Add Parameter"))
        {
            parameters.push_back(Animation::Parameter{ "Unnamed Parameter", Animation::ParameterType::Float });
        }
    }

    void AnimationGraphEditor::render_state_machine()
    {
        ImNodes::PushStyleVar(ImNodesStyleVar_LinkLineSegmentsPerLength, 0.0f);
        ImNodes::BeginNode(start_node_id);

        ImNodes::BeginNodeTitleBar();
        ImGui::Text("Entry");
        ImNodes::EndNodeTitleBar();


        ImNodes::PushColorStyle(ImNodesCol_Pin, value_color[(int)Animation::ValueType::Pose]);
        ImNodes::PushColorStyle(ImNodesCol_PinHovered, value_color_hovered[(int)Animation::ValueType::Pose]);
        ImNodes::BeginOutputAttribute(start_output_id);
        ImNodes::EndOutputAttribute();
        ImNodes::PopColorStyle();
        ImNodes::PopColorStyle();

        ImNodes::EndNode();

        for (UUID& id : m_currentNodeChildren)
        {
            Animation::StateNode* state = (Animation::StateNode*)m_currentGraph->GetNode(id);
            int32_t node_index = m_graphNodeIndex[state->GetUUID()];
            ImNodes::BeginNode(node_index);

            std::string& name = STRING_FROM_ID(state->GetName());
            ImNodes::BeginNodeTitleBar();
            ImGui::Text(name.c_str());
            ImNodes::EndNodeTitleBar();


            ImNodes::PushColorStyle(ImNodesCol_Pin, IM_COL32(255, 0, 255, 128));
            ImNodes::PushColorStyle(ImNodesCol_PinHovered, IM_COL32(255, 0, 255, 255));
            ImNodes::BeginInputAttribute(((_link_id + 1) << 24) | node_index);
            ImNodes::EndInputAttribute();
            ImNodes::PopColorStyle();
            ImNodes::PopColorStyle();

            ImNodes::PushColorStyle(ImNodesCol_Pin, IM_COL32(0, 255, 255, 128));
            ImNodes::PushColorStyle(ImNodesCol_PinHovered, IM_COL32(0, 255, 255, 255));
            ImNodes::BeginOutputAttribute(((_link_id + 2) << 24) | node_index);
            ImNodes::EndOutputAttribute();
            ImNodes::PopColorStyle();
            ImNodes::PopColorStyle();

            ImNodes::EndNode();

            ImNodes::PushColorStyle(ImNodesCol_Link, IM_COL32(255, 255, 255, 128));
            ImNodes::PushColorStyle(ImNodesCol_LinkHovered, IM_COL32(255, 255, 255, 200));
            ImNodes::PushColorStyle(ImNodesCol_LinkSelected, IM_COL32(255, 255, 255, 255));
            for (auto& link : m_currentNodeLinks)
            {
                ImNodes::Link(link.id, link.from, link.to);
            }
            ImNodes::PopColorStyle();
            ImNodes::PopColorStyle();
            ImNodes::PopColorStyle();
        }



        ImNodes::PopStyleVar();
    }

    void AnimationGraphEditor::handle_state_machine_actions()
    {
        Animation::StateMachine* state_machine = (Animation::StateMachine*)m_currentNode;
        int num_selected_nodes = ImNodes::NumSelectedNodes();
        if (num_selected_nodes == 1)
        {
            static std::vector<int> selected_nodes;
            selected_nodes.resize(static_cast<size_t>(num_selected_nodes));
            ImNodes::GetSelectedNodes(selected_nodes.data());
            int32_t index = selected_nodes[0];
            if (index != start_node_id && selected_nodes[0] != 0)
            {
                m_selectedNode = m_currentGraph->GetNode(m_graphIndex[index]);
            }
        }

        if (m_selectedNode)
        {
            int32_t node_index = m_graphNodeIndex[m_selectedNode->GetUUID()];
            if (ImNodes::IsNodeHovered(&node_index) && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
            {
                set_current_node(m_selectedNode);
                m_selectedNode = nullptr;
            }
        }

        int num_selected_links = ImNodes::NumSelectedLinks();
        if (num_selected_links == 1)
        {
            static std::vector<int> selected_links;
            selected_links.resize(static_cast<size_t>(num_selected_links));
            ImNodes::GetSelectedLinks(selected_links.data());
            int32_t index = selected_links[0];
            if (index != start_link_id)
            {
                m_selectedNode = m_currentGraph->GetNode(m_graphIndex[index]);

                int32_t link_index = m_graphNodeIndex[m_selectedNode->GetUUID()];
                if (ImNodes::IsLinkHovered(&link_index) && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                {
                    set_current_node(m_selectedNode);
                    m_selectedNode = nullptr;
                }
            }

            if (is_delete_pressed)
            {
                int32_t link_id = index;
                if (link_id == start_link_id)
                {
                    Animation::EntryNode* entry_node = (Animation::EntryNode*)m_currentGraph->GetNode(state_machine->GetEntryNode());
                    Animation::NodeInput& input = entry_node->GetInputs().back();
                    input.node_id = 0;

                    int32_t link_index = -1;
                    auto it = std::find_if(m_currentNodeLinks.begin(), m_currentNodeLinks.end(), [link_id, &link_index](Link& link)
                        {
                            ++link_index;
                            return link.id == link_id;
                        });

                    if (it != m_currentNodeLinks.end())
                    {
                        m_currentNodeLinks[link_index] = m_currentNodeLinks.back();
                        m_currentNodeLinks.pop_back();
                    }


                }
                else
                {
                    int32_t link_index = -1;
                    auto it = std::find_if(m_currentNodeLinks.begin(), m_currentNodeLinks.end(), [link_id, &link_index](Link& link)
                        {
                            ++link_index;
                            return link.id == link_id;
                        });

                    if (it != m_currentNodeLinks.end())
                    {
                        Link& link = m_currentNodeLinks[link_index];
                        UUID transition_id = m_graphIndex[link.id];
                        int32_t mask = ~(~0 << 24);
                        int32_t from_node_index = link.from & mask;
                        Animation::StateNode* from_node = (Animation::StateNode*)m_currentGraph->GetNode(m_graphIndex[from_node_index]);

                        Animation::Node* transition_node = m_currentGraph->GetNode(transition_id);
                        if (m_selectedNode == transition_node)
                        {
                            m_selectedNode = nullptr;
                        }

                        from_node->RemoveTransition(m_currentGraph.get(), transition_id);

                        remove_node(transition_id);
                        m_currentNodeLinks[link_index] = m_currentNodeLinks.back();
                        m_currentNodeLinks.pop_back();
                    }
                }
                ImNodes::ClearLinkSelection(link_id);
            }
        }

        int32_t start_attr, end_attr;
        if (ImNodes::IsLinkCreated(&start_attr, &end_attr))
        {
            int32_t mask = ~(~0 << 24);
            int32_t from_node_index = start_attr & mask;
            int32_t to_node_index = end_attr & mask;
            if (start_attr == start_output_id)
            {
                Animation::EntryNode* entry_node = (Animation::EntryNode*)m_currentGraph->GetNode(state_machine->GetEntryNode());
                if (entry_node->GetInputs().empty())
                {
                    Animation::Node* to_node = m_currentGraph->GetNode(m_graphIndex[to_node_index]);
                    Animation::NodeInput& input = entry_node->GetInputs().emplace_back();
                    input.type = Animation::ValueType::Unknown;
                    input.node_id = to_node->GetUUID();
                    input.value_index = 0;

                    Link link = {};
                    link.from = start_attr;
                    link.to = end_attr;
                    link.id = start_link_id;
                    m_currentNodeLinks.push_back(link);
                    
                }
                else if (entry_node->GetInputs()[0].node_id == 0)
                {
                    Animation::Node* to_node = m_currentGraph->GetNode(m_graphIndex[to_node_index]);
                    Animation::NodeInput& input = entry_node->GetInputs().back();
                    input.type = Animation::ValueType::Unknown;
                    input.node_id = to_node->GetUUID();
                    input.value_index = 0;

                    Link link = {};
                    link.from = start_attr;
                    link.to = end_attr;
                    link.id = start_link_id;
                    m_currentNodeLinks.push_back(link);
                }
            }
            else
            {
                int32_t from_value_index = ((~mask & start_attr) >> 24) - 1;
                int32_t to_index = ((~mask & end_attr) >> 24) - 1;

                Animation::StateNode* from_node = (Animation::StateNode*)m_currentGraph->GetNode(m_graphIndex[from_node_index]);
                Animation::StateNode* to_node = (Animation::StateNode*)m_currentGraph->GetNode(m_graphIndex[to_node_index]);

                UUID transition_id = from_node->CreateTransition(m_currentGraph.get(), to_node->GetUUID());

                int32_t node_index = ++m_graphCurrentIndex;
                m_graphNodeIndex[transition_id] = node_index;
                m_graphIndex[node_index] = transition_id;

                Link link = {};
                link.from = start_attr;
                link.to = end_attr;
                link.id = node_index;
                m_currentNodeLinks.push_back(link);

            }            

        }

        

        

    }

    void AnimationGraphEditor::handle_state_machine_pop_up()
    {
        Animation::StateMachine* state_machine = (Animation::StateMachine*)m_currentNode;
        if (ImGui::BeginPopup("add node"))
        {
            if (ImGui::MenuItem("State"))
            {
                UUID node_id = state_machine->CreateState(m_currentGraph.get(), STR_ID("New State"));
                add_new_node(node_id);

            }

            ImGui::EndPopup();
        }
    }

    void AnimationGraphEditor::add_new_node(UUID node_id)
    {
        int32_t node_index = ++m_graphCurrentIndex;
        m_graphNodeIndex[node_id] = node_index;
        m_graphIndex[node_index] = node_id;
        m_currentNodeChildren.push_back(node_id);
    }

    void AnimationGraphEditor::remove_node(UUID node_id)
    {
        int32_t node_index = m_graphNodeIndex[node_id];
        m_graphNodeIndex.erase(node_id);
        m_graphIndex.erase(node_index);

        auto it = std::find(m_currentNodeChildren.begin(), m_currentNodeChildren.end(), node_id);
        
        if (it != m_currentNodeChildren.end())
        {
            m_currentNodeChildren.erase(it);
        }
    }

}