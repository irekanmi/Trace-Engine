#pragma once

#include "animation/AnimationGraph.h"
#include "resource/Ref.h"
#include "animation/AnimationNode.h"
#include "animation/AnimationPoseNode.h"

namespace trace {

	class Event;

	class AnimationGraphEditor
	{

	public:
		AnimationGraphEditor() {};
		~AnimationGraphEditor() {};

		bool Init();
		void Shutdown();

		void Update(float deltaTime);
		void Render(float deltaTime, const std::string& window_name);
		void OnEvent(Event* p_event);
		void HandleKeyReleased(Event* p_event);
		void HandleKeyPressed(Event* p_event);

		Ref<Animation::Graph> GetCurrentGraph() { return m_currentGraph; }

		void SetAnimationGraph(Ref<Animation::Graph> graph);
		void SetAnimationGraph(std::string path);

	private:
		void generate_current_node_children();
		void generate_current_node_links();
		void generate_graph_node_id();
		void add_child_node(Animation::Node* current_node);
		void add_child_links(Animation::Node* current_node);
		void set_current_node(Animation::Node* current_node);
		void free_current_node();
		void delete_node(UUID node_id);
		int32_t AnimationGraphEditor::paramters_drop_down(int32_t id);
		int32_t AnimationGraphEditor::animations_drop_down(int32_t id);
		void AnimationGraphEditor::render_graph_data();
		void render_state_machine();
		void handle_state_machine_actions();
		void handle_state_machine_pop_up();
		void add_new_node(UUID node_id);
		void remove_node(UUID node_id);


		struct Link
		{
			int id = -1;
			int from = -1;
			int to = -1;
			Animation::ValueType value_type;
		};

		Ref<Animation::Graph> m_currentGraph;
		bool new_graph = true;
		Animation::Node* m_currentNode = nullptr;
		Animation::Node* m_selectedNode = nullptr;
		std::vector<Link> m_currentNodeLinks;
		std::vector<UUID> m_currentNodeChildren;
		std::unordered_map<UUID, int32_t> m_graphNodeIndex;
		std::unordered_map<int32_t, UUID> m_graphIndex;
		int32_t m_graphCurrentIndex;
		std::unordered_map<uint64_t, std::function<void(Animation::Node* node)>> type_node_render;
		std::unordered_map<uint64_t, std::function<void(Animation::Node* node)>> node_selected_render;
		std::vector<Animation::Node*> m_currentGraphNodePath;


	protected:

	};

}
