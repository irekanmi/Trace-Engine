#pragma once

#include "node_system/GenericGraph.h"

namespace trace {

	class Event;

	class GenericGraphEditor
	{

	public:
		struct Link
		{
			int id = -1;
			int from = -1;
			int to = -1;
			GenericValueType value_type;
		};

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
		void* GetCurrentContext() { return current_context; }

		void SetGraph(GenericGraph* graph, const std::string& graph_name);
		void SetCurrentNode(GenericNode* current_node);

		void add_new_node(UUID node_id);
		void add_new_node_not_child(UUID node_id);
		int32_t get_node_index(UUID node_id);
		int get_start_node_id();
		int get_start_output_id();
		std::vector<UUID>& get_current_node_children();
		int32_t& get_graph_node_index(UUID uuid);
		int get_link_id();
		std::vector<Link>& get_current_node_links();
		GenericNode*& get_selected_node() { return m_selectedNode; }
		UUID get_graph_index(int32_t index);
		int get_start_link_id();
		bool is_delete();
		void set_graph_index(int32_t index, UUID value);
		void set_graph_node_index(UUID uuid, int32_t value);
		int32_t& get_graph_current_index() { return m_graphCurrentIndex; }

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

	public:
		bool can_check_child_node = true;

	private:

		

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
		void* current_context = nullptr;


	protected:

	};

}
