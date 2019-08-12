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
		shouldShow { true, false, false, false, false, false, true, false },
		guiVars { 0.0f, glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.0f) },
		dragHandles 
	{ 
		io::DragHandle(*this, glm::vec3(0.0f, 0.0f, 0.0f), graphics::COLOR_WHITE),
		io::DragHandle(*this, glm::vec3(1.0f, 0.0f, 0.0f), graphics::COLOR_GUI_RED),
		io::DragHandle(*this, glm::vec3(0.0f, 1.0f, 0.0f), graphics::COLOR_GUI_GREEN),
		io::DragHandle(*this, glm::vec3(0.0f, 0.0f, 1.0f), graphics::COLOR_GUI_BLUE)
	}
	{
	}

	Entity::~Entity()
	{
	}

	glm::vec3 Entity::toLocalFrame(glm::vec3 worldVec, bool useCachedOrientation) const
	{
		static glm::mat3 matrix = glm::transpose(glm::toMat3(orientation));
		if (!useCachedOrientation)
			matrix = glm::transpose(glm::toMat3(orientation));

		return matrix * (worldVec - position);
	}

	glm::vec3 Entity::toWorldFrame(glm::vec3 localVec, bool useCachedOrientation) const
	{
		static glm::mat3 matrix = glm::toMat3(orientation);
		if (!useCachedOrientation)
			matrix = glm::toMat3(orientation);

		return matrix * localVec + position;
	}

	void Entity::renderShadow(graphics::Renderer & renderer,
		float shadowSizeMultiplier, const glm::vec3& refPoint) const
	{
		float baseRadius = shadowSizeMultiplier;
		float height = refPoint.z - graphics::FLOOR_Z;

		if (height < -baseRadius)
			return;

		float scaleFactor;
		if (height > 0.0f)
			scaleFactor = std::max(1.0f, 0.5f * height/baseRadius + 0.5f);
		else
			scaleFactor = std::cos(height/baseRadius * core::PI);

		float centerA;
		// shadow darkness
		if (height > 0.0f)
			//centerColor.a = std::min(0.8f*baseRadius*baseRadius/(height*height+1), 1.0f);
			//centerColor.a = std::min(baseRadius/(0.4f*height), std::abs(1.0f - 0.2f*height/baseRadius));
			centerA = std::max(0.0f, 1.0f - 0.2f*height/baseRadius);
		else
			centerA = scaleFactor;
		float radius = baseRadius*scaleFactor; // penumbra gets bigger with distance

		renderShadow(renderer, glm::vec2(refPoint.x, refPoint.y), radius*1.05f, centerA*0.25f);
		renderShadow(renderer, glm::vec2(refPoint.x, refPoint.y), radius*0.95f, centerA*0.75f);
	}

	void Entity::renderShadow(graphics::Renderer& renderer, 
		const glm::vec2 & centerPos, float radius, float centerA) const
	{
		glm::vec4 transparent(0.0f);
		glm::vec4 centerColor = graphics::COLOR_BLACK;
		centerColor.a = centerA;
		glm::vec3 center(centerPos.x, centerPos.y, graphics::FLOOR_Z);
		glm::vec3 radiusVec(radius, 0.0f, 0.0f);
		graphics::CenteredPoly shadow;
		shadow.emplace_back(centerColor, center);
		for (float angle = 0.0f; angle < 6.3f; angle += core::QUARTER_PI/2.0f)
		{
			shadow.emplace_back(transparent, center + glm::rotateZ(radiusVec, angle));
		}
		renderer.submit(shadow);
	}

	void Entity::renderGUI()
	{
		if (ImGui::Begin(getFullName().c_str(), &(shouldShow.gui)))
		{
			ImGui::Checkbox("Show entity", &shouldShow.body);
			ImGui::Checkbox("Show Shadow", &shouldShow.shadow);
			ImGui::Checkbox("Always show label", &shouldShow.label);

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
				ImGui::Checkbox("Show drag handles", &(shouldShow.dragHandles));
			}
			ImGui::Separator();
			{
				ImGui::Text("Velocity:"); ImGui::SameLine();
				ImGui::Text("%.3f m/s", glm::length(velocity));
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Magnitude");

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
				ImGui::Text("Angular velocity:"); ImGui::SameLine();
				ImGui::Text("%.3f rad/s", glm::length(angVelocity));
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Magnitude");

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
				ImGui::Text("Rotate:");
				ImGui::SameLine();
				ImGui::PushItemWidth(45.0f);
				ImGui::InputFloat("rad##RotateAngleInput", &guiVars.applyRotateAngle);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Rotation angle");
				ImGui::PopItemWidth();
				ImGui::InputFloat3("##RotateAxisInput", &guiVars.applyRotateAxis.x);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Rotation axis");
				ImGui::SameLine();
				if (ImGui::ArrowButton("##RotateButton", ImGuiDir_Right))
				{
					orientation = glm::normalize(
						glm::angleAxis(guiVars.applyRotateAngle, guiVars.applyRotateAxis) * orientation);
				}
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Apply");
			}
			{
				ImGui::Text("Apply impulse:");
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("kg*m/s");
				ImGui::InputFloat3("##linImpulseInput", &guiVars.applyLinImpulse.x);
				ImGui::SameLine();
				if (ImGui::ArrowButton("##linImpulseButton", ImGuiDir_Right))
					applyLinearImpulse(guiVars.applyLinImpulse);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Apply");
			}
			{
				ImGui::Text("Apply angular impulse:");
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("kg*m^2/s");
				ImGui::InputFloat3("##angImpulseInput", &guiVars.applyAngImpulse.x);
				ImGui::SameLine();
				if (ImGui::ArrowButton("##angImpulse", ImGuiDir_Right))
					applyAngularImpulse(guiVars.applyAngImpulse);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Apply");
			}
		}
		ImGui::End();
	}

	void Entity::renderLabel(const graphics::Camera& camera) const
	{
		camera.renderLabel(position, false, typeName + entityName + "NameLabel",
			typeName + ": " + entityName, graphics::COLOR_NONE, graphics::PIVOT_BOTTOM);
	}

	void Entity::renderPositionMarker(graphics::Renderer& renderer, const graphics::Camera& camera) const
	{
		renderer.renderMarkerDot(position);

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
		// d = v_o t + 1/2 a t^2

		velocity.z -= core::GRAVITY * core::TIME_STEP;
		// v = v_o + a t

		if (std::isnormal(glm::length2(angVelocity)))
			orientation = glm::normalize(
				glm::angleAxis(
					glm::length(angVelocity) * core::TIME_STEP, 
					glm::normalize(angVelocity)
				) * orientation
			);
	}

	bool Entity::intersectsFloor() const
	{
		return getLowestPointZ() <= core::FLOOR_Z;
	}

	void Entity::loadState(eqn::EntityETimestepData& data)
	{
		position = data.q;
		orientation = data.qu;
		velocity = data.v;
		angVelocity = data.w;
	}

	void Entity::applyDragHandleResult(const glm::vec3 & v)
	{
		displace(v);
	}

	glm::vec3 Entity::getDragHandlePosition() const
	{
		return getPosition();
	}

	bool Entity::isDragHandleVisible() const
	{
		return shouldShow.dragHandles;
	}

	void Entity::displace(const glm::vec3 & dx)
	{
		position += dx;
		float lowestZ = getLowestPointZ();
		if (lowestZ < core::FLOOR_Z)
			position.z += core::FLOOR_Z - lowestZ;
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

	bool Entity::handleRightMouseDrag(graphics::Camera & camera, const glm::vec2 & dx)
	{
		camera.orbit(getPosition(), dx.x, dx.y);
		return true;
	}

	void Entity::initializeDragHandles()
	{
		for (io::DragHandle& d : dragHandles)
			d.setSize(getInnerBoundingRadius());
	}



}