#pragma once

namespace trace {
	class Transform;

}

namespace trace::Animation {
	class Pose;

	void BlendPose(Pose* source, Pose* target, Pose* result, float blend_weight);
	void BlendTransform(Transform* source, Transform* target, Transform* result, float blend_weight);
	

}
