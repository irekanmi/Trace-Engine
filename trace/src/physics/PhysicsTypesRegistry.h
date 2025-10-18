#pragma once

#include "reflection/TypeRegistry.h"
#include "physics/Physics.h"

namespace trace {

	REGISTER_TYPE(RigidBodyType);

	BEGIN_REGISTER_CLASS(RigidBody)
		REGISTER_TYPE(RigidBody);
		REGISTER_MEMBER(RigidBody, mass);
		REGISTER_MEMBER(RigidBody, density);
		REGISTER_MEMBER(RigidBody, m_type);		
		REGISTER_MEMBER(RigidBody, _CCD);
		REGISTER_MEMBER(RigidBody, use_gravity);
	END_REGISTER_CLASS;

	REGISTER_TYPE(PhyShapeType);

	BEGIN_REGISTER_CLASS(PhyShape)
		REGISTER_TYPE(PhyShape);
		REGISTER_MEMBER(PhyShape, offset);
		REGISTER_MEMBER(PhyShape, type);
		REGISTER_MEMBER(PhyShape, data);
	END_REGISTER_CLASS;

	BEGIN_REGISTER_CLASS(CharacterController)
		REGISTER_TYPE(CharacterController);
		REGISTER_MEMBER(CharacterController, height);
		REGISTER_MEMBER(CharacterController, radius);
		REGISTER_MEMBER(CharacterController, min_move_distance);
		REGISTER_MEMBER(CharacterController, slope_limit);
		REGISTER_MEMBER(CharacterController, contact_offset);
		REGISTER_MEMBER(CharacterController, step_offset);
		REGISTER_MEMBER(CharacterController, offset);
	END_REGISTER_CLASS;
	


}
