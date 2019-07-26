#include "core/stdafx.h"
#include "graphics/Window.h"
#include "graphics/content/VisualSphere.h"
#include "io/MouseHandler.h"
#include "entity/Entity.h"

#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <imgui/imgui_internal.h>

namespace graphics {

	Window::Initializer::Initializer()
	{
		glfwSetErrorCallback(printGLFWError);

		/* Initialize the library */
		assert(glfwInit());

		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);

		glfwWindowHint(GLFW_SAMPLES, 4);
		glfwWindowHint(GLFW_VISIBLE, GL_FALSE);
		glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

		/* Create a windowed mode window and its OpenGL context */
		glfwWindow = glfwCreateWindow(DEFAULT_WIDTH, DEFAULT_HEIGHT,
			"Simulation-Visualizer", nullptr, nullptr);

		if (!glfwWindow)
		{
			glfwTerminate();
			assert(false && "GLFW window creation failed");
		}

		glfwSetFramebufferSizeCallback(glfwWindow, handleResize);
		glfwSetCursorPosCallback(glfwWindow, handleMouseMotion);
		glfwSetMouseButtonCallback(glfwWindow, handleMouseButton);
		glfwSetScrollCallback(glfwWindow, handleScroll);

		//glfwSetWindowPos(glfwWindow, 50, 50);
		glfwShowWindow(glfwWindow);

		/* Make the window's context current */
		glfwMakeContextCurrent(glfwWindow);

		/* Set refresh rate to once per frame (60 Hz) */
		glfwSwapInterval(1);

		/* Initialize GLEW */
		assert(glewInit() == GLEW_OK);

		std::cout << "Running in debug mode with OpenGL version " <<
			glGetString(GL_VERSION) << std::endl;
		glDebugMessageCallback(printGLDebug, nullptr);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnable(GL_POLYGON_SMOOTH);

		glEnable(GL_LINE_SMOOTH);
		glLineWidth(1);

		glEnable(GL_MULTISAMPLE);

