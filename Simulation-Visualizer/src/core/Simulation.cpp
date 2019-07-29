#include "core/stdafx.h"
#include "core/Simulation.h"

#include "graphics/Camera.h"
#include "graphics/content/VisualSphere.h"
#include "core/PathSim.h"
#include "entity/Box.h"
#include "entity/Cylinder.h"

namespace core {

	Simulation::Simulation()
		: pathSim(nullptr), entities(), environment(), 
		parameters(), timeline(), showAddEntityWindow(false)
	{
	}

	Simulation::~Simulation()
	{
		// deleting nullptr does nothing so that's fine
		for (entity::Entity* e : entities)
			delete e;
		for (entity::Entity* e : entitiesVisualOnly)
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
		playbackSpeed(1.0), loopPlayback(false), logOutput(true), logCalcTime(true)
	{
	}

	int Simulation::addSteps(core::Timeline& timeline, int numStepsToAdd, bool breakOnConstantMotion)
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

		[&]() { // if this lambda returns negative, indicates failure
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
							std::cout << "PATH step success\n";
						++successes;
					}
					else
					{
						// path not successful
						if (parameters.logOutput)
							std::cout << "PATH failed, trying again with new guess\n";
						pathSim->captureTargetState(true); // try again with fresh guess
						if (parameters.logOutput)
						{
							pathSim->printZ();
							pathSim->printCache();
						}

						if (pathSim->addStep(timeline, parameters.logOutput))
						{
							// success with new guess
							if (parameters.logOutput)
								std::cout << "Succeeded with new guess. Continuing\n";

							continue;
						}
						else
						{
							pathStreaking = false; // there's gonna be a gap in PATH streak so new guesses next time
							pathMightWork = false; // for next loop
							if (freefallMightWork)
							{
								if (parameters.logOutput)
									std::cout << "PATH still failed, switching to freefall\n";
								usePath = false; // switch to freefall
							}
							else
							{
								if (parameters.logOutput)
									std::cout << "PATH still failed, freefall failed previously, stopping calculations\n";
								return -successes; // neither method is working. Return negative
							}
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
							std::cout << "Freefall step success\n";
						++successes;
					}
					else
					{
						// freefall not successful
						freefallMightWork = false; // for next loop
						if (pathMightWork)
						{
							if (parameters.logOutput)
								std::cout << "Freefall step failed, switching to PATH\n";
							usePath = true; // switch to path
						}
						else
						{
							if (parameters.logOutput)
								std::cout << "Freefall step failed, PATH failed previously, stopping calculations\n";
							return -successes; // neither method is working. Return negative
						}
					}
				}

				static constexpr int stop = 20; // 20 is arbitrary but probably pretty good for the purpose
				if (breakOnConstantMotion && successes > stop)
				{
					int last = timeline.size() - 1;
					bool constant = true;
					for (int i = 0; i < stop; ++i)
					{
						if (glm::length2(timeline[last-i].velocity - timeline[last-1-i].velocity) > 1e-6f)
						{
							constant = false;
							break;
						}
						if (glm::length2(timeline[last-i].angVelocity - timeline[last-1-i].angVelocity) > 1e-6f)
						{
							constant = false;
							break;
						}
					}
					if (constant)
					{
						if (parameters.logOutput)
							std::cout << "Motion seems to have become constant, stopping calculations\n";
						return successes; // everything stopped, we're done. Return positive
					}
				}
			}
			
