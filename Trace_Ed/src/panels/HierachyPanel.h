#pragma once

namespace trace {

	class TraceEditor;

	class HierachyPanel
	{

	public:
		HierachyPanel();
		~HierachyPanel(){}

		void Render(float deltaTime);

	private:
		TraceEditor* m_editor;

	protected:
		friend class TraceEditor;
	};

}
