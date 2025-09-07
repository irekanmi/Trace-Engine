#pragma once

#include "node_system/GenericGraph.h"

namespace trace {

	class Event;

	class GenericGraphEditor
	{

	public:
		GenericGraphEditor() {};
		~GenericGraphEditor() {};

		bool Init();
		void Shutdown();

		void Update(float deltaTime);
		void Render(float deltaTime, const std::string& window_name);
		void OnEvent(Event* p_event);
		void HandleKeyReleased(Event* p_event);
		void HandleKeyPressed(Event* p_event);

		GenericGraph* GetCurrentGraph() { return m_currentGraph; }

		void SetAnimationGraph(GenericGraph* graph, const std::string& graph_name);

	private:
		void generate_current_node_children();
		void generate_current_node_links();
		void generate_graph_node_id();
		void add_child_node(GenericNode* current_node);
		void add_child_links(GenericNode* current_node);
		void set_current_node(GenericNode* current_node);
		void free_current_node();
		void delete_node(UUID node_id);
		int32_t GenericGraphEditor::paramters_drop_down(int32_t id);
		void GenericGraphEditor::render_graph_data();
		void add_new_node(UUID node_id);
		void remove_node(UUID node_id);


		struct Link
		{
			int id = -1;
			int from = -1;
			int to = -1;
			GenericValueType value_type;
		};

		GenericGraph* m_currentGraph = nullptr;
		bool new_graph = true;
		GenericNode* m_currentNode = nullptr;
		GenericNode* m_selectedNode = nullptr;
		std::vector<Link> m_currentNodeLinks;
		std::vector<UUID> m_currentNodeChildren;
		std::unordered_map<UUID, int32_t> m_graphNodeIndex;
		std::unordered_map<int32_t, UUID> m_graphIndex;
		int32_t m_graphCurrentIndex;
		std::unordered_map<uint64_t, std::function<void(GenericNode* node)>> type_node_render;
		std::unordered_map<uint64_t, std::function<void(GenericNode* node)>> node_selected_render;
		std::vector<GenericNode*> m_currentGraphNodePath;
		std::string m_graphName;


	protected:

	};

}
