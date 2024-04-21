#pragma once

namespace trace {

	class ResourceSystem
	{

	public:

		static bool Init();
		static void ShutDown();
		static bool LoadDefaults();
		static bool LoadDefaults_Runtime();

	private:
	protected:

	};

}

