#pragma once
#include "glm/gtx/quaternion.hpp"
#include "eqn/AutodiffTypes.h"
#include <vector>

#define LAUNCH_DIRECT
#define DECLARE_PATH_FUNCS_IN_MAIN
#ifndef DECLARE_PATH_FUNCS_IN_MAIN
#define DO_CUBE_TEST
#endif

#define START_TIMING \
auto startTime = std::chrono::steady_clock::now()
#define STOP_TIMING_AND_GET_MICROSECONDS \
std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now()-startTime).count()

#define USE_RMS_COLOR_BLEND

namespace core {
	// Value used for simulation
	static constexpr float TIME_STEP = 1e-2f, GRAVITY = 9.80665f;
	static constexpr float FLOOR_Z = 0.0f;
	static constexpr double PATH_INFINITY = 1e20;

	static constexpr float
		PI = 3.141592653589793f,
		TWO_PI = 6.283185307179586f,
		HALF_PI = 1.570796326794897f,
		QUARTER_PI = 0.785398163397448f;

	// A unit vector pointing vertically upward (+z direction)
	static const glm::vec3 VECTOR_UP(0.0f, 0.0f, 1.0f);

	// A "default" quaternion representing no rotation
	static const glm::quat QUAT_IDENTITY = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

	// Inputs need not be normalized
	static glm::quat quatLookAt(const glm::vec3& direction, const glm::vec3& up = VECTOR_UP)
	{
		float dirLength2 = glm::length2(direction);
		if (dirLength2 < 1e-12 || !std::isnormal(dirLength2))
		{
			return QUAT_IDENTITY;
		}
		else
		{
			// glm assumes identity quat looks toward -z. However we look toward +x, so add a rotation to compensate
			float upLength2 = glm::length2(up);
			glm::vec3 ndir = glm::normalize(direction);
			glm::vec3 nup = glm::normalize(up);

			if (upLength2 < 1e-12 || !std::isnormal(upLength2))
				return glm::quatLookAt(ndir, VECTOR_UP) * glm::angleAxis(HALF_PI, glm::vec3(0.0f, 1.0f, 0.0f));
			else
				// singularity condition
				if (glm::dot(ndir, nup) > 0.999999f)
					return glm::angleAxis(HALF_PI, glm::cross(glm::vec3(1.0f, 0.0f, 0.0f), nup));// assumes up is an axial direction
				else if (glm::dot(ndir , nup) < -0.999999f)
					return glm::angleAxis(HALF_PI, glm::cross(nup, glm::vec3(1.0f, 0.0f, 0.0f)));// assumes up is an axial direction
				else
					return glm::quatLookAt(ndir, nup) * glm::angleAxis(HALF_PI, glm::vec3(0.0f, 1.0f, 0.0f));
		}
	}

	static float mapRange(float val, float minOld, float maxOld, float minNew, float maxNew)
	{
		return (val - minOld) * (maxNew - minNew) / (maxOld - minOld) + minNew;
	}

	struct Timestep
	{
		glm::vec3 position;
		glm::quat orientation;
		glm::vec3 velocity;
		glm::vec3 angVelocity;
		glm::vec3 contactPoint; // if no contact point (freefall) then this will equal position

		Timestep(
			const glm::vec3& position, const glm::quat& orientation,
			const glm::vec3& velocity, const glm::vec3& angVelocity,
			const glm::vec3& contactPoint)
			: position(position), orientation(orientation), velocity(velocity),
			angVelocity(angVelocity), contactPoint(contactPoint)
		{
		}

		bool hasContactPoint() const { return position != contactPoint; }
	};

	struct Timeline : public std::vector<Timestep>
	{
		float timeOf(int timestepIndex) const { return timestepIndex * core::TIME_STEP; }
		float backTime() const { return timeOf(size() - 1); }
	};
}

namespace graphics {

	struct ColoredVertex
	{
		glm::vec4 color;
		glm::vec3 position;

