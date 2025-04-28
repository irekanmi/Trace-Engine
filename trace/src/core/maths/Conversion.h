#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include "orange_duck/quat.h"
#include "orange_duck/vec.h"

static inline orange_duck::vec3 glm_vec_to_org(glm::vec3& val)
{
    return orange_duck::vec3(val.x, val.y, val.z);
}

static inline orange_duck::quat glm_quat_to_org(glm::quat& val)
{
    return orange_duck::quat(val.w, val.x, val.y, val.z);
}

static inline glm::quat org_quat_to_glm(orange_duck::quat& val)
{
    return glm::quat(val.w, val.x, val.y, val.z);
}

static inline glm::vec3 org_vec_to_glm(orange_duck::vec3& val)
{
    return glm::vec3(val.x, val.y, val.z);
}
