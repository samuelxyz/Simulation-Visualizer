#include <core/stdafx.h>

#include "graphics/Camera.h"

#include "Entity.h" 
#include "graphics/content/VisualBox.h"
#include "graphics/content/VisualSphere.h"
#include "core/Definitions.h"

namespace entity {

	Entity::Entity(std::string entityName, glm::vec3 position, glm::quat orientation,
		glm::vec3 velocity, glm::vec3 angVel,
		float mass, glm::mat3 rotInertia, std::string typeName)
		: entityName(entityName), position(position), orientation(orientation),
		velocity(velocity), angVelocity(angVel),
		mass(mass), rotInertia(rotInertia), typeName(typeName),
		shouldShow { false, false, false, false, false }
	{
	}

	Entity::~Entity()
	{
	}

	glm::vec3 Entity::toLocalFrame(glm::vec3 worldVec) const
	{
		//return glm::inverse(glm::toMat3(orientation)) * (worldVec - position);
		return glm::transpose(glm::toMat3(orientation)) * (worldVec - position);
		// inverse == transpose for rotation matrices
	}

	glm::vec3 Entity::toWorldFrame(glm::vec3 localVec) const
	{
		//return glm::rotate(orientation, localVec) + position;
		return glm::toMat3(orientation) * localVec + position;
	}

	void Entity::renderShadow(graphics::Renderer & renderer, const glm::vec3& cameraPos) const
	{
		float baseRadius = getShadowRadius();
		float height = position.z - core::calcShadowZ(cameraPos);

		if (height < -baseRadius)
			return;

		float scaleFactor;
		if (height > 0.0f)
			scaleFactor = std::max(1.0f, 0.5f * height/baseRadius + 0.5f);
		else
			scaleFactor = std::cos(height/baseRadius * core::PI);

		//static float randomness = 0.0f; // avoid shadow z-fighting
		//randomness += core::SMALL_DISTANCE;
		//if (randomness > core::SMALL_DISTANCE * 10)
		//	randomness = 0.0f;

		glm::vec4 transparent(0.0f);
		glm::vec4 centerColor = graphics::COLOR_BLACK;
		// shadow darkness
		if (height > 0.0f)
			//centerColor.a = std::min(0.8f*baseRadius*baseRadius/(height*height+1), 1.0f);
			//centerColor.a = std::min(baseRadius/(0.4f*height), std::abs(1.0f - 0.2f*height/baseRadius));
			centerColor.a = std::max(0.0f, 1.0f - 0.2f*height/baseRadius);
		else
			centerColor.a = scaleFactor;
		glm::vec3 center(position.x, position.y, core::calcShadowZ(cameraPos)/* + randomness*/);
		glm::vec3 radius(baseRadius*scaleFactor, 0.0f, 0.0f); // penumbra gets bigger with distance

		graphics::CenteredPoly shadow;
		shadow.emplace_back(centerColor, center);
		for (float angle = 0.0f; angle < 6.3f; angle += core::QUARTER_PI/2.0f)
		{
			shadow.emplace_back(transparent, center + glm::rotateZ(radius, angle));
		}

		renderer.submit(shadow);
	}

	void Entity::renderGUI()
	{
		static glm::vec3 linImpulse(0.0f);
		static glm::vec3 angImpulse(0.0f);

		if (ImGui::Begin((typeName + ": " + entityName).c_str()))
		{
			ImGui::Checkbox("Show label", &shouldShow.label);

			ImGui::Separator();

			ImGui::Text("Mass: %.3f kg", mass);
			ImGui::Text("Inertia matrix (kg*m^2):");
			ImGui::Text("diag(%.3f, %.3f, %.3f)", 
				rotInertia[0][0], rotInertia[1][1], rotInertia[2][2]);

			ImGui::Separator();
			{
				ImGui::Text("Position:");
				//ImGui::SameLine();
				ImGui::InputFloat3("m", &position.x);
				ImGui::Checkbox("Show marker##Position", &(shouldShow.positionMarker));
			}
			ImGui::Separator();
			{
				ImGui::Text("Velocity: %.3f m/s", glm::length(velocity)); 

				ImGui::InputFloat3("##InputVelocity", &velocity.x); 
				ImGui::SameLine();
				if (ImGui::Button("Zero##ZeroVelocityButton"))
					velocity = glm::vec3(0.0f);

				ImGui::PushStyleColor(ImGuiCol_CheckMark, graphics::COLOR_VELOCITY);
				ImGui::Checkbox("Show vector##Velocity", &(shouldShow.velocityVector));
				ImGui::PopStyleColor();
			}
			ImGui::Separator();
			{
				ImGui::Text("Angular velocity: %.3f rad/s", glm::length(angVelocity));

				ImGui::InputFloat3("##InputAngVelocity", &angVelocity.x);
				ImGui::SameLine();
				if (ImGui::Button("Zero##ZeroRotationButton"))
					angVelocity = glm::vec3(0.0f);
				
				ImGui::PushStyleColor(ImGuiCol_CheckMark, graphics::COLOR_ANGVEL);
				ImGui::Checkbox("Show vector##Angular", &(shouldShow.angVelocityVector));
				ImGui::PopStyleColor();
			}
			ImGui::Separator();
			{
				ImGui::Text("Apply impulse:");
				ImGui::InputFloat3("##linImpulseInput", &linImpulse.x);
				ImGui::SameLine();
				if (ImGui::ArrowButton("##linImpulseButton", ImGuiDir_Right))
					applyLinearImpulse(linImpulse);
			}
			{
				ImGui::Text("Apply angular impulse:");
				ImGui::InputFloat3("##angImpulseInput", &angImpulse.x);
				ImGui::SameLine();
				if (ImGui::ArrowButton("##angImpulse", ImGuiDir_Right))
					applyAngularImpulse(angImpulse);
			}
		}
		ImGui::End();
	}

