#include "core/stdafx.h"
#include "core/Simulation.h"

#include "graphics/Camera.h"
#include "graphics/content/VisualSphere.h"
#include "core/PathSim.h"
#include "entity/Entity.h"

namespace core {

	Simulation::Simulation()
		: pathSim(nullptr), entities(), environment(), parameters(), timeline()
	{
	}

	Simulation::Environment::Environment()
		: floor()
	{
		// floor
		glm::vec4 colorGray(0.8f, 0.8f, 0.8f, 1.0f);
		floor.push_back(graphics::ColoredVertex { graphics::COLOR_WHITE, glm::vec3( 0.0f,  0.0f, -50.0f) });
		floor.push_back(graphics::ColoredVertex { colorGray , glm::vec3(-5.0f, -5.0f, -50.0f) });
		floor.push_back(graphics::ColoredVertex { colorGray , glm::vec3(-5.0f,  5.0f, -50.0f) });
		floor.push_back(graphics::ColoredVertex { colorGray , glm::vec3( 5.0f,  5.0f, -50.0f) });
		floor.push_back(graphics::ColoredVertex { colorGray , glm::vec3( 5.0f, -5.0f, -50.0f) });
	}

	Simulation::Parameters::Parameters()
		: timePaused(true), playbackTime(0.0f), currentStep(0),
		showShadows(true), showEnvironment(true), showContactPoint(true),
		playbackSpeed(1.0), loopPlayback(false)
	{
	}

	int Simulation::addSteps(core::Timeline& timeline, int numSteps)
	{
		// start from current state, replacing any existing version of events
		timeline.erase(timeline.begin() + parameters.currentStep, timeline.end());
		recordTimestep(pathSim->target);

		int successes = 0;
		bool usePath = false; // start with freefall since it's fast to compute
		bool lastPathSuccessful = true;
		bool lastFreefallSuccessful = true;
		while (successes < numSteps)
		{
			if (usePath)
			{
				// try a path step
				pathSim->target->loadState(timeline.back());
				pathSim->captureTargetState();
				pathSim->printZ();
				pathSim->printCache();
				if (pathSim->addStep(timeline))
				{
					lastPathSuccessful = true;
					++successes;
				}
				else
				{
					// path not successful
					lastPathSuccessful = false;
					if (lastFreefallSuccessful)
						usePath = false; // switch to freefall
					else
						return successes; // neither method is working
				}
			}
			else
			{
				// try a freefall step
				if (addFreefallStep(pathSim->target))
				{
					lastFreefallSuccessful = true;
					++successes;
				}
				else
				{
					// freefall not successful
					lastFreefallSuccessful = false;
					if (lastPathSuccessful)
						usePath = true; // switch to path
					else
						return successes; // neither method is working
				}
			}
		}
		return successes;
	}

	int Simulation::addStepsUntilEnd(core::Timeline & timeline)
	{
		static constexpr int max = 1000;

		// start from current state, replacing any existing version of events
		timeline.erase(timeline.begin() + parameters.currentStep, timeline.end());
		recordTimestep(pathSim->target);

		int successes = 0;
		bool usePath = false; // start with freefall since it's fast to compute
		bool lastPathSuccessful = true;
		bool lastFreefallSuccessful = true;

		while (true)
		{
			if (usePath)
			{
				// try a path step
				pathSim->target->loadState(timeline.back());
				pathSim->captureTargetState();
				pathSim->printZ();
				pathSim->printCache();
				if (pathSim->addStep(timeline))
				{
					lastPathSuccessful = true;
					++successes;
				}
				else
				{
					// path not successful
					lastPathSuccessful = false;
					if (lastFreefallSuccessful)
						usePath = false; // switch to freefall
					else
						return successes; // neither method is working
				}
			}
			else
			{
				// try a freefall step
				if (addFreefallStep(pathSim->target))
				{
					lastFreefallSuccessful = true;
					++successes;
				}
				else
				{
					// freefall not successful
					lastFreefallSuccessful = false;
					if (lastPathSuccessful)
						usePath = true; // switch to path
					else
						return successes; // neither method is working
				}
			}

			if (glm::length2(timeline.back().velocity) < 1e-6f &&
				glm::length2(timeline.back().angVelocity) < 1e-6f)
				return successes; // everything stopped, we're done

			if (successes > max)
				return successes;
		}
		return successes;
	}

