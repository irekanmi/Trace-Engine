#pragma once

#include "reflection/TypeRegistry.h"
#include "scene/Entity.h"
#include "scene/Scene.h"
#include "render/Transform.h"
#include "render/Camera.h"
#include "render/Graphics.h"

namespace trace {

	BEGIN_REGISTER_CLASS(Transform)
		REGISTER_TYPE(Transform);
		REGISTER_MEMBER(Transform, m_rotation);
		REGISTER_MEMBER(Transform, m_position);
		REGISTER_MEMBER(Transform, m_scale);
	END_REGISTER_CLASS;

	REGISTER_TYPE(AddressMode);

	REGISTER_TYPE(FilterMode);

	REGISTER_TYPE(UsageFlag);

	REGISTER_TYPE(BindFlag);

	REGISTER_TYPE(RenderAPI);

	REGISTER_TYPE(ShaderStage);

	REGISTER_TYPE(CullMode);

	REGISTER_TYPE(Format);

	REGISTER_TYPE(InputClassification);

	REGISTER_TYPE(TextureFormat);

	REGISTER_TYPE(ImageType);

	REGISTER_TYPE(AttachmentLoadOp);

	REGISTER_TYPE(AttachmentStoreOp);

	REGISTER_TYPE(AttachmentType);

	REGISTER_TYPE(PRIMITIVETOPOLOGY);

	REGISTER_TYPE(ShaderData);

	REGISTER_TYPE(ShaderResourceType);

	REGISTER_TYPE(ShaderResourceStage);

	REGISTER_TYPE(RENDERPASS);

	REGISTER_TYPE(GPU_QUEUE);

	REGISTER_TYPE(ShaderDataDef);

	REGISTER_TYPE(BlendFactor);

	REGISTER_TYPE(BlendOp);

	REGISTER_TYPE(FrameSettingsBit);

	REGISTER_TYPE(LightType);

	REGISTER_TYPE(PipelineType);

	BEGIN_REGISTER_CLASS(Element)
		REGISTER_TYPE(Element);
		REGISTER_MEMBER(Element, index);
		REGISTER_MEMBER(Element, offset );
		REGISTER_MEMBER(Element, stride );
		REGISTER_MEMBER(Element, format );
	END_REGISTER_CLASS;
	
	BEGIN_REGISTER_CLASS(InputLayout)
		REGISTER_TYPE(InputLayout);
		REGISTER_MEMBER(InputLayout, stride);
		REGISTER_MEMBER(InputLayout, input_class );
		REGISTER_MEMBER(InputLayout, elements );
	END_REGISTER_CLASS;

	BEGIN_REGISTER_CLASS(RasterizerState)
		REGISTER_TYPE(RasterizerState);
		REGISTER_MEMBER(RasterizerState, cull_mode);
		REGISTER_MEMBER(RasterizerState, fill_mode );
	END_REGISTER_CLASS;	

	BEGIN_REGISTER_CLASS(DepthStencilState)
		REGISTER_TYPE(DepthStencilState);
		REGISTER_MEMBER(DepthStencilState, depth_test_enable);
		REGISTER_MEMBER(DepthStencilState, depth_write_enable);
		REGISTER_MEMBER(DepthStencilState, stencil_test_enable);
		REGISTER_MEMBER(DepthStencilState, minDepth);
		REGISTER_MEMBER(DepthStencilState, maxDepth);
	END_REGISTER_CLASS;	

	BEGIN_REGISTER_CLASS(FrameInfo)
		REGISTER_TYPE(FrameInfo);
		REGISTER_MEMBER(FrameInfo, src_color);
		REGISTER_MEMBER(FrameInfo, dst_color);
		REGISTER_MEMBER(FrameInfo, color_op);
		REGISTER_MEMBER(FrameInfo, src_alpha);
		REGISTER_MEMBER(FrameInfo, dst_alpha);
		REGISTER_MEMBER(FrameInfo, alpha_op);
	END_REGISTER_CLASS;

	BEGIN_REGISTER_CLASS(ColorBlendState)
		REGISTER_TYPE(ColorBlendState);
		REGISTER_MEMBER(ColorBlendState, alpha_to_blend_coverage);
		REGISTER_MEMBER(ColorBlendState, render_targets);
		REGISTER_MEMBER(ColorBlendState, num_render_target);
	END_REGISTER_CLASS;

	BEGIN_REGISTER_CLASS(UniformMetaData)
		REGISTER_TYPE(UniformMetaData);
		REGISTER_MEMBER(UniformMetaData, _id);
		REGISTER_MEMBER(UniformMetaData, _offset);
		REGISTER_MEMBER(UniformMetaData, _size);
		REGISTER_MEMBER(UniformMetaData, _slot);
		REGISTER_MEMBER(UniformMetaData, _index);
		REGISTER_MEMBER(UniformMetaData, _count);
		REGISTER_MEMBER(UniformMetaData, meta_id);
		REGISTER_MEMBER(UniformMetaData, _struct_index);
		REGISTER_MEMBER(UniformMetaData, _resource_type);
		REGISTER_MEMBER(UniformMetaData, _shader_stage);
		REGISTER_MEMBER(UniformMetaData, data_type);
	END_REGISTER_CLASS;

	BEGIN_REGISTER_CLASS(InternalMaterialData)
		REGISTER_TYPE(InternalMaterialData);
		REGISTER_MEMBER(InternalMaterialData, hash);
		REGISTER_MEMBER(InternalMaterialData, offset);
		REGISTER_MEMBER(InternalMaterialData, type);
	END_REGISTER_CLASS;

	BEGIN_REGISTER_CLASS(Light)
		REGISTER_TYPE(Light);
		REGISTER_MEMBER(Light, position);
		REGISTER_MEMBER(Light, direction );
		REGISTER_MEMBER(Light, color );
		REGISTER_MEMBER(Light, params1 );
		REGISTER_MEMBER(Light, params2);
	END_REGISTER_CLASS;

	REGISTER_TYPE(CameraType);
	REGISTER_CONTAINER(std::vector, Transform);

	BEGIN_REGISTER_CLASS(Camera)
		REGISTER_TYPE(Camera);
		REGISTER_MEMBER(Camera, m_position);
		REGISTER_MEMBER(Camera, m_lookDirection);
		REGISTER_MEMBER(Camera, m_upDirection);
		REGISTER_MEMBER(Camera, m_fov);
		REGISTER_MEMBER(Camera, m_zNear);
		REGISTER_MEMBER(Camera, m_zFar);
		REGISTER_MEMBER(Camera, m_orthographicSize);
		REGISTER_MEMBER(Camera, m_screenWidth);
		REGISTER_MEMBER(Camera, m_screenHeight);
		REGISTER_MEMBER(Camera, m_type);
	END_REGISTER_CLASS;



	REGISTER_TYPE(SkinnedVertex);

	BEGIN_REGISTER_CLASS(SkinnedModel)
		REGISTER_TYPE(SkinnedModel);
		REGISTER_MEMBER(SkinnedModel, m_vertices);
		REGISTER_MEMBER(SkinnedModel, m_indices);
	END_REGISTER_CLASS;

	REGISTER_TYPE(Vertex);

	BEGIN_REGISTER_CLASS(Model)
		REGISTER_TYPE(Model);
		REGISTER_MEMBER(Model, m_vertices);
		REGISTER_MEMBER(Model, m_indices);
	END_REGISTER_CLASS;

		

}
