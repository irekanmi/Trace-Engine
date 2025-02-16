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
	END_REGISTER_CLASS;

	REGISTER_TYPE(PhyShapeType);

	BEGIN_REGISTER_CLASS(PhyShape)
		REGISTER_TYPE(PhyShape);
		REGISTER_MEMBER(PhyShape, offset);
		REGISTER_MEMBER(PhyShape, type);
		REGISTER_MEMBER(PhyShape, data);
	END_REGISTER_CLASS;

	


}