	// Does not record contact point
	void Simulation::recordTimestep(entity::Entity* e)
	{
		timeline.emplace_back(
			e->getPosition(),
			e->getOrientation(),
			e->getVelocity(),
			e->getAngVelocity(),
			e->getPosition()
		);
	}

	// Starts from end of timeline
	// Adds result to end of timeline and returns true if successful
	// Updates target entity to end of timeline
	bool Simulation::addFreefallStep(entity::Entity* e)
	{
		e->loadState(timeline.back());
		recordTimestep(e);

		e->update();
		if (e->intersectsFloor())
		{
			// Not gonna work, undo
			e->loadState(timeline.back());
			timeline.pop_back();
			return false;
		}
		else
		{
			// it's good
			timeline.pop_back();
			recordTimestep(e);
			return true;
		}
	}

	Simulation::~Simulation()
	{
		// deleting nullptr does nothing so that's fine
		for (entity::Entity* e : entities)
			delete e;

		delete pathSim;
	}

	void Simulation::update()
	{
		static auto prevTime = std::chrono::steady_clock::now();
		static int prevStep = parameters.currentStep;
		if (!parameters.timePaused)
		{
			// step time forward by however much the last frame corresponds to
			auto now = std::chrono::steady_clock::now();
			auto diff = std::chrono::duration_cast<std::chrono::microseconds>(now - prevTime).count();
			float timeDiff = diff*1e-6f*parameters.playbackSpeed;

			// last calculated time in the timeline. Used for clamping, below
			float backTime = timeline.backTime();

			// if the current step and current time don't match
			if (std::copysignf(1.0f, parameters.playbackSpeed) * (parameters.playbackTime / core::TIME_STEP - parameters.currentStep) >= 1.0)
			{
				// Check to see if it's just because we reached the end of the timeline
				if (parameters.playbackTime < 0.0f)
				{
					if (parameters.loopPlayback)
						parameters.playbackTime = std::fmod(parameters.playbackTime,
							backTime + core::TIME_STEP) + backTime + core::TIME_STEP;
					else
						parameters.playbackTime = 0.0f;
				}
				if (parameters.playbackTime >= backTime + core::TIME_STEP)
				{
					if (parameters.loopPlayback)
						parameters.playbackTime = std::fmod(parameters.playbackTime,
							backTime + core::TIME_STEP);
					else
						parameters.playbackTime = backTime;
				}

				// User must have changed the currentStep slider through ImGui. 
				// Update playbackTime accordingly:
				parameters.playbackTime = parameters.currentStep * core::TIME_STEP;
			}

			parameters.playbackTime += timeDiff; // advance time

			// clamp
			if (parameters.playbackTime < 0.0f)
			{
				if (parameters.loopPlayback)
					parameters.playbackTime = std::fmod(parameters.playbackTime, 
						backTime + core::TIME_STEP) + backTime + core::TIME_STEP;
				else
					parameters.playbackTime = 0.0f;
			}
			if (parameters.playbackTime >= backTime + core::TIME_STEP)
			{
				if (parameters.loopPlayback)
					parameters.playbackTime = std::fmod(parameters.playbackTime, 
						backTime + core::TIME_STEP);
				else
					parameters.playbackTime = backTime;
			}

			// Now find the step number
			int step = static_cast<int>(parameters.playbackTime/core::TIME_STEP);

			// Clamping again just in case. It do be like that
			step = std::max(step, 0);
			step = std::min(step, static_cast<int>(timeline.size())-1);

			// do it
			parameters.currentStep = step;
		}
		if (prevStep != parameters.currentStep)
			pathSim->target->loadState(timeline[parameters.currentStep]);
		prevStep = parameters.currentStep;
		prevTime = std::chrono::steady_clock::now();
	}

