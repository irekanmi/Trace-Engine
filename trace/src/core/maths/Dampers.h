#pragma once


namespace trace::Math {

	inline float fast_negexp(float x)
	{
		return 1.0f / (1.0f + x + 0.48f * x * x + 0.235f * x * x * x);
	}

    float fast_atan(float x);

    inline float squaref(float x)
    {
        return x * x;
    }

	float damper_exact(float x, float g, float halflife, float dt, float eps = 1e-5f);

    void spring_damper_exact(
        float& x,
        float& v,
        float x_goal,
        float v_goal,
        float stiffness,
        float damping,
        float dt,
        float eps = 1e-5f);

    void critical_spring_damper_exact(
        float& x,
        float& v,
        float x_goal,
        float v_goal,
        float halflife,
        float dt);

    void simple_spring_damper_exact(
        float& x,
        float& v,
        float x_goal,
        float halflife,
        float dt);

    void spring_character_update(
        float& x,
        float& v,
        float& a,
        float v_goal,
        float halflife,
        float dt);

    void spring_character_predict(
        float px[],
        float pv[],
        float pa[],
        int count,
        float x,
        float v,
        float a,
        float v_goal,
        float halflife,
        float dt);

}