		glClearColor(0.9f, 0.9f, 0.9f, 1.0f);
	}

	Window::GUIOverlay::GUIOverlay(graphics::Camera& camera)
		: camera(camera), axes(), cursorDot()
	{
		for (auto& axis: axes)
		{
			axis.style = graphics::VisualEntity::Style::SOLID_COLOR;
			axis.xMin = -axisThickness;
			axis.xMax = axisLength + axisThickness;
			axis.yMin = -axisThickness;
			axis.yMax = axisThickness;
			axis.zMin = -axisThickness;
			axis.zMax = axisThickness;
		}

		axes[0].color = graphics::COLOR_RED;
		axes[1].color = graphics::COLOR_GREEN;
		axes[2].color = graphics::COLOR_BLUE;

		axes[1].orientation = glm::angleAxis(core::HALF_PI, glm::vec3(0.0f, 0.0f, 1.0f));
		axes[2].orientation = glm::angleAxis(core::HALF_PI, glm::vec3(0.0f, -1.0f, 0.0f));

		static constexpr float dotSize = axisThickness * 1.01f;
		cursorDot.style = graphics::VisualEntity::Style::SOLID_COLOR;
		cursorDot.xMin = -dotSize;
		cursorDot.xMax = dotSize;
		cursorDot.yMin = -dotSize;
		cursorDot.yMax = dotSize;
		cursorDot.zMin = -dotSize;
		cursorDot.zMax = dotSize;
		cursorDot.color = graphics::COLOR_WHITE;
	}

	Window::GUIOverlay::~GUIOverlay()
	{
	}

	void Window::GUIOverlay::render(const Window& window, graphics::Renderer& renderer) const
	{
		if (camera.shouldShowAxes())
		{
			glm::vec3 lookTargetPos = camera.getPosition() + camera.getLookVec();

			for (unsigned int i = 0; i < axes.size(); ++i)
			{
				axes[i].position = lookTargetPos;
				axes[i].render(renderer);

				glm::vec3 labelPos3d(lookTargetPos);
				labelPos3d[i] += axisLength * 1.1f;

				std::string name;
				const glm::vec4* color;
				if (i == 0)
				{
					name = "x";
					color = &(graphics::COLOR_RED);
				}
				else if (i == 1)
				{
					name = "y";
					color = &(graphics::COLOR_GREEN);
				}
				else
				{
					name = "z";
					color = &(graphics::COLOR_BLUE);
				}

				camera.renderLabel(labelPos3d, true, "labelAxis" + name, name, *color, ImVec2(0.47f, 0.5f));
			}

			cursorDot.position = lookTargetPos;
			cursorDot.render(renderer);
		}

		static int dotTimer = 15;
		entity::Entity* orbitingEntity = window.mouseHandler.getOrbitingEntity();
		if (orbitingEntity != nullptr)
		{
			// render dot on focused entity
			if (dotTimer > 0)
				--dotTimer;

			glm::vec4 color = graphics::COLOR_ORBIT;
			color.a = 0.8f - dotTimer/20.0f;

			graphics::VisualSphere dot(orbitingEntity->getPosition(),
				core::QUAT_IDENTITY, (dotTimer/1.5f + 1.0f) * graphics::MARKER_DOT_RADIUS,
				graphics::VisualEntity::Style::SOLID_COLOR, color);

			dot.render(renderer);

			window.camera.renderLabel(orbitingEntity->getPosition(), 
				true, "CameraOrbitingMarkerLabel", "Orbiting", graphics::COLOR_DEFAULT, graphics::PIVOT_RIGHT);
		}
		else
		{
			dotTimer = 15;
		}
	}

	Window::Window()
		: initialized(false), initializer(), glfwWindow(initializer.glfwWindow),
		renderer(),
		camera(DEFAULT_WIDTH, DEFAULT_HEIGHT, glm::vec3(-5.0f, -2.0f, 10.0f), -1.2f, 0.16f),
		guiOverlay(camera),
		simulation(nullptr),
		mouseHandler(*this)
	{

		glfwSetWindowUserPointer(glfwWindow, this);

		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		// Setup Platform/Renderer bindings
		ImGui_ImplGlfw_InitForOpenGL(glfwWindow, true);
		ImGui_ImplOpenGL3_Init();

		// Setup Style
		//ImGui::StyleColorsDark();
		//ImGui::StyleColorsLight();
		ImGui::StyleColorsClassic();

		initialized = true;
	}

	Window::~Window()
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

		glfwDestroyWindow(glfwWindow);
		glfwTerminate();
	}

	void Window::setSimulation(core::Simulation* sim)
	{
		simulation = sim;
	}

	bool Window::shouldClose() const
	{
		return glfwWindowShouldClose(glfwWindow);
	}

	void Window::startGUI()
	{
		// Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void Window::update()
	{
		camera.pollKeys(glfwWindow);
		if (simulation != nullptr)
			simulation->update();
	}

	void Window::render()
	{
		int width, height;
		getDimensions(&width, &height);

		if (!(width > 0 && height > 0))
			return; // window is minimized or something

		Renderer::clearScreen();

		camera.updateWindowDims(width, height);
		renderer.updateCamera(camera.getVPMatrix());

		if (simulation != nullptr)
		{
			simulation->renderGUI(renderer, camera);
			simulation->render(renderer, camera.getPosition());
		}

		// overlays
		renderer.clearDepth();
		if (simulation != nullptr)
			simulation->renderGUIOverlay(renderer, camera);
		guiOverlay.render(*this, renderer);
		renderer.renderAndClearAll(true, true);

		// TODO: I think the problem with multiple Renderers is in the bind() calls in Renderer::draw()
		// So just working around it by reusing the same Renderer multiple times xd

		static bool showDemoWindow = false;

		camera.renderGUI();
		if (ImGui::Begin("General"))
		{
			ImGui::TextUnformatted("Degrees        Radians");
			static float deg = 0.0f, rad = 0.0f;
			ImGui::PushItemWidth(64.0f);
			if (ImGui::InputFloat("##DegInput", &deg, 0.0f, 0.0f, "%.4f"))
				rad = glm::radians(deg);
			ImGui::SameLine();
			ImGui::TextUnformatted("<->");
			ImGui::SameLine();
			if (ImGui::InputFloat("##RadInput", &rad, 0.0f, 0.0f, "%.5f"))
				deg = glm::degrees(rad);
			ImGui::PopItemWidth();

			ImGui::Checkbox("Show Demo Window", &showDemoWindow);
		}

		if (showDemoWindow)
			ImGui::ShowDemoWindow(&showDemoWindow);

		ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}

	void Window::swapBuffers()
	{
		glfwSwapBuffers(glfwWindow);
	}

	void Window::getDimensions(int* widthOut, int* heightOut)
	{
		glfwGetFramebufferSize(glfwWindow, widthOut, heightOut);
	}

	void Window::printGLFWError(int error, const char* description)
	{
		std::cerr << "[GLFW Error] " << description << std::endl;
	}

	void APIENTRY Window::printGLDebug(GLenum source,
		GLenum type, GLuint id, GLenum severity, GLsizei length,
		const GLchar* message, const void* userParam)
	{
		std::cout << "[GL] " << message << std::endl;
	}

	void Window::handleResize(GLFWwindow* glfwWindow, int width, int height)
	{
		glViewport(0, 0, width, height);
	}

	void Window::handleMouseMotion(GLFWwindow* glfwWindow, double xpos, double ypos)
	{
		Window* window = getWindow(glfwWindow);
		if (window != nullptr && window->initialized)
			window->mouseHandler.handleMouseMotionCallback(glfwWindow, xpos, ypos);
	}

	void Window::handleMouseButton(GLFWwindow* glfwWindow, int button, int action,
		int mods)
	{
		Window* window = getWindow(glfwWindow);
		if (window != nullptr && window->initialized)
			window->mouseHandler.handleMouseButtonCallback(glfwWindow, button, action, mods);
	}

	void Window::handleScroll(GLFWwindow* glfwWindow, double xoffset, double yoffset)
	{
		Window* window = getWindow(glfwWindow);
		if (window != nullptr && window->initialized)
			window->mouseHandler.handleScrollCallback(glfwWindow, xoffset, yoffset);
	}

	Window* Window::getWindow(GLFWwindow* glfwWindow)
	{
		return (Window*)glfwGetWindowUserPointer(glfwWindow);
	}

} /* namespace graphics */