#pragma once
#include "graphics/content/VisualBox.h"
#include "graphics/Camera.h"
#include <array>

namespace core {

	class GUIOverlay :	public graphics::Renderable
	{
	public:
		GUIOverlay(graphics::Camera& camera);
		virtual ~GUIOverlay();

		virtual void render(graphics::Renderer& renderer) const;

	private:
		graphics::Camera& camera;
		static constexpr float axisLength = 0.05f;
		static constexpr float axisThickness = 0.001f;
		mutable std::array<graphics::VisualBox, 3> axes;
	};

}