#pragma once

#include "Core.h"

#include "events\Events.h"

namespace trace {

	class TRACE_API Layer
	{

	public:
		Layer(const char* name = "Layer"):m_name(name){}
		virtual ~Layer() {};

		virtual void OnAttach() = 0;
		virtual void OnDetach() = 0;
		virtual void Update(float deltaTime) = 0;
		virtual void OnEvent(Event* p_event) = 0;
	private:
		const char* m_name;
	protected:

	};

	class TRACE_API LayerStack
	{

	public:
		LayerStack();
		~LayerStack();

		void PushLayer(Layer* layer);
		void PushOverLay(Layer* layer);
		void PopLayer(Layer* layer);
		void PopOverLay(Layer* layer);
		size_t Size() { return m_layers.size(); }

		const std::vector<Layer*>& GetLayers() { return m_layers; }

		void Shutdown();

		std::vector<Layer*>::iterator begin() { return m_layers.begin(); }
		std::vector<Layer*>::iterator end() { return m_layers.end(); }

	private:
		std::vector<Layer*> m_layers;
		unsigned int m_index = 0;
	protected:

	};

}
