#include "pch.h"

#include "AnimationGraph.h"
#include "animation/AnimationPoseNode.h"
#include "animation/AnimationPose.h"
#include "scene/Scene.h"
#include "scene/Entity.h"
#include "serialize/AnimationsSerializer.h"
#include "external_utils.h"
#include "core/Coretypes.h"
#include "resource/GenericAssetManager.h"
#include "debug/Debugger.h"

bool trace::AnimationGraph::HasAnimationClip(Ref<AnimationClip> clip)
{

    for (AnimationState& state : m_states)
    {
        Ref<AnimationClip> current_state_clip = state.GetAnimationClip();
        if ( current_state_clip && current_state_clip->Compare(clip.get()) )
        {
            return true;
        }
    }

    return false;
}

void trace::AnimationGraph::SetAsRuntime()
{
    for (AnimationState& state : m_states)
    {
        state.SetAsRuntime();
    }

}

void trace::AnimationGraph::Start()
{
    m_started = true;
    m_currrentStateIndex = m_startIndex;

    AnimationState& current_state = m_states[m_currrentStateIndex];

    if (current_state.GetAnimationClip())
    {
        current_state.Play();
    }
}

namespace trace::Animation {
    GraphInstance::GraphInstance()
    {
        m_instanciated = false;
        m_started = false;
    }
    GraphInstance::GraphInstance(GraphInstance& other)
    {
        m_instanciated = false;
        m_started = false;
        m_graph = other.m_graph;
        m_nodesData.clear();
    }
    GraphInstance::GraphInstance(const GraphInstance& other)
    {
        m_instanciated = false;
        m_started = false;
        m_graph = const_cast<Ref<Graph>&>(other.m_graph);
        m_nodesData.clear();
    }
    GraphInstance::~GraphInstance()
    {
        DestroyInstance();
    }
    bool GraphInstance::CreateInstance(Ref<Graph> graph, Scene* scene, UUID entity_id)
    {

        if (!graph)
        {
            TRC_ERROR("Invalid graph handle");
            return false;
        }

        if (!graph->GetSkeleton())
        {
            TRC_ERROR("Invalid skeleton handle");
            return false;
        }

        if (!scene)
        {
            TRC_ERROR("Invalid scene handle");
            return false;
        }

        if (m_instanciated)
        {
            TRC_ERROR("These Instance has already been created, Try to destroy the instance before trying again");
            return false;
        }

        m_graph = graph;
        m_skeletonInstance.SetSkeleton(graph->GetSkeleton());        

        
        std::vector<Parameter>& parameters = graph->GetParameters();
        m_parameterData.resize(parameters.size());

        uint32_t i = 0;
        for (Parameter& param : parameters)
        {
            m_parameterLUT[param.first] = i;
            i++;
        }


        m_skeletonInstance.CreateInstance(graph->GetSkeleton(), scene, entity_id);
        std::unordered_map<UUID, Node*>& nodes = m_graph->GetNodes();
        for (auto& i : nodes)
        {
            i.second->Instanciate(this);
        }

        m_instanciated = true;
        return true;
    }

    void GraphInstance::DestroyInstance()
    {
        if (m_instanciated)
        {
            for (auto& i : m_nodesData)
            {
                uintptr_t _id = (uintptr_t)i.second;
                TRC_DEBUG("Node Data Ptr: {}", _id);
                delete i.second;// TODO: Use custom allocator
            }
            m_nodesData.clear();
            m_instanciated = false;
        }

    }

    void GraphInstance::Start(Scene* scene, UUID id)
    {
        if (!m_instanciated)
        {
            TRC_ERROR("This Graph Instance has not been initialised");
            return;
        }
        m_started = true;
    }

    void GraphInstance::Stop(Scene* scene, UUID id)
    {
        m_started = false;
    }

    