		ColoredVertex(const glm::vec4& color, const glm::vec3& position)
			: color(color), position(position)
		{}
	};

	static const glm::vec4
		COLOR_RED { 1.0f, 0.0f, 0.0f, 1.0f },
		COLOR_GREEN { 0.0f, 1.0f, 0.0f, 1.0f },
		COLOR_BLUE { 0.0f, 0.0f, 1.0f, 1.0f },
		COLOR_CYAN { 0.0f, 1.0f, 1.0f, 1.0f },
		COLOR_MAGENTA { 1.0f, 0.0f, 1.0f, 1.0f },
		COLOR_YELLOW { 1.0f, 1.0f, 0.0f, 1.0f },
		COLOR_WHITE { 1.0f, 1.0f, 1.0f, 1.0f },
		COLOR_BLACK { 0.0f, 0.0f, 0.0f, 1.0f },
		COLOR_TRANSPARENT { 0.0f },
		COLOR_PINK { 1.0f, 0.5f, 0.75f, 1.0f },

		// gui colors are slightly brighter so they can always stand out from entities
		COLOR_GUI_RED { 1.0f, 0.15f, 0.15f, 1.0f },
		COLOR_GUI_GREEN { 0.15f, 1.0f, 0.15f, 1.0f },
		COLOR_GUI_BLUE { 0.15f, 0.15f, 1.0f, 1.0f },
		COLOR_GUI_CYAN { 0.15f, 1.0f, 1.0f, 1.0f },
		COLOR_GUI_MAGENTA { 1.0f, 0.15f, 1.0f, 1.0f },
		COLOR_GUI_YELLOW { 1.0f, 1.0f, 0.15f, 1.0f },
		COLOR_GUI_BLACK { 0.15f, 0.15f, 0.15f, 1.0f },
		COLOR_GUI_PINK { 1.0f, 0.6f, 0.8f, 1.0f };


	// theme colors
	static const glm::vec4&
		COLOR_NONE { COLOR_TRANSPARENT },
		COLOR_DEFAULT { COLOR_NONE },
		COLOR_VELOCITY { COLOR_GUI_CYAN },
		COLOR_ANGVEL { COLOR_GUI_MAGENTA },
		COLOR_CONTACT { COLOR_GUI_RED },
		COLOR_NO_CONTACT { COLOR_GUI_PINK },
		COLOR_ORBIT { COLOR_GUI_GREEN };

	typedef std::array<ColoredVertex, 3> Triangle;
	typedef std::array<ColoredVertex, 4> Quad; // clockwise or counterclockwise
	typedef std::vector<ColoredVertex> CenteredPoly; // first vertex is center

	static const ImVec2
		PIVOT_LEFT { 0.0f, 0.5f },
		PIVOT_CENTER { 0.5f, 0.5f },
		PIVOT_RIGHT { 1.0f, 0.5f },
		PIVOT_TOP { 0.5f, 0.0f },
		PIVOT_BOTTOM { 0.5f, 1.0f };

	static constexpr float FRAME_DURATION = 1.0f/60;
	static constexpr float RENDER_DISTANCE = 100.0f;

	// height of the visual "floor" in the visualization
	static constexpr float FLOOR_Z = core::FLOOR_Z;
	static constexpr float MARKER_DOT_RADIUS = 0.04f;

	static glm::vec4 colorMix(const glm::vec4& x, const glm::vec4& y, float a)
	{
#ifndef USE_RMS_COLOR_BLEND
		return glm::mix(x, y, a);
#else
		// using correct gamma stuff maybe? @minutephysics
		glm::vec4 result(0.0f);
		for (int i = 0; i < 4; ++i)
		{
			result[i] = std::sqrt((x[i]*x[i]*(1-a) + y[i]*y[i]*a));
		}
		return result;
#endif
	}
}

namespace eqn {
	static const autodiff::dual h = core::TIME_STEP;
	static const autodiff::dual g = core::GRAVITY;
}