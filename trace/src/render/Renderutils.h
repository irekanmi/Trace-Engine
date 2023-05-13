#pragma once

#include "Graphics.h"

namespace trace {

	class GHandle;
	class GDevice;
	class GContext;
	class GBuffer;
	struct BufferInfo;

	void AutoFillPipelineDesc(PipelineStateDesc& desc, bool input_layout = true, bool raterizer_state = true, bool depth_sten_state = true, bool color_blend_state = true, bool view_port = true, bool scissor = true, bool render_pass = true, bool primitive_topology = true);
	bool operator ==(ShaderResourceBinding lhs, ShaderResourceBinding rhs);


	typedef bool (*__CreateContext)(GContext*);
	typedef bool (*__DestroyContext)(GContext*);

	typedef bool (*__CreateDevice)(GDevice*);
	typedef bool (*__DestroyDevice)(GDevice*);

	typedef bool (*__CreateBuffer)(GBuffer*, BufferInfo*);
	typedef bool (*__ValidateHandle)(GHandle*);

	class TRACE_API RenderFuncLoader
	{
	public:
		static bool LoadVulkanRenderFunctions();
	private:
	protected:
	};

	class TRACE_API RenderFunc
	{

	public:
		static bool CreateContext(GContext* context);
		static bool DestroyContext(GContext* context);

		static bool CreateDevice(GDevice* device);
		static bool DestroyDevice(GDevice* device);

		static bool CreateBuffer(GBuffer* buffer, BufferInfo* buffer_info);
		static bool ValidateHandle(GHandle* handle);

	private:
		static __CreateContext _createContext;
		static __DestroyContext _destroyContext;

		static __CreateDevice _createDevice;
		static __DestroyDevice _destroyDevice;

		static __CreateBuffer _createBuffer;
		static __ValidateHandle _validateHandle;

	protected:
		friend RenderFuncLoader;
	};

}
