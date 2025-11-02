#pragma once

#include "reflection/TypeRegistry.h"
#include "particle_effects/ParticleRender.h"
#include "render/GTexture.h"
#include "render/Material.h"

#include "glm/glm.hpp"

namespace trace {

	class Camera;

	class BillBoardRender : public ParticleRender
	{

	public:

		virtual void Render(ParticleGeneratorInstance* gen_instance, Camera* camera, glm::mat4 transform, int32_t render_graph_index = 0) override;

		Ref<GTexture> GetTexture() { return m_texture; }
		void SetTexture(Ref<GTexture> texture) { m_texture = texture; }

		bool GetVelocityAligned() { return m_velocityAligned; }
		void SetVelocityAligned(bool velocity_aligned) { m_velocityAligned = velocity_aligned; }

		Ref<MaterialInstance> GetMaterial() { return m_material; }
		void SetMaterial(Ref<MaterialInstance> material) { m_material = material; }

	private:
		Ref<GTexture> m_texture;
		bool m_velocityAligned = false;
		Ref<MaterialInstance> m_material;

	protected:
		ACCESS_CLASS_MEMBERS(BillBoardRender);
		GET_TYPE_ID;

	};

}
