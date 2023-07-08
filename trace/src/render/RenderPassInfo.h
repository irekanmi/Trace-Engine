#pragma once

#include "core/Enums.h"
#include "core/Core.h"
#include "core/pch.h"
#include "Graphics.h"
#include "glm/glm.hpp"

namespace trace {

	struct AttachmentInfo
	{
		TextureFormat initial_format = TextureFormat::UNKNOWN;
		TextureFormat final_format = TextureFormat::UNKNOWN;
		AttachmentLoadOp load_operation = AttachmentLoadOp::OP_NONE;
		AttachmentStoreOp store_operation = AttachmentStoreOp::OP_NONE;
		uint32_t attachmant_index = uint32_t(-1);
		Format attachment_format = Format::NONE;
		bool is_depth = false;
	};

	struct SubPassDescription
	{
		uint32_t attachment_count = 0;
		AttachmentInfo* attachments = {};
	};

	struct RenderPassDescription
	{
		SubPassDescription subpass;
		glm::vec4 clear_color = {};
		glm::vec4 render_area = {};
		float depth_value = 0.0f;
		uint32_t stencil_value = 0;
	};

}
