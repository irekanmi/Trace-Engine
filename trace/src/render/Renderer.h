#pragma once

#include "core/Core.h"
#include "core/Object.h"
#include "GPUtypes.h"
#include "GBuffer.h"


namespace trace {

	class GContext;

	class TRACE_API Renderer : public Object
	{

	public:
		Renderer();
		~Renderer();

		bool Init(RenderAPI api);
		void BeginScene();
		void EndScene();
		void Draw(GBuffer* buffer, BufferUsage usage);

		static Renderer* s_instance;
		static Renderer* get_instance();
		static RenderAPI get_api();
	private:
		GContext* m_context;

	protected:

	};

}
