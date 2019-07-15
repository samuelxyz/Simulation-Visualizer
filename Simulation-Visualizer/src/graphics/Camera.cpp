#include "core/stdafx.h"

#include "graphics/Camera.h"
#include "core/Simulation.h"
#include "entity/Entity.h"
#include "graphics/Window.h"

namespace graphics {

	Camera::Camera(int windowWidth, int windowHeight,
		glm::vec3 position, float pitch, float yaw)
		: windowWidth(windowWidth), windowHeight(windowHeight),
		position(position), pitch(pitch), yaw(yaw), fov(glm::radians(45.0f)),
		showAxes(false)
	{
	}

	glm::mat4& Camera::getVPMatrix(bool useCached) const
	{
		static glm::mat4 cached;

		if (!useCached)
		{
			glm::mat4 projection = glm::perspective(fov, static_cast<float>(windowWidth)/windowHeight, 0.1f, graphics::RENDER_DISTANCE);
			glm::mat4 view = glm::lookAt(position, position + getLookVec(), core::VECTOR_UP);

			cached = projection * view;
		}

		return cached;
	}

	glm::vec3 Camera::getPosition() const
	{
		return position;
	}

	glm::vec3 Camera::getLookVec() const
	{
		return glm::vec3(std::cos(yaw)*std::cos(pitch), std::sin(yaw)*std::cos(pitch), std::sin(pitch));
	}

	glm::vec2 Camera::toScreenSpace(const glm::vec3& worldPos) const
	{
		glm::vec4 pos4 = glm::vec4(worldPos, 1.0f);
		glm::vec4 screenPos = getVPMatrix() * pos4;

		screenPos.z += 0.18f; // idk why but without this it's a bit off

		float screenX = (screenPos.x + screenPos.z) * (windowWidth/(2*screenPos.z));
		float screenY = (-screenPos.y + screenPos.z) * (windowHeight/(2*screenPos.z));

		return glm::vec2(screenX, screenY);
	}

	glm::vec3 Camera::getMouseRay() const
	{
		glm::vec2 screenPos = ImGui::GetMousePos();

		// let screenPos.z = 1.0f
		float x = 2*screenPos.x / windowWidth - 1.0f;
		float y = 1.0f - 2*screenPos.y / windowHeight;

		// 1.002f is another magic number that idk why it works lol
		glm::vec4 scrPos4(x, y, 1.002f, 1.0f);
		glm::vec4 ray = glm::inverse(getVPMatrix()) * scrPos4;

		return glm::normalize(glm::vec3(ray.x, ray.y, ray.z));
	}

	void Camera::renderGUI()
	{
		if (ImGui::Begin("Camera"))
		{
			ImGui::Text("Position:");
			ImGui::InputFloat3("##InputPositionSlider", &(position.x));
			ImGui::Text("Pitch: %+.3f (%.1f deg)", pitch, glm::degrees(pitch));
			ImGui::Text("Yaw:   %+.3f (%.1f deg)", yaw, glm::degrees(yaw));
			ImGui::Separator();
			{
				ImGui::Text("FOV:");
				ImGui::SameLine();
				std::string fovStr("%.3f ");
				std::string fovDeg = fmt::sprintf("(%+05.1f deg)", glm::degrees(fov));
				fovStr += std::string(fovDeg);
				ImGui::SliderFloat("##FOV", &fov, fovMin, fovMax, fovStr.c_str() );
			}

			ImGui::Separator();

			ImGui::Checkbox("Show Axes", &showAxes);
			//if (showAxes)
			//{
			//	ImGui::SameLine();
			//	ImVec4 rectColor(0.0f, 0.0f, 0.0f, 1.0f);
			//	ImGui::PushStyleColor(ImGuiCol_Button, rectColor);
			//	ImGui::PushStyleColor(ImGuiCol_ButtonActive, rectColor);
			//	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, rectColor);
			//	ImGui::Button("   ##LabelXYZBackgroundRect");
			//	ImGui::PopStyleColor(3);

			//	ImGui::SameLine(105.0f);
			//	ImGui::TextColored(graphics::COLOR_RED, "x");

			//	ImGui::SameLine(0.0f, 1.0f);
			//	ImGui::TextColored(ImVec4(0.2f, 0.7f, 0.2f, 1.0f), "y");

			//	ImGui::SameLine(0.0f, 1.0f);
			//	ImGui::TextColored(ImVec4(0.2f, 0.2f, 1.0f, 1.0f), "z");

			//}
		}
		ImGui::End();
	}