	void Entity::renderLabel(const graphics::Camera& camera) const
	{
		camera.renderLabel(position, false, typeName + entityName + "NameLabel",
			typeName + ": " + entityName, graphics::COLOR_NONE, graphics::PIVOT_CENTER);
	}

	void Entity::renderPositionMarker(graphics::Renderer& renderer, const graphics::Camera& camera) const
	{
		static constexpr float radius = 0.04f;

		graphics::VisualSphere sphere(position, core::QUAT_IDENTITY, radius);

		sphere.render(renderer);

		camera.renderLabel(position, true, typeName + entityName + "PositionMarkerLabel",
			fmt::sprintf("(%.3f, %.3f, %.3f)", position.x, position.y, position.z), 
			graphics::COLOR_NONE);
	}

	void Entity::renderVelocityVector(graphics::Renderer& renderer, const graphics::Camera& camera) const
	{
		static constexpr float thickness = 0.01f;

		 graphics::VisualBox box(position, velocity,
			thickness, core::VECTOR_UP,
			graphics::VisualEntity::Style::SOLID_COLOR, graphics::COLOR_VELOCITY);

		box.render(renderer);

		camera.renderLabel(position + velocity, true, typeName + entityName + "VelocityVectorLabel",
			fmt::sprintf("(%.3f, %.3f, %.3f)", velocity.x, velocity.y, velocity.z), 
			graphics::COLOR_VELOCITY);
	}

	void Entity::renderAngularVelocity(graphics::Renderer& renderer, const graphics::Camera& camera) const
	{
		static constexpr float thickness = 0.01f;

		 graphics::VisualBox box(position, angVelocity,
			thickness, core::VECTOR_UP,
			graphics::VisualEntity::Style::SOLID_COLOR, graphics::COLOR_ANGVEL);

		box.render(renderer);

		camera.renderLabel(position + angVelocity, true, typeName + entityName + "AngVelVectorLabel",
			fmt::sprintf("(%.3f, %.3f, %.3f)", angVelocity.x, angVelocity.y, angVelocity.z), 
			graphics::COLOR_ANGVEL);
	}

	// freefall
	void Entity::update()
	{
		position.x += velocity.x * core::TIME_STEP;
		position.y += velocity.y * core::TIME_STEP;
		position.z += velocity.z * core::TIME_STEP - 0.5f * core::GRAVITY * core::TIME_STEP * core::TIME_STEP;

		velocity.z -= core::GRAVITY * core::TIME_STEP;

		if (std::isnormal(glm::length2(angVelocity)))
			orientation = glm::normalize(
				glm::angleAxis(
					glm::length(angVelocity) * core::TIME_STEP, 
					glm::normalize(angVelocity)
				) * orientation
			);
	}

	void Entity::loadState(core::Timestep& timestep)
	{
		position = timestep.position;
		orientation = timestep.orientation;
		velocity = timestep.velocity;
		angVelocity = timestep.angVelocity;
	}

	void Entity::applyLinearImpulse(glm::vec3 impulse)
	{
		velocity += impulse/mass;
	}

	void Entity::applyAngularImpulse(glm::vec3 impulse)
	{
		glm::vec3 delta_omega = toWorldFrame(glm::inverse(rotInertia) * toLocalFrame(impulse));

		angVelocity += delta_omega;
	}

	void Entity::applyWrenchImpulse(glm::vec3 worldPos, glm::vec3 impulse)
	{
		applyLinearImpulse(impulse);
		applyAngularImpulse(glm::cross(worldPos - position, impulse));
	}

}