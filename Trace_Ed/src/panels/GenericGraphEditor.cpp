
#include "GenericGraphEditor.h"

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

    static uint32_t value_color[(int)GenericValueType::Max] =
    {
        IM_COL32(255, 204, 153, 128),
        IM_COL32(51, 102, 255, 128),
        IM_COL32(51, 204, 204, 128),
        IM_COL32(153, 204, 0, 128),
        IM_COL32(255, 204, 0, 128),
    };

    static uint32_t value_color_hovered[(int)GenericValueType::Max] =
    {
        IM_COL32(255, 204, 153, 255),
        IM_COL32(51, 102, 255, 255),
        IM_COL32(51, 204, 204, 255),
        IM_COL32(153, 204, 0, 255),
        IM_COL32(255, 204, 0, 255),
    };

    static std::unordered_map<uint64_t, std::string> type_names =
    {
        {0, "__Invalid_Node__"}
    };

    bool GenericGraphEditor::Init()
    {
        

        return true;
    }

    void GenericGraphEditor::Shutdown()
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
        }
    }

    void GenericGraphEditor::Update(float deltaTime)
    {
    }

    void GenericGraphEditor::OnEvent(Event* p_event)
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

    void GenericGraphEditor::HandleKeyReleased(Event* p_event)
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

    void GenericGraphEditor::HandleKeyPressed(Event* p_event)
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

    void GenericGraphEditor::Render(float deltaTime, const std::string& window_name)
    {
        TraceEditor* editor = TraceEditor::get_instance();

        ImGui::Begin(window_name.c_str());
        ImGui::Columns(2);

        if (m_currentGraph)
        {
            std::string& graph_name = m_graphName;


            ImGui::Button(graph_name.c_str());

            if (ImGui::TreeNode("Graph Data"))
            {
                render_graph_data();
                ImGui::TreePop();
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

        ImGui::NextColumn();
        if (m_currentGraph)
        {
            for (uint32_t i = 0; i < m_currentGraphNodePath.size(); i++)
            {

                GenericNode* node = m_currentGraphNodePath[i];
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

            auto it = type_node_render.find(m_currentNode->GetTypeID());
            if (it != type_node_render.end())
            {
                it->second(m_currentNode);
            }

            for (UUID& id : m_currentNodeChildren)
            {
                GenericNode* node = m_currentGraph->GetNode(id);
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

            if (ImGui::BeginPopup("add node"))
            {
                if (ImGui::MenuItem("__Test(Button_t0)__"))
                {
                    UUID node_id = 0;// m_currentGraph->CreateNode<GenericNode>();
                    add_new_node(node_id);
                }
                ImGui::EndPopup();
            }
        }


        ImNodes::MiniMap();

        is_window_focused = ImGui::IsWindowFocused() && ImGui::IsWindowHovered();
        ImNodes::EndNodeEditor();

        if (m_currentGraph)
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
                        GenericNode* node = m_currentGraph->GetNode(node_id);
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

                GenericNode* from_node = m_currentGraph->GetNode(m_graphIndex[from_node_index]);
                GenericNode* to_node = m_currentGraph->GetNode(m_graphIndex[to_node_index]);

                GenericNodeInput& input = to_node->GetInputs()[to_index];
                GenericNodeOutput& output = from_node->GetOutputs()[from_value_index];
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

                        GenericNode* from_node = m_currentGraph->GetNode(m_graphIndex[from_node_index]);
                        GenericNode* to_node = m_currentGraph->GetNode(m_graphIndex[to_node_index]);

                        GenericNodeInput& input = to_node->GetInputs()[to_index];
                        input.node_id = 0;
                        input.value_index = -1;

                        m_currentNodeLinks[link_id] = m_currentNodeLinks.back();
                        m_currentNodeLinks.pop_back();
                        ImNodes::ClearLinkSelection(_id);
                    }

                }
            }
        };



        ImGui::Columns(1);




        ImGui::End();


    }

    void GenericGraphEditor::SetAnimationGraph(GenericGraph* graph, const std::string& graph_name)
    {
        if (!graph)
        {
            return;
        }

        TraceEditor* editor = TraceEditor::get_instance();
        if (m_currentGraph)
        {
            free_current_node();

            std::string db_path = editor->GetCurrentProject()->GetProjectCurrentDirectory() + "/InternalAssetsDB/" + m_graphName + ".gnrbin";
            FileStream stream(db_path, FileMode::WRITE);
            Reflection::Serialize(m_graphCurrentIndex, &stream, nullptr, Reflection::SerializationFormat::BINARY);
            Reflection::Serialize(m_graphIndex, &stream, nullptr, Reflection::SerializationFormat::BINARY);
            Reflection::Serialize(m_graphNodeIndex, &stream, nullptr, Reflection::SerializationFormat::BINARY);
        }

        m_currentGraph = graph;
        m_graphName = graph_name;
        new_graph = true;
        m_graphCurrentIndex = 0;
        m_graphIndex.clear();
        m_graphNodeIndex.clear();


        std::string db_path = editor->GetCurrentProject()->GetProjectCurrentDirectory() + "/InternalAssetsDB/" + m_graphName + ".gnrbin";
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


        GenericNode* root = m_currentGraph->GetNode(m_currentGraph->GetRootNodeUUID());
        TRC_ASSERT(root, "Ensure graph has a root");
        set_current_node(root);



    }

    void GenericGraphEditor::generate_current_node_children()
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
        GenericNode* current_node = m_currentNode;
        for (uint32_t i = 0; i < current_node->GetInputs().size(); i++)
        {
            GenericNodeInput& input = current_node->GetInputs()[i];
            if (input.node_id != 0)
            {
                m_currentNodeChildren.push_back(input.node_id);
                add_child_node(m_currentGraph->GetNode(input.node_id));
            }
        }


    }

    void GenericGraphEditor::generate_current_node_links()
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
        GenericNode* current_node = m_currentNode;
        for (uint32_t i = 0; i < current_node->GetInputs().size(); i++)
        {
            GenericNodeInput& input = current_node->GetInputs()[i];
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

    void GenericGraphEditor::generate_graph_node_id()
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

    void GenericGraphEditor::add_child_node(GenericNode* current_node)
    {
        TRC_ASSERT(current_node != nullptr, "Invalid node pointer Function: {}", __FUNCTION__);
        for (uint32_t i = 0; i < current_node->GetInputs().size(); i++)
        {
            GenericNodeInput& input = current_node->GetInputs()[i];
            if (input.node_id != 0)
            {
                m_currentNodeChildren.push_back(input.node_id);
                add_child_node(m_currentGraph->GetNode(input.node_id));
            }
        }
    }

    void GenericGraphEditor::add_child_links(GenericNode* current_node)
    {
        TRC_ASSERT(current_node != nullptr, "Invalid node pointer Function: {}", __FUNCTION__);
        for (uint32_t i = 0; i < current_node->GetInputs().size(); i++)
        {
            GenericNodeInput& input = current_node->GetInputs()[i];
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

    void GenericGraphEditor::set_current_node(GenericNode* current_node)
    {
        if (m_currentNode)
        {
            m_currentGraphNodePath.push_back(m_currentNode);
            free_current_node();

        }

        m_currentNode = current_node;

        m_currentNodeChildren.clear();
        TraceEditor* editor = TraceEditor::get_instance();
        std::string node_db_path = editor->GetCurrentProject()->GetProjectCurrentDirectory() + "/InternalAssetsDB/" + m_graphName + std::to_string(uint64_t(m_currentNode->GetUUID())) + ".gnrnodes";
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


        std::string db_path = editor->GetCurrentProject()->GetProjectCurrentDirectory() + "/InternalAssetsDB/" + m_graphName + std::to_string(uint64_t(m_currentNode->GetUUID())) + ".ini";
        if (std::filesystem::exists(db_path))
        {
            ImNodes::LoadCurrentEditorStateFromIniFile(db_path.c_str());
        }

    }

    void GenericGraphEditor::free_current_node()
    {

        TraceEditor* editor = TraceEditor::get_instance();

        std::string node_db_path = editor->GetCurrentProject()->GetProjectCurrentDirectory() + "/InternalAssetsDB/" + m_graphName + std::to_string(uint64_t(m_currentNode->GetUUID())) + ".gnrnodes";
        FileStream stream(node_db_path, FileMode::WRITE);
        Reflection::SerializeContainer(m_currentNodeChildren, &stream, nullptr, Reflection::SerializationFormat::BINARY);

        std::string db_path = editor->GetCurrentProject()->GetProjectCurrentDirectory() + "/InternalAssetsDB/" + m_graphName + std::to_string(uint64_t(m_currentNode->GetUUID())) + ".ini";
        ImNodes::SaveCurrentEditorStateToIniFile(db_path.c_str());

        m_currentNodeChildren.clear();
        m_currentNodeChildren.clear();
        m_currentNode = nullptr;
        m_selectedNode = nullptr;
    }

    void GenericGraphEditor::delete_node(UUID node_id)
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
                GenericNode* to_node = m_currentGraph->GetNode(to_id);
                for (GenericNodeInput& input : to_node->GetInputs())
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

    int32_t GenericGraphEditor::paramters_drop_down(int32_t id)
    {
        int32_t result = -1;

        if (ImGui::BeginPopup("Parameters Drop Down"))
        {
            for (int32_t i = 0; i < m_currentGraph->GetParameters().size(); i++)
            {
                GenericParameter& param = m_currentGraph->GetParameters()[i];
                if (ImGui::MenuItem(param.name.c_str()))
                {
                    result = i;
                    break;
                }
            }
            ImGui::EndPopup();
        }

        return result;
    }

    void GenericGraphEditor::render_graph_data()
    {
        //TODO: Generic Graph Data
        // Graph_Type_Render(m_currentNode->GetTypeID());

        ImGui::Text("Parameters: ");
        std::vector<GenericParameter>& parameters = m_currentGraph->GetParameters();
        for (uint32_t i = 0; i < parameters.size(); i++)
        {
            GenericParameter& param = parameters[i];
            ImGui::PushID((int)i);
            ImGui::InputText("Parameter Name", &param.name);

            char* parameter_type_string[] =
            {
                "Float",
                "Bool",
                "Int"
            };

            if (ImGui::BeginCombo("Parameter Type", parameter_type_string[(int)param.type]))
            {
                for (uint32_t j = 0; j < (uint32_t)GenericValueType::Max; j++)
                {
                    bool selected = (param.type == (GenericValueType)j);
                    if (ImGui::Selectable(parameter_type_string[j], selected))
                    {
                        param.type = (GenericValueType)j;
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
            parameters.push_back(GenericParameter{ "Unnamed Parameter", GenericValueType::Bool });
        }
    }

    void GenericGraphEditor::add_new_node(UUID node_id)
    {
        int32_t node_index = ++m_graphCurrentIndex;
        m_graphNodeIndex[node_id] = node_index;
        m_graphIndex[node_index] = node_id;
        m_currentNodeChildren.push_back(node_id);
    }

    void GenericGraphEditor::remove_node(UUID node_id)
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