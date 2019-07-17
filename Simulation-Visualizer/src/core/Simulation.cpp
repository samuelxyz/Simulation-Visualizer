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

	Simulation::~Simulation()
	{
		// deleting nullptr does nothing so that's fine
		for (entity::Entity* e : entities)
			delete e;

		delete pathSim;
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
		playbackSpeed(1.0), loopPlayback(false), logOutput(true)
	{
	}

	int Simulation::addSteps(core::Timeline& timeline, int numStepsToAdd)
	{
		START_TIMING;

		// start from current state, replacing any existing version of events
		timeline.erase(timeline.begin() + parameters.currentStep, timeline.end());
		recordTimestep(pathSim->target);

		int successes = 0;
		//bool usePath = false; // start with freefall since it's fast to compute
		bool usePath = true; // maybe this will help things behave themselves
		bool pathStreaking = false;
		bool pathMightWork = true;
		bool freefallMightWork = true;

		while (successes < numStepsToAdd)
		{
			if (usePath)
			{
				// try a path step
				pathSim->target->loadState(timeline.back());
				pathSim->captureTargetState(!pathStreaking); // if we're in a streak then no need to update guesses

				if (parameters.logOutput)
				{
					pathSim->printZ();
					pathSim->printCache();
				}

				if (pathSim->addStep(timeline, parameters.logOutput))
				{
					pathStreaking = true;

					// Alright this step is finished! Next step, anything might happen
					pathMightWork = true;
					freefallMightWork = true;

					if (parameters.logOutput)
						std::cout << "PATH step success" << std::endl;
					++successes;
				}
				else
				{
					// path not successful
					pathStreaking = false; // there's gonna be a gap in PATH streak so new guesses next time
					pathMightWork = false; // for next loop
					if (freefallMightWork)
					{
						if (parameters.logOutput)
							std::cout << "PATH failed, switching to freefall" << std::endl;
						usePath = false; // switch to freefall
					}
					else
					{
						if (parameters.logOutput)
							std::cout << "PATH failed, freefall failed previously, stopping calculations" <<
								"\nCalculation time: " << STOP_TIMING_AND_GET_MICROSECONDS / 1e6f << " s" << std::endl;
						return successes; // neither method is working
					}
				}
			}
			else
			{
				// try a freefall step
				if (addFreefallStep(pathSim->target))
				{
					// Alright this step is finished! Next step, anything might happen
					pathMightWork = true;
					freefallMightWork = true;

					if (parameters.logOutput)
						std::cout << "Freefall step success" << std::endl;
					++successes;
				}
				else
				{
					// freefall not successful
					freefallMightWork = false; // for next loop
					if (pathMightWork)
					{
						if (parameters.logOutput)
							std::cout << "Freefall step failed, switching to PATH" << std::endl;
						usePath = true; // switch to path
					}
					else
					{
						if (parameters.logOutput)
							std::cout << "Freefall step failed, PATH failed previously, stopping calculations" <<
								"\nCalculation time: " << STOP_TIMING_AND_GET_MICROSECONDS / 1e6f << " s" << std::endl;
						return successes; // neither method is working
					}
				}
			}
		}
		if (parameters.logOutput)
			std::cout << "Finished calculations" <<
				"\nCalculation time: " << STOP_TIMING_AND_GET_MICROSECONDS / 1e6f << " s" << std::endl;
		return successes;
	}

	int Simulation::addStepsUntilEnd(core::Timeline & timeline)
	{
		static constexpr int max = 1000;

		START_TIMING;

		// start from current state, replacing any existing version of events
		timeline.erase(timeline.begin() + parameters.currentStep, timeline.end());
		recordTimestep(pathSim->target);

		int successes = 0;
		//bool usePath = false; // start with freefall since it's fast to compute
		bool usePath = true; // maybe this will help things behave themselves
		bool pathStreaking = false;
		bool pathMightWork = true;
		bool freefallMightWork = true;

		while (true)
		{
			if (usePath)
			{
				// try a path step
				pathSim->target->loadState(timeline.back());
				pathSim->captureTargetState(!pathStreaking); // if we're in a streak then no need to update guesses

				if (parameters.logOutput)
				{
					pathSim->printZ();
					pathSim->printCache();
				}

				if (pathSim->addStep(timeline, parameters.logOutput))
				{
					pathStreaking = true;

					// Alright this step is finished! Next step, anything might happen
					pathMightWork = true;
					freefallMightWork = true;

					if (parameters.logOutput)
						std::cout << "PATH step success" << std::endl;
					++successes;
				}
				else
				{
					// path not successful
					pathStreaking = false; // there's gonna be a gap in PATH streak so new guesses next time
					pathMightWork = false; // for next loop
					if (freefallMightWork)
					{
						if (parameters.logOutput)
							std::cout << "PATH failed, switching to freefall" << std::endl;
						usePath = false; // switch to freefall
					}
					else
					{
						if (parameters.logOutput)
							std::cout << "PATH failed, freefall failed previously, stopping calculations" <<
								"\nCalculation time: " << STOP_TIMING_AND_GET_MICROSECONDS / 1e6f << " s" << std::endl;
						return successes; // neither method is working
					}
				}
			}
			else
			{
				// try a freefall step
				if (addFreefallStep(pathSim->target))
				{
					// Alright this step is finished! Next step, anything might happen
					pathMightWork = true;
					freefallMightWork = true;

					if (parameters.logOutput)
						std::cout << "Freefall step success" << std::endl;
					++successes;
				}
				else
				{
					// freefall not successful
					freefallMightWork = false; // for next loop
					if (pathMightWork)
					{
						if (parameters.logOutput)
							std::cout << "Freefall step failed, switching to PATH" << std::endl;
						usePath = true; // switch to path
					}
					else
					{
						if (parameters.logOutput)
							std::cout << "Freefall step failed, PATH failed previously, stopping calculations" <<
								"\nCalculation time: " << STOP_TIMING_AND_GET_MICROSECONDS / 1e6f << " s" << std::endl;
						return successes; // neither method is working
					}
				}
			}

			if (successes > 5 && // 5 is arbitrary but probably pretty good for the purpose
				glm::length2(timeline.back().velocity) < 1e-6f &&
				glm::length2(timeline.back().angVelocity) < 1e-6f)
			{
				if (parameters.logOutput)
					std::cout << "Motion seems to have ended, stopping calculations" <<
						"\nCalculation time: " << STOP_TIMING_AND_GET_MICROSECONDS / 1e6f << " s" << std::endl;
				return successes; // everything stopped, we're done
			}

			if (successes > max)
			{
				if (parameters.logOutput)
					std::cout << "Maximum step limit reached, stopping calculations" <<
						"\nCalculation time: " << STOP_TIMING_AND_GET_MICROSECONDS / 1e6f << " s" << std::endl;
				return successes;
			}
		}
		if (parameters.logOutput)
			std::cout << "I don't even know how you got to this point, it's after a while(true) with no breaks" <<
				"\nAnyway, calculation time: " << STOP_TIMING_AND_GET_MICROSECONDS / 1e6f << " s" << std::endl;
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

	void Simulation::scrollTimeline(int scr)
	{
		parameters.currentStep -= scr;
		if (scr != 0)
			parameters.timePaused = true;

		// now clamp
		if (parameters.currentStep >= static_cast<int>(timeline.size()))
			parameters.currentStep = timeline.size() - 1;
		else if (parameters.currentStep < 0)
			parameters.currentStep = 0;
	}

	void Simulation::startStop()
	{
		if (!parameters.loopPlayback && parameters.playbackSpeed > 0 && parameters.currentStep == timeline.size() - 1)
		{
			parameters.timePaused = false;
			parameters.currentStep = 0;
		}
		else if (!parameters.loopPlayback && parameters.playbackSpeed < 0 && parameters.currentStep == 0)
		{
			parameters.timePaused = false;
			parameters.currentStep = timeline.size() - 1;
		}
		else
			parameters.timePaused = !parameters.timePaused;
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

				// User must have changed the currentStep slider through ImGui or scrolling. 
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
			// Timestep calculation control
			{
				ImGui::Text("Calculate Steps:");
				ImGui::InputInt("##CalculateStepsNum", &calculateStepsNum);
				if (ImGui::IsItemHovered())
				{
					calculateStepsNum += static_cast<int>(ImGui::GetIO().MouseWheel);
					ImGui::SetTooltip("Scroll to adjust");
				}
				if (calculateStepsNum < 0)
					calculateStepsNum = 0;
				ImGui::SameLine();
				if (ImGui::ArrowButton("##CalculateStepsButton", ImGuiDir_Right))
				{
					addSteps(timeline, calculateStepsNum);
					parameters.timePaused = false;
					parameters.loopPlayback = false;
				}
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Calculate");
				if (ImGui::Button("Calculate Until Stop"))
				{
					addStepsUntilEnd(timeline);
					parameters.timePaused = false;
					parameters.loopPlayback = false;
				}
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Maximum 1000 steps");
				ImGui::Checkbox("Log Output", &(parameters.logOutput));
			}

			ImGui::Separator();
			// Parameter checkboxes
			{
				//ImGui::Checkbox("Enable Gravity", &(parameters.gravityEnabled));
				ImGui::Checkbox("Pause Playback", &(parameters.timePaused));
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
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Go to beginning of timeline");
				ImGui::Text("Playback Speed:");
				ImGui::InputFloat("##PlaybackSpeedNum", &(parameters.playbackSpeed));
				if (ImGui::IsItemHovered())
				{
					static constexpr float factor = 0.1f;
					float scr = ImGui::GetIO().MouseWheel;
					parameters.playbackSpeed += scr * factor;
					ImGui::SetTooltip("Scroll to adjust");
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
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Scroll to move through timeline");

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
					scrollTimeline(scr);
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

					ImGui::PushStyleColor(ImGuiCol_CheckMark, graphics::COLOR_BLACK);
					ImGui::Checkbox("Show Shadows", &(parameters.showShadows));
					ImGui::PopStyleColor();

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
						ImGui::SameLine();
						if (pathSim->target == e)
							ImGui::TextColored(graphics::COLOR_WHITE, "(Active)");
						else
						{
							if (ImGui::ArrowButton((std::string("##setActive") + e->getName()).c_str(), ImGuiDir_Right))
							{
								setTarget(e, false);
								for (entity::Entity* ee : entities)
									ee->shouldShow.body = (ee == e); // make everything else invisible
							}
							if (ImGui::IsItemHovered())
								ImGui::SetTooltip("Set Active");
						}
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
	}

	void Simulation::setTarget(entity::Entity* e, bool autoAdd)
	{
		// Add to this->entities if not already present
		if (autoAdd && std::find(entities.begin(), entities.end(), e) == entities.end())
			add(e);

		if (pathSim != nullptr)
			delete pathSim;

		pathSim = e->createPathSim();
		pathSim->setTarget(e);
		timeline.clear();
			recordTimestep(e);
	}

	const entity::Entity* const Simulation::getFocusedEntity(const graphics::Camera& camera) const
	{
		const entity::Entity* target = getHoveredEntity(camera);
		if (target != nullptr)
			return target;

		// this is modified from Camera::toScreenSpace()
		for (entity::Entity* e : entities)
		{
			if (!e->shouldShow.body)
				continue;

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

		// Algorithm: "Restricted II"
		// For each entity, checks for proximity, then "hitscans" outward 
		// in both directions from where the entity "center" is along mouse ray
		// Then finds the surface of the object closest to the camera

			const entity::Entity* closestE = nullptr;
			float rClosest = std::numeric_limits<float>::max();
			//float rClosest = graphics::RENDER_DISTANCE;
			for (const entity::Entity* e : entities)
			{
				// must be visible
				if (!(e->shouldShow.body))
					continue;

				float rCenter = glm::length(e->getPosition() - camera.getPosition());
				float sr = e->getOuterBoundingRadius();
				float step = e->getInnerBoundingRadius()/30.0f;

				if (rCenter - sr > rClosest)
					continue; // closer entity already found

				if (glm::length2(camera.getPosition() + rCenter*mouseRay - e->getPosition()) > sr*sr)
					continue; // we're nowhere near e

				// start from center location and spread forward/backward to check for hits
				bool toggle = true;
				for (float dr = step; dr < sr; dr += (toggle)? step : 0.0f)
				{
					float r = (toggle) ? rCenter + dr : rCenter - dr;
					if (e->containsPoint(camera.getPosition() + r*mouseRay))
					{
						// now definitely hovering, but where's the front of the object?
						// zigzag back and forth to find the edge

						//step *= 10.0f;
						//
						//r -= step;
						//while (e->containsPoint(camera.getPosition() + r*mouseRay))
						//	r -= step;
						//
						//step /= 4.0f;
						//r += step;
						//while (!(e->containsPoint(camera.getPosition() + r*mouseRay)))
						//	r += step;
						//
						//step /= 4.0f;
						//r -= step;
						//while (e->containsPoint(camera.getPosition() + r*mouseRay))
						//	r -= step;
						//
						//step /= 4.0f;
						//r += step;
						//while (!(e->containsPoint(camera.getPosition() + r*mouseRay)))
						//	r += step;

						// condensed version of the above:
						step *= 40.0f;
						bool searchAway = false;
						for (int i = 0; i < 4; ++i)
						{
							step /= 4.0f;
							r += (searchAway)? step : -step;
							while (searchAway != (e->containsPoint(camera.getPosition() + r*mouseRay)))
								r += (searchAway) ? step : -step;
							searchAway = !searchAway;
						}
						
						// and see if it's the closest point on any entity thus far
						r -= step;
						if (r < rClosest)
						{
							rClosest = r;
							closestE = e;
						}
						break;
					}
					toggle = !toggle;
				}
			}

			return closestE;

	}

	void Simulation::renderEntities(graphics::Renderer& renderer, const glm::vec3& cameraPos) const
	{
		for (entity::Entity* e : entities)
		{
			if (e->shouldShow.body)
			{
				e->render(renderer);
				if (parameters.showShadows && parameters.showEnvironment && e->shouldShow.shadow)
					e->renderShadow(renderer, cameraPos);
			}
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