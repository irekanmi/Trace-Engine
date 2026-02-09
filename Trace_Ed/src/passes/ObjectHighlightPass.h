#pragma once

#include "core/Core.h"
#include "core/Enums.h"
#include "render/render_graph/RenderPass.h"
#include "resource/Ref.h"
#include "scene/Entity.h"
#include "scene/Scene.h"

#include <vector>
#include <map>

namespace trace {

	class GPipeline;
	class Renderer;

	class TRACE_API ObjectHighlightPass : public RenderPass
	{

	public:
		ObjectHighlightPass() {}
		~ObjectHighlightPass() {}

		virtual void Init(Renderer* renderer) override;
		virtual void Setup(RenderGraph* render_graph, RenderPassPacket& pass_inputs) override;
		virtual void Setup(RenderGraph* render_graph, RGBlackBoard& black_board, int32_t render_graph_index, int32_t draw_index = 0) override;
		virtual void ShutDown() override;

	public:
		void SetHighlightedEntities(int32_t render_graph_index, std::vector<Entity>* entites);


	private:
		Ref<GPipeline> m_pipeline;
		Ref<GPipeline> imageDialationPipeline;
		Ref<GPipeline> finalBlendPipeline;
		GRenderPass imageDialationPass;
		GRenderPass finalBlendPass;
		std::map<uint32_t, std::vector<Entity>*> object_highlight;

	protected:


	};

}

