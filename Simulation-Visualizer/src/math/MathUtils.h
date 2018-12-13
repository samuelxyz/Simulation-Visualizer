#pragma once

#include "glm/gtx/quaternion.hpp"

namespace math {
	glm::quat identityQuat();
	glm::quat rotationBetween(glm::vec3 start, glm::vec3 end);
	glm::quat lookToward(glm::vec3 direction, glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f));
}