	void Simulation::render(graphics::Renderer& renderer, const glm::vec3& cameraPos) const
	{
		if (cameraPos.z > graphics::FLOOR_Z)
		{
			if (parameters.showEnvironment)
				renderEnvironment(renderer);
			renderEntities(renderer, cameraPos);
		}
		else // reverse the rendering order
		{
			renderEntities(renderer, cameraPos);
			if (parameters.showEnvironment)
				renderEnvironment(renderer);
		}


	}

	void Simulation::renderGUI(const graphics::Camera& camera)
	{
		// FPS calculations
		static auto previous = std::chrono::steady_clock::now();

		static unsigned int frameCount = 0u;
		frameCount++;

		static float fps = 0.0;
		if (frameCount >= FPS_TRACKER_SMOOTHING)
		{
			auto now = std::chrono::steady_clock::now();
			auto diff = std::chrono::duration_cast<std::chrono::microseconds>(now - previous).count();
			previous = std::chrono::steady_clock::now();

			fps = 1.0e6f*frameCount / diff;
			frameCount = 0u;
		}

		static int calculateStepsNum = 100;

		if (ImGui::Begin("Simulation"))
		{
			// Parameter checkboxes
			{
				//ImGui::Checkbox("Enable Gravity", &(parameters.gravityEnabled));
				ImGui::Checkbox("Pause Time", &(parameters.timePaused));
				ImGui::Checkbox("Loop Playback", &(parameters.loopPlayback));
			}
			// Playback controls
			{
				if (ImGui::Button("Once"))
				{
					parameters.timePaused = false;
					parameters.loopPlayback = false;
					parameters.playbackSpeed = 1.0f;
					parameters.currentStep = 0;
				}
				ImGui::SameLine();
				if (ImGui::Button("Loop"))
				{
					parameters.timePaused = false;
					parameters.loopPlayback = true;
					parameters.playbackSpeed = 1.0f;
					parameters.currentStep = 0;
				}
				ImGui::SameLine();
				if (ImGui::Button("Reset"))
				{
					parameters.timePaused = true;
					parameters.currentStep = 0;
				}
				ImGui::Text("Playback Speed:");
				ImGui::InputFloat("##PlaybackSpeedNum", &(parameters.playbackSpeed));
				if (ImGui::IsItemHovered())
				{
					static constexpr float factor = 0.1f;
					float scr = ImGui::GetIO().MouseWheel;
					parameters.playbackSpeed += scr * factor;
				}
				ImGui::Text("Elapsed Time: %.3f s", timeline.timeOf(parameters.currentStep));
			}
			// Timestep slider
			{

				ImGui::Text("Current Timestep:");
				std::string numStepsString("%d / ");
				numStepsString += std::to_string(timeline.size()-1);
				ImGui::BeginGroup();
				ImGui::SliderInt("##CurrentTimestepNum", &(parameters.currentStep), 0, 
					static_cast<int>(timeline.size())-1, numStepsString.c_str());

				// Fine adjustment buttons
				ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
				static const float buttonSize = ImGui::GetFrameHeight();
				static const ImVec2 buttonSizeV = ImVec2(buttonSize, buttonSize);
				ImGui::PushButtonRepeat(true);
				if (ImGui::Button("-##Timestep", buttonSizeV) &&
					parameters.currentStep > 0)
					--parameters.currentStep;
				ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
				if (ImGui::Button("+##Timestep", buttonSizeV) &&
					parameters.currentStep < static_cast<int>(timeline.size()) - 1)
					++parameters.currentStep;
				ImGui::PopButtonRepeat();
				ImGui::EndGroup();

				// Can use mouse wheel to scroll timeline
				if (ImGui::IsItemHovered())
				{
					int scr = static_cast<int>(ImGui::GetIO().MouseWheel);
					parameters.currentStep -= scr;
					if (scr != 0)
						parameters.timePaused = true;
					
					// now clamp
					if (parameters.currentStep >= static_cast<int>(timeline.size()))
						parameters.currentStep = timeline.size() - 1;
					else if (parameters.currentStep < 0)
						parameters.currentStep = 0;
				}
			}
			// Timestep calculation control
			{
				ImGui::Text("Calculate Steps:");
				ImGui::InputInt("##CalculateStepsNum", &calculateStepsNum);
				if (ImGui::IsItemHovered())
					calculateStepsNum += static_cast<int>(ImGui::GetIO().MouseWheel);
				if (calculateStepsNum < 0)
					calculateStepsNum = 0;
				ImGui::SameLine();
				if (ImGui::ArrowButton("##CalculateStepsButton", ImGuiDir_Right))
				{
					addSteps(timeline, calculateStepsNum);
					parameters.timePaused = false;
					parameters.loopPlayback = false;
				}
				if (ImGui::Button("Calculate Until Stop"))
				{
					addStepsUntilEnd(timeline);
					parameters.timePaused = false;
					parameters.loopPlayback = false;
				}
			}
			{
				ImGui::Separator();
				ImGui::Text("FPS: %.0f", fps);
			}
			// Visual controls
			ImGui::Separator();
			{
				ImGui::Checkbox("Show Environment", &(parameters.showEnvironment));

				ImGui::Indent(); // showShadows should be greyed out if !showEnvironment
				{
					if (!parameters.showEnvironment)
					{
						ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
						ImGui::PushStyleColor(ImGuiCol_CheckMark, ImGui::GetStyle().Colors[ImGuiCol_FrameBg]);
						ImGui::InvisibleButton("##ShowShadowsClickInterceptor", ImVec2(110.0f, ImGui::GetFrameHeight()));
						ImGui::SameLine(ImGui::GetStyle().IndentSpacing +
							ImGui::GetStyle().WindowPadding.x, 0.0f);
					}

					ImGui::Checkbox("Show Shadows", &(parameters.showShadows));

					if (!parameters.showEnvironment)
						ImGui::PopStyleColor(2);
				}
				ImGui::Unindent();
				
				ImGui::PushStyleColor(ImGuiCol_CheckMark, graphics::COLOR_CONTACT);
				ImGui::Checkbox("Show Contact Point", &(parameters.showContactPoint));
				ImGui::PopStyleColor();
			}
			// Entity checkboxes
			ImGui::Separator();
			{
				ImGui::Text("Show Entity Information:");
				{
					ImGui::BeginChild("Entity Scroll Pane");
					for (entity::Entity* e : entities)
					{
						ImGui::Checkbox(e->getName().c_str(), &(e->shouldShow.gui));
					}
					ImGui::EndChild();
				}
			}
		}
		ImGui::End();

		for (entity::Entity* e : entities)
		{
			if (e->shouldShow.gui)
				e->renderGUI();
			if(e->shouldShow.label)
				e->renderLabel(camera);
		}

		const entity::Entity* hovered = getHoveredEntity(camera);
		if (hovered != nullptr && !(hovered->shouldShow.label))
			hovered->renderLabel(camera);
	}