    void GraphInstance::Update(float deltaTime, Scene* scene, UUID id)
    {
        if (!m_started)
        {
            TRC_WARN("Graph has not started can't update graph instance. Function: {}", __FUNCTION__);
            return;
        }

        PoseNode* root_node = m_graph->GetRootNode();

        root_node->Update(this, deltaTime);
        PoseNodeResult* final_pose = root_node->GetFinalPose(this);

        //TRC_ASSERT(final_pose != nullptr, "Funtion: {}", __FUNCTION__);

        if (final_pose)
        {
            final_pose->pose_data.SetEntityLocalPose();
            Transform& root_motion_delta = final_pose->pose_data.GetRootMotionDelta();
            Entity entity = scene->GetEntity(id);
            Transform& pose = entity.GetComponent<TransformComponent>()._transform;
            Transform::ApplyRootMotion(pose, root_motion_delta);

           
        }
        


    }


    ParameterData* GraphInstance::GetParameterData(Parameter& param)
    {
        int32_t index = m_parameterLUT[param.first];

        return &m_parameterData[index];
    }

    void GraphInstance::set_parameter_data(const std::string& param_name, void* data, uint32_t size)
    {
        auto it = m_parameterLUT.find(param_name);
        if (it == m_parameterLUT.end())
        {
            TRC_WARN("'{}' is not a parameter", param_name);
            return;
        }

        ParameterData& param_data = m_parameterData[it->second];
        
        memcpy(param_data.data, data, size);

        //.. Check for Transitions
    }

    bool Graph::Create()
    {
        m_rootNode = CreateNode<FinalOutputNode>();
        FinalOutputNode* root_node = (FinalOutputNode*)GetNode(m_rootNode);
        root_node->Init(this);

        return true;
    }

    void Graph::Destroy()
    {
        for (auto& node : m_nodes)
        {
            delete node.second;// TODO: Use custom allocator
        }

    }

    void Graph::CreateParameter(const std::string& param_name, ParameterType type)
    {
        Parameter param;
        param.first = param_name;
        param.second = type;
        m_parameters.emplace_back(param);
    }

    void Graph::SetSkeleton(Ref<Skeleton> skeleton)
    {
        if (skeleton)
        {
            m_skeleton = skeleton;
        }
    }

    Node* Graph::GetNode(UUID node_id)
    {
        auto it = m_nodes.find(node_id);
        
        if (it != m_nodes.end())
        {
            return m_nodes[node_id];
        }

        TRC_ASSERT(false, "Ensure to Get valid nodes, Function: {}", __FUNCTION__);
        return nullptr;
    }

    void Graph::DestroyNode(UUID node_id)
    {
        Node* node = GetNode(node_id);
        if (node)
        {
            node->Destroy(this);
            m_nodes.erase(node_id);
            delete node;//TODO: Use custom allocator
        }
    }

    void Graph::DestroyNodeWithInputs(UUID node_id)
    {
        Node* node = GetNode(node_id);
        if (node)
        {
            node->Destroy(this);
            for (NodeInput& input : node->GetInputs())
            {
                if (input.node_id != 0)
                {
                    DestroyNode(input.node_id);
                }
            }
            m_nodes.erase(node_id);
            delete node;//TODO: Use custom allocator
        }
    }

    void Graph::AddAnimationClip(Ref<AnimationClip> clip)
    {
        m_animationDataSet.push_back(clip);
    }

    Ref<Graph> Graph::Deserialize(UUID id)
    {
        Ref<Graph> result;

        if (AppSettings::is_editor)
        {
            std::string file_path = GetPathFromUUID(id).string();
            if (!file_path.empty())
            {
                result = AnimationsSerializer::DeserializeAnimGraph(file_path);
            }
        }
        else
        {
            return GenericAssetManager::get_instance()->Load_Runtime<Graph>(id);
        }

        return result;
    }

    Ref<Graph> Graph::Deserialize(DataStream* stream)
    {
        return AnimationsSerializer::DeserializeAnimGraph(stream);
    }


}