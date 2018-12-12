#include "core/stdafx.h"

#include <imgui/imgui.h>

#include "GLFW/glfw3.h"

#include "graphics/Camera.h"
#include "graphics/Definitions.h"

#include <iostream>

namespace graphics {

	//Camera::Camera(glm::vec3 position, glm::quat orientation)
	//	: position(position), orientation(orientation), 
	//	fov(glm::radians(45.0f))
	//{
	//}

	//Camera::Camera(glm::vec3 position, glm::vec3 target)
	//	: position(position), orientation(lookAt(position, target)), 
	//	fov(glm::radians(45.0f))
	//{
	//}

	Camera::Camera(glm::vec3 position, float pitch, float yaw)
		: position(position), pitch(pitch), yaw(yaw), fov(glm::radians(45.0f)),
		showAxes(false)
	{
	}

	glm::mat4 Camera::getVPMatrix(int windowWidth, int windowHeight) const
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

	void Camera::renderGUI()
	{
		ImGui::Begin("Camera");

		ImGui::Text("Position: (%.3f, %.3f, %.3f)", position.x, position.y, position.z);
		ImGui::Text("Pitch: %.3f, Yaw: %.3f", pitch, yaw);
		ImGui::SliderFloat("FOV", &fov, fovMin, fovMax);
		ImGui::Checkbox("Show Axes", &showAxes);

		ImGui::End();
	}

	void Camera::pollKeys(GLFWwindow* window)
	{
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
		//glm::vec3 euler = glm::eulerAngles(orientation);
		//std::cout << "(" << position.x << ", " << position.y << ", " << position.z << 
		//	"), (" << euler.x << ", " << euler.y << ", " << euler.z << ")" << std::endl;

		//std::cout << "(" << position.x << ", " << position.y << ", " << position.z <<
		//	")\tPitch: " << pitch << ", yaw: " << yaw << std::endl;

		//if (action == GLFW_RELEASE)
		//	return;

		// QUATERNION METHOD
		//glm::mat3 orient = glm::toMat3(orientation);

		//if (key == GLFW_KEY_W)
		//	position += orient * glm::vec3(movementSpeed, 0.0f, 0.0f);
		//if (key == GLFW_KEY_S)
		//	position += orient * glm::vec3(-movementSpeed, 0.0f, 0.0f);
		//if (key == GLFW_KEY_A)
		//	position += orient * glm::vec3(0.0f, 0.0f, -movementSpeed);
		//if (key == GLFW_KEY_D)
		//	position += orient * glm::vec3(1.0f, 0.0f,  movementSpeed);
		//if (key == GLFW_KEY_R || key == GLFW_KEY_SPACE)
		//	position += glm::vec3(0.0f,  movementSpeed, 0.0f);
		//if (key == GLFW_KEY_F || key == GLFW_KEY_LEFT_SHIFT)
		//	position += glm::vec3(0.0f, -movementSpeed, 0.0f);

		// CURRENT METHOD
		//glm::vec3 forward(std::cos(yaw), 0.0f, -std::sin(yaw));
		//glm::vec3 right(std::sin(yaw), 0.0f, std::cos(yaw));

		//if (key == GLFW_KEY_W)
		//	position += forward * movementSpeed;
		//if (key == GLFW_KEY_S)
		//	position += forward * -movementSpeed;
		//if (key == GLFW_KEY_A)
		//	position += right * -movementSpeed;
		//if (key == GLFW_KEY_D)
		//	position += right * movementSpeed;
		//if (key == GLFW_KEY_R || key == GLFW_KEY_SPACE)
		//	position += glm::vec3(0.0f, movementSpeed, 0.0f);
		//if (key == GLFW_KEY_F || key == GLFW_KEY_LEFT_SHIFT)
		//	position += glm::vec3(0.0f, -movementSpeed, 0.0f);
	}

	void Camera::handleMouseMotion(float xoffset, float yoffset, bool constrainPitch)
	{
		float sensitivity = mouseSensitivity * (fov / core::QUARTER_PI);

		xoffset *= sensitivity /* * std::cos(pitch) */; // at high pitches it's less sensitive
		yoffset *= sensitivity;

		//// avoid flipping
		//if (constrainPitch)
		//{
		//	float pitch = glm::pitch(orientation);

		//	if (pitch + yoffset > glm::radians(89.0f))
		//		yoffset = glm::radians(89.0f) - pitch;
		//	else if (pitch + yoffset < glm::radians(-89.0f))
		//		yoffset = glm::radians(-89.0f) - pitch;
		//}

		////orientation = glm::normalize(glm::quat(glm::vec3(yoffset, xoffset, 0.0f)) * orientation);
		//glm::quat pitcher = glm::angleAxis(yoffset, glm::toMat3(orientation) * glm::vec3(0.0f, 0.0f, 1.0f));
		//glm::quat yawer = glm::angleAxis(xoffset, glm::vec3(0.0f, -1.0f, 0.0f));

		//orientation = glm::normalize(yawer * (pitcher * orientation));

		//float roll = glm::roll(orientation);
		//if (roll != 0.0f)
		//{
		//	glm::quat rollCorrection(glm::vec3(0.0f, 0.0f, -roll));
		//	orientation = rollCorrection * orientation;
		//}

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

	//glm::quat Camera::lookAt(glm::vec3 camPos, glm::vec3 target)
	//{
	//	glm::vec3 toTarget = glm::normalize(target - camPos);
	//	glm::vec3 axis = glm::cross(glm::vec3(1.0f, 0.0f, 0.0f), toTarget);
	//	float angle = std::acos(glm::dot(glm::vec3(1.0f, 0.0f, 0.0f), toTarget));
	//	glm::quat rotateToTarget = glm::angleAxis(angle, axis);

	//	// todo: rotate to face upward?

	//	return rotateToTarget;
	//}
}