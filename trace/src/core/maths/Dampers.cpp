#include "pch.h"

#include "core/maths/Dampers.h"
#include "Utils.h"

#include "glm/glm.hpp"

namespace trace::Math {

    float fast_atan(float x)
    {
        float z = fabs(x);
        float w = z > 1.0f ? 1.0f / z : z;
        float y = (glm::pi<float>() / 4.0f) * w - w * (w - 1) * (0.2447f + 0.0663f * w);
        return copysign(z > 1.0f ? glm::pi<float>() / 2.0f - y : y, x);
    }

    float halflife_to_damping(float halflife, float eps = 1e-5f)
    {
        return (4.0f * 0.69314718056f) / (halflife + eps);
    }

    float damping_to_halflife(float damping, float eps = 1e-5f)
    {
        return (4.0f * 0.69314718056f) / (damping + eps);
    }


	float damper_exact(float x, float g, float halflife, float dt, float eps)
	{
		return lerp(x, g, 1.0f - fast_negexp((0.69314718056f * dt) / (halflife + eps)));
	}

    void spring_damper_exact(
        float& x,
        float& v,
        float x_goal,
        float v_goal,
        float stiffness,
        float damping,
        float dt,
        float eps)
    {
        float g = x_goal;
        float q = v_goal;
        float s = stiffness;
        float d = damping;
        float c = g + (d * q) / (s + eps);
        float y = d / 2.0f;

        if (fabs(s - (d * d) / 4.0f) < eps) // Critically Damped
        {
            float j0 = x - c;
            float j1 = v + j0 * y;

            float eydt = fast_negexp(y * dt);

            x = j0 * eydt + dt * j1 * eydt + c;
            v = -y * j0 * eydt - y * dt * j1 * eydt + j1 * eydt;
        }
        else if (s - (d * d) / 4.0f > 0.0) // Under Damped
        {
            float w = sqrtf(s - (d * d) / 4.0f);
            float j = sqrtf(squaref(v + y * (x - c)) / (w * w + eps) + squaref(x - c));
            float p = fast_atan((v + (x - c) * y) / (-(x - c) * w + eps));

            j = (x - c) > 0.0f ? j : -j;

            float eydt = fast_negexp(y * dt);

            x = j * eydt * cosf(w * dt + p) + c;
            v = -y * j * eydt * cosf(w * dt + p) - w * j * eydt * sinf(w * dt + p);
        }
        else if (s - (d * d) / 4.0f < 0.0) // Over Damped
        {
            float y0 = (d + sqrtf(d * d - 4 * s)) / 2.0f;
            float y1 = (d - sqrtf(d * d - 4 * s)) / 2.0f;
            float j1 = (c * y0 - x * y0 - v) / (y1 - y0);
            float j0 = x - j1 - c;

            float ey0dt = fast_negexp(y0 * dt);
            float ey1dt = fast_negexp(y1 * dt);

            x = j0 * ey0dt + j1 * ey1dt + c;
            v = -y0 * j0 * ey0dt - y1 * j1 * ey1dt;
        }
    }

    void critical_spring_damper_exact(
        float& x,
        float& v,
        float x_goal,
        float v_goal,
        float halflife,
        float dt)
    {
        float g = x_goal;
        float q = v_goal;
        float d = halflife_to_damping(halflife);
        float c = g + (d * q) / ((d * d) / 4.0f);
        float y = d / 2.0f;
        float j0 = x - c;
        float j1 = v + j0 * y;
        float eydt = fast_negexp(y * dt);

        x = eydt * (j0 + j1 * dt) + c;
        v = eydt * (v - j1 * y * dt);
    }

    void simple_spring_damper_exact(
        float& x,
        float& v,
        float x_goal,
        float halflife,
        float dt)
    {
        float y = halflife_to_damping(halflife) / 2.0f;
        float j0 = x - x_goal;
        float j1 = v + j0 * y;
        float eydt = fast_negexp(y * dt);

        x = eydt * (j0 + j1 * dt) + x_goal;
        v = eydt * (v - j1 * y * dt);
    }


    void spring_character_update(
        float& x,
        float& v,
        float& a,
        float v_goal,
        float halflife,
        float dt)
    {
        float y = halflife_to_damping(halflife) / 2.0f;
        float j0 = v - v_goal;
        float j1 = a + j0 * y;
        float eydt = fast_negexp(y * dt);

        x = eydt * (((-j1) / (y * y)) + ((-j0 - j1 * dt) / y)) +
            (j1 / (y * y)) + j0 / y + v_goal * dt + x;
        v = eydt * (j0 + j1 * dt) + v_goal;
        a = eydt * (a - j1 * y * dt);
    }

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
        float dt)
    {
        for (int i = 0; i < count; i++)
        {
            px[i] = x;
            pv[i] = v;
            pa[i] = a;
        }

        for (int i = 0; i < count; i++)
        {
            spring_character_update(px[i], pv[i], pa[i], v_goal, halflife, i * dt);
        }
    }


}