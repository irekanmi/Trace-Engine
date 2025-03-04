#pragma once

#include "glm/glm.hpp"
#include "render/GPipeline.h"
#include "resource/Ref.h"
#include "core/Coretypes.h"

#include <unordered_map>

namespace trace {

	class Debugger
	{

	public:
		struct DebugRenderData
		{
			std::vector<glm::vec4> positions;
			uint32_t vert_count = 0;
		};
		

		bool Init();
		void Shutdown();
		bool IsInitialized() { return m_initialized; }

		// Strings -----------------------------------
		void SetString(StringID string_id, const std::string& string_data);
		std::string& GetString(StringID string_id);
		// -------------------------------------------

		// Rendering ---------------------------------
		void AddDebugLine(glm::vec3 point_0, glm::vec3 point_1, glm::mat4 transform, uint32_t color = 1);
		void AddDebugLine(glm::vec3 from, glm::vec3 to, uint32_t color = 1);
		void DrawDebugCapsule(float radius, float height, glm::mat4 transform, uint32_t color = 1);
		void DrawDebugCylinder(float radius, float height, glm::mat4 transform, uint32_t color = 1);
		void DrawDebugCircle(float radius, uint32_t steps, glm::mat4 transform, uint32_t color = 1, int32_t render_graph_index = 0);
		void DrawDebugSemiCircle(float radius, uint32_t steps, glm::mat4 transform, uint32_t color = 1, int32_t render_graph_index = 0);
		void DrawDebugSphere(float radius, uint32_t steps, glm::mat4 transform, uint32_t color = 1, int32_t render_graph_index = 0);
		void DrawDebugHemiSphere(float radius, uint32_t steps, glm::mat4 transform, uint32_t color = 1, int32_t render_graph_index = 0);

		DebugRenderData& GetRenderData() { return m_renderData; }

		// ----------------------------------------------

		static Debugger* get_instance();
	private:
		bool m_initialized = false;
		DebugRenderData m_renderData;
		std::unordered_map<StringID, std::string> m_stringData;


	protected:


	};

}