	void Simulation::renderGUIOverlay(graphics::Renderer& renderer, const graphics::Camera& camera) const
	{
		for (entity::Entity* e : entities)
		{
			if (e->shouldShow.velocityVector)
				e->renderVelocityVector(renderer, camera);
			if (e->shouldShow.angVelocityVector)
				e->renderAngularVelocity(renderer, camera);
			if (e->shouldShow.positionMarker)
				e->renderPositionMarker(renderer, camera);
		}

		if (parameters.showContactPoint)
			renderContactPoint(renderer, camera);
	}

	void Simulation::add(entity::Entity* e)
	{
		entities.push_back(e);

		if (timeline.empty())
		{
			// initialize timeline
			recordTimestep(e);
		}
	}

	void Simulation::setTarget(entity::Entity* e)
	{
		// Add to this->entities if not already present
		if (std::find(entities.begin(), entities.end(), e) == entities.end())
			add(e);

		pathSim = e->createPathSim();
		pathSim->setTarget(e);
	}

	const entity::Entity* const Simulation::getFocusedEntity(const graphics::Camera& camera) const
	{
		// ideally this would maybe have something to do with the mouse position but this is fine for now

		const entity::Entity* target = getHoveredEntity(camera);
		if (target != nullptr)
			return target;

		// this is modified from Camera::toScreenSpace()
		for (entity::Entity* e : entities)
		{
			glm::vec4 pos4 = glm::vec4(e->getPosition(), 1.0f);
			glm::vec4 screenPos = camera.getVPMatrix(true) * pos4;
			screenPos.z += 0.18f; // idk why but without this it's a bit off

			float screenX = (screenPos.x + screenPos.z) / screenPos.z;
			float screenY = (-screenPos.y + screenPos.z) / screenPos.z;

			if (screenPos.z > 0.0f && 0.0f <= screenX && screenX <= 2.0f && 0.0f <= screenY && screenY <= 2.0f)
			{
				if (target == nullptr)
				{
					target = e;
				}
				else
				{
					if (glm::length2(e->getPosition() - camera.getPosition()) <
						glm::length2(target->getPosition() - camera.getPosition()))
						target = e;
				}
			}
		}

		return target;
	}

