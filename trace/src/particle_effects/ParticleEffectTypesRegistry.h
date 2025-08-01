#pragma once


#include "reflection/TypeRegistry.h"
#include "particle_effects/ParticleEffect.h"
#include "particle_effects/ParticleGenerator.h"
#include "particle_effects/ParticleInitialize.h"
#include "particle_effects/ParticleRender.h"
#include "particle_effects/ParticleSpawner.h"
#include "particle_effects/ParticleUpdate.h"


namespace trace {

	BEGIN_REGISTER_CLASS(ParticleEffect)
		REGISTER_TYPE(ParticleEffect);
		REGISTER_MEMBER(ParticleEffect, m_lifeTime);
		REGISTER_MEMBER(ParticleEffect, m_generators);
	END_REGISTER_CLASS;
	
	BEGIN_REGISTER_CLASS(ParticleEffectInstance)
		REGISTER_TYPE(ParticleEffectInstance);
		REGISTER_MEMBER(ParticleEffectInstance, m_particleEffect);
	END_REGISTER_CLASS;
	
	BEGIN_REGISTER_CLASS(ParticleGenerator)
		REGISTER_TYPE(ParticleGenerator);
		REGISTER_MEMBER(ParticleGenerator, m_spawner);
		REGISTER_MEMBER(ParticleGenerator, m_initializers);
		REGISTER_MEMBER(ParticleGenerator, m_updates);
		REGISTER_MEMBER(ParticleGenerator, m_renderers);
		REGISTER_MEMBER(ParticleGenerator, m_capacity);
	END_REGISTER_CLASS;
	
	BEGIN_REGISTER_CLASS(ParticleGeneratorInstance)
		REGISTER_TYPE(ParticleGeneratorInstance);
		REGISTER_MEMBER(ParticleGeneratorInstance, m_gen);
	END_REGISTER_CLASS;
	
	BEGIN_REGISTER_CLASS(ParticleInitializer)
		REGISTER_TYPE(ParticleInitializer);
	END_REGISTER_CLASS;
	
	BEGIN_REGISTER_CLASS(ParticleRender)
		REGISTER_TYPE(ParticleRender);
	END_REGISTER_CLASS;
	
	BEGIN_REGISTER_CLASS(ParticleSpawner)
		REGISTER_TYPE(ParticleSpawner);
	END_REGISTER_CLASS;
	
	BEGIN_REGISTER_CLASS(ParticleUpdate)
		REGISTER_TYPE(ParticleUpdate);
	END_REGISTER_CLASS;

}
