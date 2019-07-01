#pragma once
#include "core/Definitions.h"
#include "core/Definitions.h"
#include "glm/gtx/quaternion.hpp"
#include "glm/vec3.hpp"
#include <vector>

namespace entity {
	class Entity;
	class Box;
}

namespace graphics {
	class Renderer;
	class Camera;
}

namespace core {

	class Simulation
	{
	public:

		struct Parameters {
			bool timePaused;
			float playbackTime;
			int currentStep;
			bool showShadows;
			bool showEnvironment;
			bool showContactPoint;
			float playbackSpeed;
			bool loopPlayback;
			Parameters();
		};

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
				const glm::vec3& contactPoint);

			bool hasContactPoint() const { return position != contactPoint; }
		};

		struct Timeline : public std::vector<Timestep>
		{
			float timeOf(int timestepIndex) const;
			float backTime() const;
		};

		struct PathSim
		{
			entity::Box* box;

			static constexpr double g = static_cast<double>(core::GRAVITY);
			static constexpr double h = static_cast<double>(core::TIME_STEP);
			static constexpr double PATH_INFINITY = 1e20;

			// cache
			double q_xo, q_yo, q_zo;
			double q0_o, q1_o, q2_o, q3_o;
			double v_xo, v_yo, v_zo;
			double w_xo, w_yo, w_zo;
			// applied impulses
			double p_x, p_y, p_z, p_xt, p_yt, p_zt;

			// constants
			double m;
			double mu;
			double I_xx, I_yy, I_zz;
			double e_o, e_r, e_t;
			double len, wid, heg;

			// path vars
			static constexpr int n = 24; // number of unknowns
			static constexpr int nnz = 24*24; // overestimate of jacobian nonzeros
			int status;
			double z[n];
			double F[n];
			double lower_bounds[n];
			double upper_bounds[n];

			PathSim();
			void setBox(entity::Box*);
			int cubeFuncEval(double* z, double* F);
			int cubeJacEval(double* z, int* column_starting_indices,
				int* len_of_each_column, int* row_index_of_each_value, double* values);
			// Returns true if PATH solves successfully
			bool addStep(Simulation::Timeline& timeline);
			void captureBoxState();
			void printZ();
			void printCache();
			void printConstants();
		} pathSim;

	private:
		static constexpr unsigned int FPS_TRACKER_SMOOTHING = 30u;

		std::vector<entity::Entity*> entities;
		struct Environment {
			graphics::CenteredPoly floor;
			Environment();
		} environment;
		Parameters parameters;
		Timeline timeline;

	public:
		Simulation();
		~Simulation();

		void update();
		void render(graphics::Renderer& renderer, const glm::vec3& cameraPos) const;
		void renderGUI(const graphics::Camera& camera);
		void renderGUIOverlay(graphics::Renderer& renderer, const graphics::Camera& camera) const;

		void add(entity::Entity*);
		const glm::vec3& getFocusedEntityPosition() const;

		int addSteps(Simulation::Timeline& timeline, int numSteps);
		void recordTimestep(entity::Entity*);
		// Returns true on success
		bool addFreefallStep(entity::Entity*);

	private:
		void renderEntities(graphics::Renderer&, const glm::vec3& cameraPos) const;
		void renderEnvironment(graphics::Renderer&) const;
		void renderContactPoint(graphics::Renderer&, const graphics::Camera& camera) const;
	};

}