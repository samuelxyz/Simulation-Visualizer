#pragma once

#include <glm/gtx/quaternion.hpp>

#include "core/Definitions.h"

namespace graphics {

	class Camera
	{
	private:
		// Camera Attributes
		glm::vec3 position;
		float pitch, yaw; // radians, yaw is from +x toward -z (counterclockwise for player)
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
		Camera(float windowWidth, float windowHeight, 
			glm::vec3 position = glm::vec3(0.0f), float pitch = 0.0f, float yaw = 0.0f);

		glm::mat4 getVPMatrix() const;
		glm::vec3 getPosition() const;
		glm::vec3 getLookVec() const;
		glm::vec2 toScreenSpace(const glm::vec3& worldPos) const;
		bool shouldShowAxes() const { return showAxes; }

		void renderGUI();

		void updateWindowDims(int width, int height);
		void pollKeys(GLFWwindow* window);
		void handleKey(int key, int action);
		void handleMouseMotion(float xoffset, float yoffset, bool constrainPitch = true);
		void handleScroll(float yoffset);

	private:
		//glm::quat lookAt(glm::vec3 camPos, glm::vec3 target);
	};
}