	// ray-scan entities using mouse position ray thingy
	const entity::Entity* const Simulation::getHoveredEntity(const graphics::Camera & camera) const
	{
		glm::vec3 mouseRay = camera.getMouseRay();

		std::vector<float> srSq; // cached shadow radii of each entity (used for cr
		float rMaxSq = 0.0f;
		for (const entity::Entity* e : entities)
		{
			float sr = e->getShadowRadius();
			srSq.push_back(sr*sr);

			float rSq = glm::length2(e->getPosition() - camera.getPosition());
			if (rSq > rMaxSq)
				rMaxSq = rSq;
		}

		for (float r = 0.0f; r < rMaxSq; r += 0.05f)
		{
			glm::vec3 testPos = camera.getPosition() + r*mouseRay;
			for (unsigned int i = 0; i < entities.size(); ++i)
				if (glm::length2(entities[i]->getPosition() - testPos) < srSq[i])
					if (entities[i]->containsPoint(testPos))
						return entities[i];
		}

		return nullptr;
	}

	void Simulation::renderEntities(graphics::Renderer& renderer, const glm::vec3& cameraPos) const
	{
		for (entity::Entity* e : entities)
		{
			e->render(renderer);
			if (parameters.showShadows && parameters.showEnvironment)
				e->renderShadow(renderer, cameraPos);
		}
	}

	void Simulation::renderEnvironment(graphics::Renderer& renderer) const
	{
		// floor
		//renderer.submit(environment.floor);
		glm::vec4 squareColor;
		float floorHalfwidth = 10.0f;
		float floorCheckerSize = 1.0f;
		for (float i = -floorHalfwidth; i < floorHalfwidth; i += floorCheckerSize)
		{
			for (float j = -floorHalfwidth; j < floorHalfwidth; j += floorCheckerSize)
			{
				if (std::abs(std::fmodf(i+j, 2.0f)) < 1e-3)
					squareColor = graphics::COLOR_WHITE;
				else
					squareColor = glm::vec4(0.6f, 0.6f, 0.6f, 0.5f); // gray

				squareColor.a = 0.5f;

				renderer.submit(graphics::Quad {{
					{ squareColor, glm::vec3(i, j, graphics::FLOOR_Z) },
					{ squareColor, glm::vec3(i, j+floorCheckerSize, graphics::FLOOR_Z) },
					{ squareColor, glm::vec3(i+floorCheckerSize, j+floorCheckerSize, graphics::FLOOR_Z) },
					{ squareColor, glm::vec3(i+floorCheckerSize, j,graphics::FLOOR_Z) }
				}});
			}
		}
	}

	void Simulation::renderContactPoint(graphics::Renderer& renderer, const graphics::Camera& camera) const
	{
		const Timestep& now = timeline[parameters.currentStep];

		if (!now.hasContactPoint())
			return;

		graphics::VisualSphere sphere(now.contactPoint, 
			core::QUAT_IDENTITY, graphics::MARKER_DOT_RADIUS, graphics::VisualEntity::Style::SOLID_COLOR, 
			graphics::COLOR_CONTACT);

		sphere.render(renderer);

		camera.renderLabel(now.contactPoint, true, "PathSimContactPointLabel", 
			fmt::sprintf("(%.3f, %.3f, %.3f)", now.contactPoint.x, now.contactPoint.y, now.contactPoint.z), 
			graphics::COLOR_CONTACT);
	}


}