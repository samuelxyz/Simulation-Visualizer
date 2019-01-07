#include "core/stdafx.h"

#include <imgui/imgui.h>

#include "GLFW/glfw3.h"

#include "graphics/Camera.h"
#include "graphics/Definitions.h"

#include <iostream>

namespace graphics {

	Camera::Camera(float windowWidth, float windowHeight,
		glm::vec3 position, float pitch, float yaw)
		: windowWidth(windowWidth), windowHeight(windowHeight),
		position(position), pitch(pitch), yaw(yaw), fov(glm::radians(45.0f)),
		showAxes(false)
	{
	}

	glm::mat4 Camera::getVPMatrix() const
	{
		glm::mat4 projection = glm::perspective(fov, static_cast<float>(windowWidth)/windowHeight, 0.1f, 100.0f);
		glm::mat4 view = glm::lookAt(position, position + getLookVec(), glm::vec3(0.0f, 1.0f, 0.0f));

		return projection * view;
	}

	glm::vec3 Camera::getPosition() const
	{
		return position;
	}

	glm::vec3 Camera::getLookVec() const
	{
		return glm::vec3(std::cos(yaw)*std::cos(pitch), std::sin(pitch), -std::sin(yaw)*std::cos(pitch));
	}

	glm::vec2 Camera::toScreenSpace(const glm::vec3& worldPos) const
	{
		glm::vec4 pos4 = glm::vec4(worldPos, 1.0f);
		glm::vec4 screenPos = getVPMatrix() * pos4;

		float screenX = (screenPos.x + screenPos.z) * (windowWidth/(2*screenPos.z));
		float screenY = (-screenPos.y + screenPos.z) * (windowHeight/(2*screenPos.z));

		return glm::vec2(screenX, screenY);
	}

	void Camera::renderGUI()
	{
		if (ImGui::Begin("Camera"))
		{
			ImGui::Text("Position: (%.3f, %.3f, %.3f)", position.x, position.y, position.z);
			ImGui::Text("Pitch: %.3f (%.1f deg), Yaw: %.3f (%.1f deg)", 
				pitch, glm::degrees(pitch), yaw, glm::degrees(yaw));

			{
				ImGui::SliderFloat("##FOV", &fov, fovMin, fovMax, "FOV: %.3f");
				ImGui::SameLine();
				ImGui::Text("(%.1f deg)", glm::degrees(fov));
			}

			ImGui::Checkbox("Show Axes", &showAxes);
		}
		ImGui::End();
	}

	void Camera::updateWindowDims(int width, int height)
	{
		windowWidth = width;
		windowHeight = height;
	}

	void Camera::pollKeys(GLFWwindow* window)
	{
		if (ImGui::GetIO().WantCaptureKeyboard)
			return;

		glm::vec3 forward(std::cos(yaw), 0.0f, -std::sin(yaw));
		glm::vec3 right(std::sin(yaw), 0.0f, std::cos(yaw));

		float speed = movementSpeed;
		if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
			speed *= ctrlSpeedModifier;

		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			position += forward * speed;
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			position += forward * -speed;
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			position += right * -speed;
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			position += right * speed;
		if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
			position += glm::vec3(0.0f, speed, 0.0f);
		if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
			position += glm::vec3(0.0f, -speed, 0.0f);
	}

	void Camera::handleKey(int key, int action)
	{
	}

	void Camera::handleMouseMotion(float xoffset, float yoffset, bool constrainPitch)
	{
		float sensitivity = mouseSensitivity * (fov / core::QUARTER_PI);

		xoffset *= sensitivity /* * std::cos(pitch) */; // at high pitches it's less sensitive
		yoffset *= sensitivity;

		pitch -= yoffset;
		yaw += xoffset;
		
		if (yaw > core::PI)
			yaw -= core::TWO_PI;
		else if (yaw <= -core::PI)
			yaw += core::TWO_PI;

		if (constrainPitch)
		{
			if (pitch > glm::radians(89.0f))
				pitch = glm::radians(89.0f);
			else if (pitch < glm::radians(-89.0f))
				pitch = glm::radians(-89.0f);
		}
	}

	void Camera::handleScroll(float yoffset)
	{
		fov -= scrollSensitivity * yoffset;

		if (fov < fovMin)
			fov = fovMin;
		if (fov > fovMax)
			fov = fovMax;
	}
}