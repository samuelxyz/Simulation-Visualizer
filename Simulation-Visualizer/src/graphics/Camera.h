#pragma once

#include <glm/gtx/quaternion.hpp>

#include "core/Definitions.h"

namespace graphics {

	class Renderer;

	class Camera
	{
	private:
		// Camera Attributes
		glm::vec3 position;
		float pitch, yaw; // radians, yaw is from +x toward +y (counterclockwise for player)
		float fov; // also radians
		int windowWidth, windowHeight;
		bool showAxes;

		// Camera options
		static constexpr float
			movementSpeed = 0.1f,
			mouseSensitivity = 1e-3f,
			scrollSensitivity = 0.1f,
			ctrlSpeedModifier = 3.0f,
			fovMin = 1.0f * core::PI/180.0f,
			fovMax = 89.0f * core::PI/180.0f;

	public:
		Camera(int windowWidth, int windowHeight, 
			glm::vec3 position = glm::vec3(0.0f), float pitch = 0.0f, float yaw = 0.0f);

		glm::mat4 getVPMatrix() const;
		glm::vec3 getPosition() const;
		glm::vec3 getLookVec() const;
		glm::vec2 toScreenSpace(const glm::vec3& worldPos) const;
		bool shouldShowAxes() const { return showAxes; }

		void renderGUI();

		// TitleID does not need "##"
		// TextColor zero vector to use ImGui default
		void renderLabel(const glm::vec3& worldPos, 
			bool backgroundOnHoverOnly, const std::string& titleID,
			const std::string& text, const glm::vec4& textColor = COLOR_NONE, 
			const ImVec2& pivot = PIVOT_LEFT) const;

		void updateWindowDims(int width, int height);
		void pollKeys(GLFWwindow* window);
		void handleKey(int key, int action);
		void handleLeftMouseMotion(float xoffset, float yoffset, bool constrainPitch = true);
		void handleRightMouseMotion(float xoffset, float yoffset, const glm::vec3& targetPos, bool constrainPitch = true);
		void handleScroll(float yoffset, GLFWwindow* window, const glm::vec3& targetPos);

	private:
		//glm::quat lookAt(glm::vec3 camPos, glm::vec3 target);
		void rotate(float dpitch, float dyaw, bool constrainPitch);
	};
}