			// reached the end
			if (parameters.logOutput)
				std::cout << "Finished request of " << numStepsToAdd << " steps, stopping calculations\n";
			return successes;
		}(); // end lambda

		if (parameters.logCalcTime)
			std::cout << "Calculation time: " << STOP_TIMING_AND_GET_MICROSECONDS / 1e6f;
		if (parameters.logCalcTime || parameters.logOutput)
			std::cout << std::endl;
		return successes;
	}

	int Simulation::addStepsUntilEnd(core::Timeline & timeline)
	{
		static constexpr int max = 1000;

		return addSteps(timeline, max, true);
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
			{
				renderEnvironment(renderer);
				renderer.renderAndClearAll(true, true);

				if (parameters.showShadows)
				{
					renderShadows(renderer, cameraPos);
					renderer.renderAndClearAll(false);
				}
			}
			renderEntities(renderer);
			renderer.renderAndClearAll(true, true);
		}
		else // reverse the rendering order
		{
			renderEntities(renderer);
			renderer.renderAndClearAll(true, true);
			if (parameters.showEnvironment)
			{
				if (parameters.showShadows)
				{
					renderShadows(renderer, cameraPos);
					renderer.renderAndClearAll(false);
				}
				
				renderEnvironment(renderer); 
				renderer.renderAndClearAll(true, true);
			}
		}
	}

	void Simulation::renderGUI(graphics::Renderer& renderer, const graphics::Camera& camera)
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
					ImGui::SetTooltip("Calculate the specified number of timesteps");
				if (ImGui::Button("Auto Calculate"))
				{
					addStepsUntilEnd(timeline);
					parameters.timePaused = false;
					parameters.loopPlayback = false;
				}
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Calculate until the motion becomes constant\nMaximum 1000 steps");
				ImGui::Checkbox("Log Output", &(parameters.logOutput));
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Will cause calculation to take significantly longer");
				ImGui::Checkbox("Log Calculation Time", &(parameters.logCalcTime));
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
				ImGui::Text("Show Information For:");
				{
					ImGui::BeginChild("Entity Scroll Pane", ImVec2(0.0f, -26.0f));
					ImGui::Indent();
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
				//ImGui::Separator();
				if (ImGui::Button("New Entity"))
					showAddEntityWindow = true;
			}
		}
		ImGui::End();

		if (showAddEntityWindow)
			renderAddEntityGUI(renderer, camera);

		for (entity::Entity* e : entities)
		{
			if (e->shouldShow.gui)
				e->renderGUI();
			if(e->shouldShow.label)
				e->renderLabel(camera);
		}

		for (entity::Entity* e : entitiesVisualOnly)
			if (e->shouldShow.label)
				e->renderLabel(camera);

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

		for (io::DragHandle* d : dragHandles)
			if (d->target.isDragHandleVisible())
				d->render(renderer);

		if (parameters.showContactPoint)
			renderContactPoint(renderer, camera);
	}

	void Simulation::add(entity::Entity* e)
	{
		entities.push_back(e);

		for (io::DragHandle& d : e->getDragHandles())
			addDragHandle(&d);
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

	void Simulation::addDragHandle(io::DragHandle * d)
	{
		dragHandles.push_back(d);
	}

	entity::Entity* Simulation::getFocusedEntity(const graphics::Camera& camera)
	{
		entity::Entity* target = getHoveredEntity(camera);
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
	entity::Entity* Simulation::getHoveredEntity(const graphics::Camera & camera)
	{
		glm::vec3 mouseRay = camera.getMouseRay();

		// Algorithm: "Restricted II"
		// For each entity, checks for proximity, then "hitscans" outward 
		// in both directions from where the entity "center" is along mouse ray
		// Then finds the surface of the object closest to the camera

			entity::Entity* closestE = nullptr;
			float rClosest = std::numeric_limits<float>::max();
			//float rClosest = graphics::RENDER_DISTANCE;
			for (entity::Entity* e : entities)
			{
				// must be visible
				if (!(e->shouldShow.body))
					continue;

				float rCenter = glm::length(e->getPosition() - camera.getPosition());
				float br = e->getOuterBoundingRadius();
				float step = e->getInnerBoundingRadius()/30.0f;

				if (rCenter - br > rClosest)
					continue; // closer entity already found

				if (glm::length2(camera.getPosition() + rCenter*mouseRay - e->getPosition()) > br*br)
					continue; // we're nowhere near e

				// start from center location and spread forward/backward to check for hits
				bool toggle = true;
				for (float dr = step; dr < br; dr += (toggle)? step : 0.0f)
				{
					float r = (toggle) ? rCenter + dr : rCenter - dr;
					if (e->containsPoint(camera.getPosition() + r*mouseRay, dr != step))
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
							while (searchAway != (e->containsPoint(camera.getPosition() + r*mouseRay, true)))
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

	io::MouseDragTarget * Simulation::getLeftMouseDragTarget(const graphics::Camera & camera)
	{
		// the only possible candidates are drag handles
		
		return getHoveredDragHandle(camera);
	}

	void Simulation::renderEntities(graphics::Renderer& renderer) const
	{
		for (entity::Entity* e : entities)
			if (e->shouldShow.body)
				e->render(renderer);
		for (entity::Entity* e : entitiesVisualOnly)
			if (e->shouldShow.body)
				e->render(renderer);
	}

	void Simulation::renderShadows(graphics::Renderer& renderer, const glm::vec3& cameraPos) const
	{
		for (entity::Entity* e : entities)
			if (e->shouldShow.body && e->shouldShow.shadow)
				e->renderShadow(renderer);
		for (entity::Entity* e : entitiesVisualOnly)
			if (e->shouldShow.body && e->shouldShow.shadow)
				e->renderShadow(renderer);
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

		glm::vec4 color = (std::abs(now.contactPoint.z) > 1e-6f) ? 
			graphics::COLOR_NO_CONTACT : graphics::COLOR_CONTACT;

		renderer.renderMarkerDot(now.contactPoint, color);

		camera.renderLabel(now.contactPoint, true, "PathSimContactPointLabel", 
			fmt::sprintf("(%.3f, %.3f, %.3f)", now.contactPoint.x, now.contactPoint.y, now.contactPoint.z), 
			color);
	}

	void Simulation::renderAddEntityGUI(graphics::Renderer& renderer, const graphics::Camera& camera)
	{
		if (ImGui::Begin("Add Entity"))
		{
			ImGui::PushItemWidth(-15.0f);

			static entity::Entity* preview = nullptr;
			float makeNewPreview = false;

			static const char* typeNames[] = { "Box", "Cylinder" };
			static int selectedType = 0;
			ImGui::TextUnformatted("Select Entity Type:");
			static char nameBuf[64] = "NewBox";
			std::string defaultName("New");
			if (ImGui::Combo("##SelectType", &selectedType, typeNames, IM_ARRAYSIZE(typeNames)))
			{
				makeNewPreview = true;
				if (0 <= selectedType && selectedType <= IM_ARRAYSIZE(typeNames))
					defaultName += typeNames[selectedType];
				else
					defaultName += "Entity";

				strcpy_s<64>(nameBuf, "New");
				strcpy_s<64>(nameBuf, defaultName.c_str());
			}

			if (preview == nullptr && 0 <= selectedType && selectedType <= IM_ARRAYSIZE(typeNames))
			{
					makeNewPreview = true;
			}

			static glm::vec3 oldPreviewPos(0.0f);
			glm::vec3 previewPos = camera.getPosition() + camera.getLookVec() * 6.0f;

			if (previewPos != oldPreviewPos)
			{
				makeNewPreview = true;
				oldPreviewPos = previewPos;
			}


			ImGui::TextUnformatted("Entity Name:");
			ImGui::InputText("##InputName", nameBuf, 64);

			static float boxDims[] = { 1.0f, 1.0f, 1.0f };
			static float cylDims[] = { 1.0f, 1.0f };

			switch (selectedType)
			{
			case 0: // box
				ImGui::TextUnformatted("Length/Width/Height:");
				if (ImGui::InputFloat3("m##InputBoxDims", boxDims))
					makeNewPreview = true;
				break;
			case 1: // cylinder
				ImGui::TextUnformatted("Height/Radius:");
				if (ImGui::InputFloat2("m##InputCylDims", cylDims))
					makeNewPreview = true;
				break;
			}

			static float mass = 1.0f;
			ImGui::TextUnformatted("Mass:");
			ImGui::InputFloat("kg##InputMass", &mass);

			static float momIn[] = { 1.0f, 1.0f, 1.0f };
			ImGui::TextUnformatted("Principal moments");
			ImGui::TextUnformatted("of inertia (kg*m^2):");
			ImGui::InputFloat3("##InputInertia", momIn);

			// make & render preview
			if (makeNewPreview)
			{
				auto pos = std::find(entitiesVisualOnly.begin(), entitiesVisualOnly.end(), preview);
				if (pos != entitiesVisualOnly.end())
					entitiesVisualOnly.erase(pos, pos+1);
				//delete preview; // apparently already done by erase()
				glm::mat3 inertia = glm::identity<glm::mat3>();
				for (int i = 0; i < 3; ++i)
					inertia[i][i] = momIn[i];
				switch (selectedType)
				{
				case 0: // box
					if (previewPos.z - boxDims[2]/2 < core::FLOOR_Z)
						previewPos.z = core::FLOOR_Z + boxDims[2]/2;
					preview = new entity::Box(std::string(nameBuf), 
						previewPos, core::QUAT_IDENTITY, glm::vec3(0.0f), glm::vec3(0.0f), 
						mass, inertia, boxDims[0], boxDims[1], boxDims[2]
					);
					break;
				case 1: // cylinder
					if (previewPos.z - cylDims[0]/2 < core::FLOOR_Z)
						previewPos.z = core::FLOOR_Z + cylDims[0]/2;
					preview = new entity::Cylinder(std::string(nameBuf),
						previewPos, core::QUAT_IDENTITY, glm::vec3(0.0f), glm::vec3(0.0f),
						mass, inertia, cylDims[0], cylDims[1]
					);
					break;
				}
				preview->shouldShow.label = true;
				entitiesVisualOnly.push_back(preview);
			}

			ImGui::Separator();
			if (ImGui::Button("Cancel"))
			{
				showAddEntityWindow = false;
				auto pos = std::find(entitiesVisualOnly.begin(), entitiesVisualOnly.end(), preview);
				if (pos != entitiesVisualOnly.end())
					entitiesVisualOnly.erase(pos, pos+1);
			}
			ImGui::SameLine();
			if (ImGui::Button("OK"))
			{
				auto pos = std::find(entitiesVisualOnly.begin(), entitiesVisualOnly.end(), preview);
				if (pos != entitiesVisualOnly.end())
					entitiesVisualOnly.erase(pos, pos+1);
				preview->shouldShow.label = false; // restore to default
				add(preview);
				preview = nullptr;
				showAddEntityWindow = false;
			}
			ImGui::PopItemWidth();
			ImGui::End();
		}
	}

	io::DragHandle * Simulation::getHoveredDragHandle(const graphics::Camera & camera)
	{
		glm::vec3 mouseRay = camera.getMouseRay();

		// Algorithm: "Restricted II" (see getHoveredEntity())

		io::DragHandle* closest = nullptr;
		float rClosest = std::numeric_limits<float>::max();
		//float rClosest = graphics::RENDER_DISTANCE;
		for (io::DragHandle* d : dragHandles)
		{
			// must be visible
			if (!(d->target.isDragHandleVisible()))
				continue;

			float rCenter = glm::length(d->target.getDragHandlePosition() - camera.getPosition());
			float br = d->length + d->thickness;
			float step = (d->thickness)/30.0f;

			if (rCenter - br > rClosest)
				continue; // closer one already found

			if (glm::length2(camera.getPosition() + rCenter*mouseRay - d->target.getDragHandlePosition()) > br*br)
				continue; // we're nowhere near it

			// start from center location and spread forward/backward to check for hits
			bool toggle = true;
			for (float dr = step; dr < br; dr += (toggle) ? step : 0.0f)
			{
				float r = (toggle) ? rCenter + dr : rCenter - dr;
				if (d->containsPoint(camera.getPosition() + r*mouseRay, dr != step))
				{
					// now definitely hovering, but where's the front of the object?
					// zigzag back and forth to find the edge
					step *= 40.0f;
					bool searchAway = false;
					for (int i = 0; i < 4; ++i)
					{
						step /= 4.0f;
						r += (searchAway) ? step : -step;
						while (searchAway != (d->containsPoint(camera.getPosition() + r*mouseRay, true)))
							r += (searchAway) ? step : -step;
						searchAway = !searchAway;
					}

					// and see if it's the closest point on any entity thus far
					r -= step;
					if (r <= rClosest)
					{
						rClosest = r;
						closest = d;
					}
					break;
				}
				toggle = !toggle;
			}
		}

		return closest;
	}
}