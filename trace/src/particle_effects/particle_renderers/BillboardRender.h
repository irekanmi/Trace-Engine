#pragma once

#include "reflection/TypeRegistry.h"
#include "particle_effects/ParticleRender.h"
#include "render/GTexture.h"

#include "glm/glm.hpp"

namespace trace {

	class Camera;

	class BillBoardRender : public ParticleRender
	{

	public:

		virtual void Render(ParticleGeneratorInstance* gen_instance, Camera* camera, glm::mat4 transform) override;

		Ref<GTexture> GetTexture() { return m_texture; }
		void SetTexture(Ref<GTexture> texture) { m_texture = texture; }

		bool GetVelocityAligned() { return m_velocityAligned; }
		void SetVelocityAligned(bool velocity_aligned) { m_velocityAligned = velocity_aligned; }

	private:
		Ref<GTexture> m_texture;
		bool m_velocityAligned = false;

	protected:
		ACCESS_CLASS_MEMBERS(BillBoardRender);
		GET_TYPE_ID;

	};

}
