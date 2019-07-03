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
			Parameters();
		};

		PathSim* pathSim;

	private:
		static constexpr unsigned int FPS_TRACKER_SMOOTHING = 30u;

		std::vector<entity::Entity*> entities;
		struct Environment
		{
			graphics::CenteredPoly floor;
			Environment();
		} environment;
		Parameters parameters;
		Timeline timeline;

	public:
		Simulation(PathSim* pathSim);
		~Simulation();

		void update();
		void render(graphics::Renderer& renderer, const glm::vec3& cameraPos) const;
		void renderGUI(const graphics::Camera& camera);
		void renderGUIOverlay(graphics::Renderer& renderer, const graphics::Camera& camera) const;

		void add(entity::Entity*);
		const glm::vec3& getFocusedEntityPosition() const;

		int addSteps(core::Timeline& timeline, int numSteps);
		int addStepsUntilEnd(core::Timeline& timeline);
		void recordTimestep(entity::Entity*);
		// Returns true on success
		bool addFreefallStep(entity::Entity*);

	private:
		void renderEntities(graphics::Renderer&, const glm::vec3& cameraPos) const;
		void renderEnvironment(graphics::Renderer&) const;
		void renderContactPoint(graphics::Renderer&, const graphics::Camera& camera) const;
	};

}