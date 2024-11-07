#pragma once


namespace trace::Animation {
	class Pose;

	void BlendPose(Pose* source, Pose* target, Pose* result, float blend_weight);

}
