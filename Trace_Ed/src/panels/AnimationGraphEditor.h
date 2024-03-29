#pragma once

#include "animation/AnimationGraph.h"
#include "animation/AnimationState.h"
#include "resource/Ref.h"

namespace trace {

	class AnimationGraphEditor
	{

	public:
		AnimationGraphEditor() {};
		~AnimationGraphEditor() {};

		bool Init();
		void Shutdown();

		void Update(float deltaTime);
		void Render(float deltaTime);

		void SetAnimationGraph(Ref<AnimationGraph> graph);
		void SetAnimationGraph(std::string path);

	private:
		void create_new_state();

		struct Link
		{
			int id = -1;
			int from = -1;
			int to = -1;
		};

		Ref<AnimationGraph> m_currentGraph;
		AnimationState* m_selectedState = nullptr;
		std::vector<Link> m_links;
		bool new_graph = true;

	protected:

	};

}
