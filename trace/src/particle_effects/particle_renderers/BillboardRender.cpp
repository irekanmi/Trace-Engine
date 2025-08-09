#include "pch.h"

#include "BillboardRender.h"
#include "particle_effects/ParticleGenerator.h"
#include "backends/Renderutils.h"
#include "resource/DefaultAssetsManager.h"
#include "render/Camera.h"
#include "render/Renderer.h"
#include "particle_effects/ParticleEffect.h"


namespace trace {

	void BillBoardRender::Render(ParticleGeneratorInstance* gen_instance, Camera* camera, glm::mat4 transform)
	{
		ParticleData& particle_data = gen_instance->GetParticlesData();

		if (!m_texture)
		{
			return;
		}
		uint32_t num_alive = gen_instance->GetNumAlive();

		if (num_alive <= 0)
		{
			return;
		}

		Renderer* renderer = Renderer::get_instance();

		glm::mat4 proj = camera->GetProjectionMatix() * camera->GetViewMatrix();
		GDevice* device = renderer->GetDevice();
		Model& quad_model = renderer->GetQuadModel();

		glm::vec3 camera_position = camera->GetPosition();
		glm::mat4 model_pose = glm::mat4(1.0f);

		// NOTE: Enable if particle should be spawned in local_position
		model_pose = transform;

		if (m_velocityAligned)
		{
			int32_t remaining_particles = num_alive;
			uint32_t offset = 0;

			while (remaining_particles > 0)
			{
				uint32_t particles_to_render = remaining_particles > MAX_QUAD_INSTANCE ? MAX_QUAD_INSTANCE : remaining_particles;

				RenderFunc::OnDrawStart(device, DefaultAssetsManager::particle_velocity_aligned_pipeline.get());
				RenderFunc::SetPipelineTextureData(DefaultAssetsManager::particle_velocity_aligned_pipeline.get(), "u_textures", ShaderResourceStage::RESOURCE_STAGE_INSTANCE, m_texture.get());
				RenderFunc::SetPipelineData(DefaultAssetsManager::particle_velocity_aligned_pipeline.get(), "_projection", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, &proj, sizeof(glm::mat4));
				RenderFunc::SetPipelineData(DefaultAssetsManager::particle_velocity_aligned_pipeline.get(), "positions", ShaderResourceStage::RESOURCE_STAGE_INSTANCE, offset + particle_data.positions.data(), particles_to_render * sizeof(glm::vec4));
				RenderFunc::SetPipelineData(DefaultAssetsManager::particle_velocity_aligned_pipeline.get(), "colors", ShaderResourceStage::RESOURCE_STAGE_INSTANCE, offset + particle_data.color.data(), particles_to_render * sizeof(glm::vec4));
				RenderFunc::SetPipelineData(DefaultAssetsManager::particle_velocity_aligned_pipeline.get(), "scale", ShaderResourceStage::RESOURCE_STAGE_INSTANCE, offset + particle_data.scale.data(), particles_to_render * sizeof(glm::vec4));
				RenderFunc::SetPipelineData(DefaultAssetsManager::particle_velocity_aligned_pipeline.get(), "velocities", ShaderResourceStage::RESOURCE_STAGE_INSTANCE, offset + particle_data.velocities.data(), particles_to_render * sizeof(glm::vec4));
				RenderFunc::SetPipelineData(DefaultAssetsManager::particle_velocity_aligned_pipeline.get(), "model", ShaderResourceStage::RESOURCE_STAGE_INSTANCE, &model_pose, sizeof(glm::mat4));
				RenderFunc::BindPipeline_(DefaultAssetsManager::particle_velocity_aligned_pipeline.get());
				RenderFunc::BindPipeline(device, DefaultAssetsManager::particle_velocity_aligned_pipeline.get());
				RenderFunc::BindVertexBuffer(device, quad_model.GetVertexBuffer());
				RenderFunc::BindIndexBuffer(device, quad_model.GetIndexBuffer());
				RenderFunc::DrawIndexedInstanced(device, 0, quad_model.GetIndexCount(), particles_to_render);
				RenderFunc::OnDrawEnd(device, DefaultAssetsManager::particle_velocity_aligned_pipeline.get());

				remaining_particles -= MAX_QUAD_INSTANCE;
				offset += MAX_QUAD_INSTANCE;
			}

		}
		else
		{

			int32_t remaining_particles = num_alive;
			uint32_t offset = 0;

			while (remaining_particles > 0)
			{
				uint32_t particles_to_render = remaining_particles > MAX_QUAD_INSTANCE ? MAX_QUAD_INSTANCE : remaining_particles;

				RenderFunc::OnDrawStart(device, DefaultAssetsManager::particle_billboard_pipeline.get());
				RenderFunc::SetPipelineTextureData(DefaultAssetsManager::particle_billboard_pipeline.get(), "u_textures", ShaderResourceStage::RESOURCE_STAGE_INSTANCE, m_texture.get());
				RenderFunc::SetPipelineData(DefaultAssetsManager::particle_billboard_pipeline.get(), "_projection", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, &proj, sizeof(glm::mat4));
				RenderFunc::SetPipelineData(DefaultAssetsManager::particle_billboard_pipeline.get(), "camera_position", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, &camera_position, sizeof(glm::vec3));
				RenderFunc::SetPipelineData(DefaultAssetsManager::particle_billboard_pipeline.get(), "positions", ShaderResourceStage::RESOURCE_STAGE_INSTANCE, offset + particle_data.positions.data(), particles_to_render * sizeof(glm::vec4));
				RenderFunc::SetPipelineData(DefaultAssetsManager::particle_billboard_pipeline.get(), "colors", ShaderResourceStage::RESOURCE_STAGE_INSTANCE, offset + particle_data.color.data(), particles_to_render * sizeof(glm::vec4));
				RenderFunc::SetPipelineData(DefaultAssetsManager::particle_billboard_pipeline.get(), "scale", ShaderResourceStage::RESOURCE_STAGE_INSTANCE, offset + particle_data.scale.data(), particles_to_render * sizeof(glm::vec4));
				RenderFunc::SetPipelineData(DefaultAssetsManager::particle_billboard_pipeline.get(), "model", ShaderResourceStage::RESOURCE_STAGE_INSTANCE, &model_pose, sizeof(glm::mat4));
				RenderFunc::BindPipeline_(DefaultAssetsManager::particle_billboard_pipeline.get());
				RenderFunc::BindPipeline(device, DefaultAssetsManager::particle_billboard_pipeline.get());
				RenderFunc::BindVertexBuffer(device, quad_model.GetVertexBuffer());
				RenderFunc::BindIndexBuffer(device, quad_model.GetIndexBuffer());
				RenderFunc::DrawIndexedInstanced(device, 0, quad_model.GetIndexCount(), particles_to_render);
				RenderFunc::OnDrawEnd(device, DefaultAssetsManager::particle_billboard_pipeline.get());

				remaining_particles -= MAX_QUAD_INSTANCE;
				offset += MAX_QUAD_INSTANCE;
			}

		}

	}

}
