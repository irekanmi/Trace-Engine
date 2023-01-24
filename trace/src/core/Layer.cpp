#include "pch.h"

#include "Layer.h"
#include "Enums.h"

namespace trace {

	LayerStack::LayerStack()
	{

	}

	LayerStack::~LayerStack()
	{
		Shutdown();
	}

	void LayerStack::PopLayer(Layer* layer)
	{
		auto it = std::find(begin(), end(), layer);
		if (it != end())
		{
			layer->OnDetach();
			m_Layers.erase(it);
			SAFE_DELETE(*it, it);
			m_index--;
		}
	}

	void LayerStack::PopOverLay(Layer* layer)
	{
		
		auto it = eastl::find(begin(), end(), layer);
		
		if (it != end())
		{
			layer->OnDetach();
			m_Layers.erase(it);
			SAFE_DELETE(*it, it);
		}
	}

	void LayerStack::PushLayer(Layer* layer)
	{
		m_Layers.emplace(m_index + begin(), layer);
		layer->OnAttach();
		m_index++;
	}

	void LayerStack::PushOverLay(Layer* layer)
	{
		m_Layers.push_back(layer);
		layer->OnAttach();
	}

	void LayerStack::Shutdown()
	{
		int i = Size() - 1;
		for (; i >= 0; --i)
		{
			Layer* &layer = m_Layers[i];
			if (layer != nullptr)
			{
				layer->OnDetach();
				SAFE_DELETE(layer, layer);
			}
		}


	}
}