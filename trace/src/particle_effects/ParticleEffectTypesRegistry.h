#pragma once


#include "reflection/TypeRegistry.h"
#include "particle_effects/ParticleEffect.h"
#include "particle_effects/ParticleGenerator.h"
#include "particle_effects/ParticleInitialize.h"
#include "particle_effects/ParticleRender.h"
#include "particle_effects/ParticleSpawner.h"
#include "particle_effects/ParticleUpdate.h"
#include "particle_effects/EmissionVolume.h"
#include "particle_effects/particle_renderers/BillboardRender.h"


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
	
	BEGIN_REGISTER_CLASS(EmissionVolume)
		REGISTER_TYPE(EmissionVolume);
	END_REGISTER_CLASS;
	
	
	BEGIN_REGISTER_CLASS(ParticleUpdate)
		REGISTER_TYPE(ParticleUpdate);
	END_REGISTER_CLASS;
	
	BEGIN_REGISTER_CLASS(BillBoardRender)
		REGISTER_TYPE_PARENT(BillBoardRender, ParticleRender);
		REGISTER_MEMBER(BillBoardRender, m_texture);
		REGISTER_MEMBER(BillBoardRender, m_velocityAligned);
	END_REGISTER_CLASS;
	
	BEGIN_REGISTER_CLASS(PointVolume)
		REGISTER_TYPE_PARENT(PointVolume, EmissionVolume);
		REGISTER_MEMBER(PointVolume, m_position);
	END_REGISTER_CLASS;
	
	BEGIN_REGISTER_CLASS(SphereVolume)
		REGISTER_TYPE_PARENT(SphereVolume, EmissionVolume);
		REGISTER_MEMBER(SphereVolume, m_center);
		REGISTER_MEMBER(SphereVolume, m_radius);
	END_REGISTER_CLASS;
	
	BEGIN_REGISTER_CLASS(BoxVolume)
		REGISTER_TYPE_PARENT(BoxVolume, EmissionVolume);
		REGISTER_MEMBER(BoxVolume, m_center);
		REGISTER_MEMBER(BoxVolume, m_extents);
		REGISTER_MEMBER(BoxVolume, m_axis);
	END_REGISTER_CLASS;
	
	BEGIN_REGISTER_CLASS(CircleVolume)
		REGISTER_TYPE_PARENT(CircleVolume, EmissionVolume);
		REGISTER_MEMBER(CircleVolume, m_center);
		REGISTER_MEMBER(CircleVolume, m_normal);
		REGISTER_MEMBER(CircleVolume, m_radius);
	END_REGISTER_CLASS;
	
	BEGIN_REGISTER_CLASS(ParticleSpawner)
		REGISTER_TYPE(ParticleSpawner);
		REGISTER_MEMBER(ParticleSpawner, m_emissionVolume);
	END_REGISTER_CLASS;
	
	BEGIN_REGISTER_CLASS(RateSpawner)
		REGISTER_TYPE_PARENT(RateSpawner, ParticleSpawner);
		REGISTER_MEMBER(RateSpawner, m_rate);
	END_REGISTER_CLASS;

	BEGIN_REGISTER_CLASS(VelocityInitializer)
		REGISTER_TYPE_PARENT(VelocityInitializer, ParticleInitializer);
		REGISTER_MEMBER(VelocityInitializer, m_minVelocity);
		REGISTER_MEMBER(VelocityInitializer, m_maxVelocity);
	END_REGISTER_CLASS;
	
	BEGIN_REGISTER_CLASS(LifetimeInitializer)
		REGISTER_TYPE_PARENT(LifetimeInitializer, ParticleInitializer);
		REGISTER_MEMBER(LifetimeInitializer, m_min);
		REGISTER_MEMBER(LifetimeInitializer, m_max);
	END_REGISTER_CLASS;
	
	BEGIN_REGISTER_CLASS(GravityUpdate)
		REGISTER_TYPE_PARENT(GravityUpdate, ParticleUpdate);
		REGISTER_MEMBER(GravityUpdate, m_gravity);
	END_REGISTER_CLASS;
	
	BEGIN_REGISTER_CLASS(DragUpdate)
		REGISTER_TYPE_PARENT(DragUpdate, ParticleUpdate);
		REGISTER_MEMBER(DragUpdate, m_dragCoefficient);
	END_REGISTER_CLASS;
	
	BEGIN_REGISTER_CLASS(WindUpdate)
		REGISTER_TYPE_PARENT(WindUpdate, ParticleUpdate);
		REGISTER_MEMBER(WindUpdate, m_windForce);
		REGISTER_MEMBER(WindUpdate, m_turbulence);
	END_REGISTER_CLASS;
	
	BEGIN_REGISTER_CLASS(VelocityUpdate)
		REGISTER_TYPE_PARENT(VelocityUpdate, ParticleUpdate);
	END_REGISTER_CLASS;
}
