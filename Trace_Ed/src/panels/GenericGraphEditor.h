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
		void Render(float deltaTime);
		void OnEvent(Event* p_event);
		void HandleKeyReleased(Event* p_event);
		void HandleKeyPressed(Event* p_event);

		GenericGraph* GetCurrentGraph() { return m_currentGraph; }

		void* GetUserData() { return user_data; }
		void SetUserData(void* ptr) { user_data = ptr; }

		void SetGraph(GenericGraph* graph, const std::string& graph_name);
		void SetCurrentNode(GenericNode* current_node);

		void add_new_node(UUID node_id);

	public:
		void generate_current_node_children();
		void generate_current_node_links();
		void generate_graph_node_id();
		void add_child_node(GenericNode* current_node);
		void add_child_links(GenericNode* current_node);
		void free_current_node();
		void delete_node(UUID node_id);
		int32_t paramters_drop_down(int32_t id);
		void render_graph_data();
		void remove_node(UUID node_id);

	private:

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
		std::vector<GenericNode*> m_currentGraphNodePath;
		std::string m_graphName;
		void* user_data = nullptr;


	protected:

	};

}
