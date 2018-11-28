#pragma once
#include "graphics/Definitions.h"
#include <vector>

namespace entity {
	class Entity;
}

namespace graphics {
	class Renderer;
}

namespace core {

	class Simulation
	{
	public:

		struct Parameters {
			bool gravityEnabled;
			bool timePaused;
			float elapsedTime;
			Parameters();
		};

	private:
		std::vector<entity::Entity*> entities;
		struct Environment {
			graphics::CenteredPoly floor;
			Environment();
		} environment;
		Parameters parameters;

	public:
		Simulation();
		~Simulation();

		void update();
		void render(graphics::Renderer&) const;
		void renderGUI();

		void add(entity::Entity*);

	private:
		void renderEntities(graphics::Renderer&) const;
		void renderEnvironment(graphics::Renderer&) const;
	};

}