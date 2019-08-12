#pragma once
#include "core/Definitions.h"
#include "eqn/EqnAssembler.h"
#include "eqn/TimelineE.h"

namespace entity {
	class Entity;
	class Box;
}

namespace graphics {
	class Renderer;
	class Camera;
}

namespace io {
	class MouseDragTarget;
	struct DragHandle;
}

namespace core {

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
			bool showContactPoints;
			float playbackSpeed;
			bool loopPlayback;
			bool logOutput;
			bool logCalcTime;
			Parameters();
		};

		eqn::EquationAssembler eqnA; // needs to be accessed from PATH solver

	private:
		static constexpr unsigned int FPS_TRACKER_SMOOTHING = 30u;

		std::vector<entity::Entity*> entities;
		// for previews, etc. Will not be updated or otherwise interactable
		std::vector<entity::Entity*> entitiesVisualOnly;
		std::vector<io::DragHandle*> dragHandles;
		struct Environment
		{
			graphics::CenteredPoly floor;
			Environment();
		} environment;
		Parameters parameters;
		eqn::TimelineE timeline;
		bool showAddEntityWindow;

	public:
		Simulation();
		~Simulation();

		void update();
		// Calls Renderer::renderAndClearAll()
		void render(graphics::Renderer& renderer, const glm::vec3& cameraPos) const;
		// May contribute an extra entity to Simulation's render list. Call before Simulation::render()
		void renderGUI(graphics::Renderer&, const graphics::Camera& camera, const io::MouseDragTarget* leftDragTarget);
		// Entity vectors, markers, contact points, etc
		void renderGUIOverlay(graphics::Renderer& renderer, const graphics::Camera& camera) const;

		void add(entity::Entity*);

		void addDragHandle(io::DragHandle*);

		// Returns with the following priority:
		// - Hovered entity
		// - Onscreen entity nearest to the camera
		// - (Entity)nullptr
		entity::Entity* getFocusedEntity(const graphics::Camera& camera);
		entity::Entity* getHoveredEntity(const graphics::Camera& camera);
		io::MouseDragTarget* getLeftMouseDragTarget(const graphics::Camera& camera);

		//void recordTimestep(entity::Entity*);
		void loadTimestep(int step);
		//// Returns true on success
		//bool addFreefallStep(entity::Entity*);

		void scrollTimeline(int scr);
		void startStop();

	private:
		void renderEntities(graphics::Renderer&) const;
		void renderShadows(graphics::Renderer&, const glm::vec3& cameraPos) const;
		void renderEnvironment(graphics::Renderer&) const;
		void renderContactPoints(graphics::Renderer&, const graphics::Camera& camera) const;
		void renderAddEntityGUI(graphics::Renderer&, const graphics::Camera&);
		io::DragHandle* getHoveredDragHandle(const graphics::Camera& camera);

		// remove an entity
		void remove(int entityIndex);
	};

}