	void Camera::renderLabel(const glm::vec3& worldPos, 
		bool backgroundOnHoverOnly, const std::string& titleID,
		const std::string& text, const glm::vec4& textColor, 
		const ImVec2& pivot) const
	{
		ImVec2 screenPos = toScreenSpace(worldPos);
		if (screenPos.y < ImGui::GetFrameHeight())
			return;

		ImGui::SetNextWindowPos(screenPos, 0, pivot);

		if (backgroundOnHoverOnly)
		{
			ImGui::Begin((std::string("##") + titleID).c_str(), nullptr,
				ImGuiWindowFlags_NoDecoration |
				ImGuiWindowFlags_NoNav |
				ImGuiWindowFlags_AlwaysAutoResize |
				ImGuiWindowFlags_NoBackground);
		}
		else
		{
			ImGui::Begin((std::string("##") + titleID).c_str(), nullptr,
				ImGuiWindowFlags_NoDecoration |
				ImGuiWindowFlags_NoInputs |
				ImGuiWindowFlags_AlwaysAutoResize |
				ImGuiWindowFlags_NoBackground);
		}

		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, ImGui::GetStyle().WindowRounding);

		if (backgroundOnHoverOnly)
			ImGui::PushStyleColor(ImGuiCol_Button, COLOR_NONE);
		else
			ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetColorU32(ImGuiCol_WindowBg));

		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetColorU32(ImGuiCol_WindowBg));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetColorU32(ImGuiCol_WindowBg));

		if (textColor != COLOR_DEFAULT)
			ImGui::PushStyleColor(ImGuiCol_Text, textColor);

		ImGui::Button(text.c_str());

		ImGui::PopStyleVar();

		if (textColor != COLOR_DEFAULT)
			ImGui::PopStyleColor(4);
		else
			ImGui::PopStyleColor(3);

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

		glm::vec3 forward(std::cos(yaw), std::sin(yaw), 0.0f);
		glm::vec3 right(std::sin(yaw), -std::cos(yaw), 0.0f);

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
			position += speed * core::VECTOR_UP;
		if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
			position += -speed * core::VECTOR_UP;
	}

	void Camera::handleKey(int key, int action)
	{
	}

	void Camera::handleLeftMouseMotion(float xoffset, float yoffset, bool constrainPitch)
	{
		float sensitivity = mouseSensitivity * (fov / core::QUARTER_PI);

		xoffset *= sensitivity /* * std::cos(pitch) */; // at high pitches it's less sensitive
		yoffset *= sensitivity;

		rotate(-yoffset, xoffset, constrainPitch);
	}

	void Camera::handleRightMouseMotion(float xoffset, float yoffset, const entity::Entity* const focusedEntity, bool constrainPitch)
	{
		// Tricky tricky. For the right effect, we want to orbit around the target when considering yaw.
		// However, when considering pitch we want to orbit around the center of the screen.

		glm::vec3 targetPos;
		if (focusedEntity == nullptr)
			targetPos = getPosition() + getLookVec();
		else
			targetPos = focusedEntity->getPosition();

		if (glm::length2(targetPos - position) < 1e-6f)
			return;

		float sensitivity = 2 * mouseSensitivity * (fov / core::QUARTER_PI);

		xoffset *= sensitivity /* * std::cos(pitch) */; // at high pitches it's less sensitive
		yoffset *= sensitivity; 

		// handle pitch: orbit around "center" of screen
		{
			// find rotation center - same z as target, but along camera yaw heading
			glm::vec3 toTarget(targetPos - position);
			glm::vec2 toTargetPlanar(toTarget.x, toTarget.y);
			glm::vec2 cameraForwardPlanar(std::cos(yaw), std::sin(yaw));
			// the projection of toTargetPlanar onto cameraForwardPlanar
			glm::vec2 toCenterPlanar = cameraForwardPlanar * (glm::dot(cameraForwardPlanar, toTargetPlanar));

			// from center to camera
			glm::vec3 radius(-toCenterPlanar, position.z - targetPos.z);

			glm::vec2 planarRadius(radius.x, radius.y);
			float pr = glm::length(planarRadius);

			float orbitPitch;
			if (pr == 0.0f)
			{
				if (radius.z > 0)
					orbitPitch = glm::radians(90.0f);
				else
					orbitPitch = glm::radians(-90.0f);
			}
			else
			{
				orbitPitch = std::atan2f(radius.z, pr);
			}

			float orbitYaw;
			if (pr == 0.0f)
				orbitYaw = 0.0f;
			else
				orbitYaw = std::atan2f(planarRadius.y, planarRadius.x);

			float newOrbitPitch = orbitPitch - yoffset;
			if (constrainPitch)
			{
				if (newOrbitPitch > glm::radians(89.0f))
					newOrbitPitch = glm::radians(89.0f);
				else if (newOrbitPitch < glm::radians(-89.0f))
					newOrbitPitch = glm::radians(-89.0f);
			}

			glm::vec3 newRadius(
				std::cos(newOrbitPitch) * std::cos(orbitYaw),
				std::cos(newOrbitPitch) * std::sin(orbitYaw),
				std::sin(newOrbitPitch)
			);
			newRadius *= glm::length(radius);

			glm::vec3 dx = newRadius - radius;
			position += dx;

			rotate(orbitPitch - newOrbitPitch, 0.0f, constrainPitch);
		}

		// handle yaw: orbit around object
		{
			glm::vec3 radius = position - targetPos;

			glm::vec2 planarRadius(radius.x, radius.y);
			float pr = glm::length(planarRadius);

			float orbitYaw;
			if (pr == 0.0f)
				orbitYaw = 0.0f;
			else
				orbitYaw = std::atan2f(planarRadius.y, planarRadius.x);
			float newOrbitYaw = orbitYaw - xoffset;

			glm::vec2 newPlanarRadius(std::cos(newOrbitYaw), std::sin(newOrbitYaw));
			newPlanarRadius *= pr;

			glm::vec2 dx = newPlanarRadius - planarRadius;
			position += glm::vec3(dx, 0.0f);

			rotate(0.0f, -xoffset, false);
		}
	}

	void Camera::handleScroll(float yoffset, GLFWwindow* window, const entity::Entity* const focusedEntity)
	{
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) 
		{ 
			glm::vec3 targetPos;
			if (focusedEntity == nullptr)
				targetPos = getPosition() + getLookVec();
			else
				targetPos = focusedEntity->getPosition();

			// move in and out from target
			constexpr static float stepSize = 4.0f;

			glm::vec3 toTarget = targetPos - position;
			
			float radius = glm::length(toTarget);

			if (radius < 1e-6f)
				return;

			if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
				yoffset *= 5.0f;
			float newRadius = radius - scrollSensitivity * stepSize * yoffset;

			if (newRadius < 1e-1f)
				newRadius = 1e-1f;

			toTarget *= newRadius / radius;

			position = targetPos - toTarget;
		}
		else if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) // ctrl+scroll changes field of view
		{
			fov -= scrollSensitivity * yoffset;

			if (fov < fovMin)
				fov = fovMin;
			if (fov > fovMax)
				fov = fovMax;
		}
		else // plain scrolling scrolls the simulation timeline
		{
			Window::getSimulation(window)->scrollTimeline(static_cast<int>(yoffset));
		}
	}

	void Camera::rotate(float dpitch, float dyaw, bool constrainPitch)
	{
		pitch += dpitch;
		yaw += dyaw;

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
}