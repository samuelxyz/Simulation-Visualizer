#pragma once
#include "core/Definitions.h"
extern "C" {
#include "path_standalone/Standalone_Path.h"
}

namespace entity {
	class Entity;
	class Box;
}

namespace graphics {
	class Renderer;
	class Camera;
}

namespace core {

	struct PathSim;

	class Simulation
	{
	public:

		struct Parameters 
		{
			bool timePaused;
			float playbackTime;
			int currentStep;
			bool showShadows;
			bool showEnvironment;
			bool showContactPoint;
			float playbackSpeed;
			bool loopPlayback;
			bool logOutput;
			bool logCalcTime;
			Parameters();
		};

		PathSim* pathSim; // needs to be accessed from PATH solver

	private:
		static constexpr unsigned int FPS_TRACKER_SMOOTHING = 30u;

		std::vector<entity::Entity*> entities;
		// for previews, etc. Will not be updated or otherwise interactable
		std::vector<entity::Entity*> entitiesVisualOnly;
		struct Environment
		{
			graphics::CenteredPoly floor;
			Environment();
		} environment;
		Parameters parameters;
		Timeline timeline;
		bool showAddEntityWindow;

	public:
		Simulation();
		~Simulation();

		void update();
		// Calls Renderer::renderAndClearAll()
		void render(graphics::Renderer& renderer, const glm::vec3& cameraPos) const;
		// May contribute an extra entity to Simulation's render list. Call before Simulation::render()
		void renderGUI(graphics::Renderer&, const graphics::Camera& camera);
		// Entity vectors, markers, contact points, etc
		void renderGUIOverlay(graphics::Renderer& renderer, const graphics::Camera& camera) const;

		void add(entity::Entity*);
		void setTarget(entity::Entity*, bool autoAdd = true);

		// Returns with the following priority:
		// - Hovered entity
		// - Onscreen entity nearest to the camera
		// - (Entity)nullptr
		const entity::Entity* const getFocusedEntity(const graphics::Camera& camera) const;
		const entity::Entity* const getHoveredEntity(const graphics::Camera& camera) const;

		int addSteps(core::Timeline& timeline, int numSteps, bool breakOnConstantMotion = false);
		int addStepsUntilEnd(core::Timeline& timeline);
		void recordTimestep(entity::Entity*);
		// Returns true on success
		bool addFreefallStep(entity::Entity*);

		void scrollTimeline(int scr);
		void startStop();

	private:
		void renderEntities(graphics::Renderer&) const;
		void renderShadows(graphics::Renderer&, const glm::vec3& cameraPos) const;
		void renderEnvironment(graphics::Renderer&) const;
		void renderContactPoint(graphics::Renderer&, const graphics::Camera& camera) const;
		void renderAddEntityGUI(graphics::Renderer&, const graphics::Camera&);
	};

}