#pragma once


namespace trace {

	class ScriptEngine
	{

	public:
		ScriptEngine();
		~ScriptEngine();

		bool Init();
		void Shutdown();

		static ScriptEngine* get_instance();
	private:
		static ScriptEngine* s_instance;

	protected:

	};

}
