#include "core/stdafx.h"
#include "MathUtils.h"

namespace math {

	glm::quat identityQuat()
	{
		return glm::angleAxis(0.0f, glm::vec3(1.0f, 0.0f, 0.0f));
	}

	//glm::quat rotationBetween(glm::vec3 start, glm::vec3 end)
	//{
	//	start = glm::normalize(start);
	//	end = glm::normalize(end);

	//	// get the angle
	//	float cosAngle = glm::dot(start, end);

	//	// get the axis
	//	glm::vec3 axis;
	//	// special case if vectors are exactly parallel
	//	if (std::abs(cosAngle) > 0.9999f)
	//	{
	//		// try crossing with y-axis
	//		axis = glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), start);
	//		if (axis.length() < 0.0001f) // start is parallel to y-axis
	//			axis = glm::cross(glm::vec3(1.0f, 0.0f, 0.0f), start);

	//		return glm::angleAxis(
	//			std::acosf(std::roundf(cosAngle)),
	//			normalize(axis)
	//		);
	//	}
	//	axis = glm::cross(start, end);
	//	return glm::angleAxis(std::acosf(cosAngle), glm::normalize(axis));
	//}

	glm::quat rotationBetween(glm::vec3 start, glm::vec3 end)
	{
		return glm::quat(start, end);
	}

	glm::quat lookToward(glm::vec3 direction, glm::vec3 up /* default y-axis */)
	{
		if (direction.length() < 1e-6f)
			return identityQuat();

		// make up perpendicular to direction
		{
			glm::vec3 right = glm::cross(direction, up);
			// if up and direction are parallel, just pick a coordinate axis for up
			if (right.length() < 1e-6f)
			{
				// try x-axis
				right = glm::cross(direction, glm::vec3(1.0f, 0.0f, 0.0f));
				// if x-axis fails, use z-axis
				if (right.length() < 1e-6f)
					right = glm::cross(direction, glm::vec3(0.0f, 0.0f, 1.0f));
			}

			up = glm::cross(right, direction);
		}

		glm::quat rot1 = rotationBetween(glm::vec3(1.0f, 0.0f, 0.0f), direction);
		glm::vec3 newUp = glm::mat3_cast(rot1) * up;
		glm::quat rot2 = rotationBetween(newUp, up);

		return rot2 * rot1;
	}
}