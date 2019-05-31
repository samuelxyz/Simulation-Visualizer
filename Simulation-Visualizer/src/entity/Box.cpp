#include "Box.h"
#include "core/Definitions.h"

namespace entity {

	Box::Box(std::string entityName, glm::vec3 position, glm::quat orientation, glm::vec3 velocity, glm::quat angVel,
		float mass, glm::mat3 rotInertia,
		float xMin, float xMax, float yMin, float yMax, float zMin, float zMax)
		: Entity(entityName, position, orientation, velocity, angVel, mass, rotInertia, "EntityBox"),
		xMin(xMin), xMax(xMax), yMin(yMin), yMax(yMax), zMin(zMin), zMax(zMax),
		visualBox(position, orientation, xMin, xMax, yMin, yMax, zMin, zMax)
	{
	}

	Box::~Box()
	{
	}

	bool Box::containsPoint(glm::vec3 worldPoint) const
	{
		glm::vec3 localPoint = toLocalFrame(worldPoint);
		return 
			xMin <= localPoint.x && localPoint.x <= xMax &&
			yMin <= localPoint.y && localPoint.y <= yMax &&
			zMin <= localPoint.z && localPoint.z <= zMax;
	}

	void Box::render(graphics::Renderer& renderer) const
	{
		visualBox.position = position;
		visualBox.orientation = orientation;

		visualBox.render(renderer);
	}

	int Box::funcEval(int n, double * z, double * F)
	{
		// for collision with horizontal floor. Let's try that for now

		return 0;
		// algebraic: limits (-1e20, 1e20) -> F(z) = 0
		// complementarity: limits (0, 1e20) -> 0 <= z perp F >= 0

		// variables:
			// 0-2:		linear velocity
			// 3-5:		angular velocity
			// 6:		friction sigma (slip velocity magnitude)
			// 7-10:	contact impulses (n, t, o, r)
			// 11-13:	a_1
			// 14-16:	a_2
			// 17-:		KKT multipliers
		
		// variables at beginning of time step
		glm::vec3& vel_now = velocity;
		glm::vec3 omeg_now = glm::axis(angVelocity)*glm::angle(angVelocity);

		// give nice names to all the variables 
		glm::vec3	vel_next(z[0], z[1], z[2]);
		glm::vec3	omeg_next(z[3], z[4], z[5]);
		float		friction_sigma = z[6];
		float		impulse_n = z[7];
		float		impulse_t = z[8];
		float		impulse_o = z[9];
		float		impulse_r = z[10];
		glm::vec3	ecp_a_1(z[11], z[12], z[13]);
		glm::vec3	ecp_a_2(z[14], z[15], z[16]);
		double*		kkt_multipliers = z + 17;

		// useful things

		//static const glm::vec3 wrench_n_f(0.0f, 1.0f, 0.0f);
		//       const glm::vec3 wrench_n_m(glm::cross(r, wrench_n_f));
		//static const glm::vec3 wrench_t_f(1.0f, 0.0f, 0.0f);
		//       const glm::vec3 wrench_t_m(glm::cross(r, wrench_t_f));
		//static const glm::vec3 wrench_o_f(0.0f, 0.0f, 1.0f);
		//       const glm::vec3 wrench_o_m(glm::cross(r, wrench_o_f));
		//static const glm::vec3 wrench_r_f(0.0f);
		//       const glm::vec3 wrench_r_m(wrench_n_f);

		// unit vectors for wrenches
		const glm::vec3 // doesn't change until next function call
			n_vec(0.0f, 1.0f, 0.0f),
			t_vec(1.0f, 0.0f, 0.0f),
			o_vec(0.0f, 0.0f, 1.0f),
			r_vec = ecp_a_1 - (position + vel_next*core::TIME_STEP); // from box center to contact point
		// other stuff
		static const glm::vec3 impulse_applied(0.0f, -core::GRAVITY*core::TIME_STEP, 0.0f);
		const glm::mat3 R = glm::mat3_cast(orientation);
		const glm::mat3 world_inertia = R*rotInertia*glm::transpose(R); // I'

		// Newton-Euler equations
		glm::vec3 F_0_2 = mass*(vel_now - vel_next) + n_vec*impulse_n + 
			t_vec*impulse_t + o_vec*impulse_o + impulse_applied;
		glm::vec3 F_3_5 = world_inertia*(omeg_now - omeg_next) + 
			glm::cross(r_vec, n_vec)*impulse_n + glm::cross(r_vec, t_vec)*impulse_t + 
			glm::cross(r_vec, o_vec)*impulse_o + n_vec*impulse_r + impulse_applied;
		//glm::vec3 F_1_3 = wrench_n_f*impulse_n + wrench_t_f*impulse_t + wrench_o_f*impulse_o /*+ wrench_r_f*impulse_r*/ + impulse_applied;

		

	}

	int Box::jacEval(int n, int nnz, double * z, int * indices_of_used_columns, 
		int * len_of_each_column, int * row_index_of_each_value, double * values)
	{
		return 0;
	}
}