#pragma once
#include "glm/vec3.hpp"
#include "glm/gtx/quaternion.hpp"

namespace core {
	// Value used for simulation
	static constexpr float TIME_STEP = 1.0f/60, GRAVITY = 9.80665f;

	static constexpr float
		PI = 3.141592653589793f,
		TWO_PI = 6.283185307179586f,
		HALF_PI = 1.570796326794897f,
		QUARTER_PI = 0.785398163397448f;

	// A unit vector pointing vertically upward (+z direction)
	static const glm::vec3 VECTOR_UP(0.0f, 0.0f, 1.0f);

	// A "default" quaternion representing no rotation
	static const glm::quat QUAT_ZERO = glm::angleAxis(0.0f, glm::vec3(1.0f, 0.0f, 0.0f));
}