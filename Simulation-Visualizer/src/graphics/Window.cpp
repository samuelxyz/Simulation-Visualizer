#include "graphics/Window.h"
#include "graphics/Definitions.h"

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include <cassert>
#include <iostream>

namespace graphics {

	Window::Initializer::Initializer()
	{
		glfwSetErrorCallback(printGLFWError);

		/* Initialize the library */
		assert(glfwInit());

		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);

		glfwWindowHint(GLFW_SAMPLES, 4);
		glfwWindowHint(GLFW_VISIBLE, GL_FALSE);
		//glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

		/* Create a windowed mode window and its OpenGL context */
		glfwWindow = glfwCreateWindow(DEFAULT_WIDTH, DEFAULT_HEIGHT,
			"Simulation-Visualizer", nullptr, nullptr);

		if (!glfwWindow)
		{
			glfwTerminate();
			assert(false && "GLFW window creation failed");
		}

		glfwSetFramebufferSizeCallback(glfwWindow, handleResize);
		glfwSetKeyCallback(glfwWindow, handleKey);
		glfwSetCursorPosCallback(glfwWindow, handleMouseMotion);
		glfwSetMouseButtonCallback(glfwWindow, handleMouseButton);
		glfwSetScrollCallback(glfwWindow, handleScroll);

		glfwSetWindowPos(glfwWindow, 50, 50);
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

		glEnable(GL_DEPTH_TEST);

		glClearColor(0.2f, 0.4f, 0.3f, 1.0f);
	}

	Window::Window()
		: initialized(false), initializer(), glfwWindow(initializer.glfwWindow),
		renderer(),
		camera(DEFAULT_WIDTH, DEFAULT_HEIGHT, glm::vec3(-5.0f, -2.0f, 10.0f), -1.2f, 0.16f),
		guiOverlay(camera),
		simulation(nullptr),
		mouseTracker { 0.0f, 0.0f, false }
	{

		glfwSetWindowUserPointer(glfwWindow, this);

		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		// Setup Platform/Renderer bindings
		ImGui_ImplGlfw_InitForOpenGL(glfwWindow, true);
		ImGui_ImplOpenGL3_Init();

		// Setup Style
		ImGui::StyleColorsDark();
		//ImGui::StyleColorsClassic();

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
		Renderer::clearScreen();

		int width, height;
		getDimensions(&width, &height);
		camera.updateWindowDims(width, height);
		renderer.updateCamera(camera.getVPMatrix());

		if (simulation != nullptr)
			simulation->render(renderer);
		renderer.renderAndClearAll();

		// GUI WIP code
		
		//glDisable(GL_DEPTH_TEST);
		renderer.clearDepth();
		guiOverlay.render(renderer);
		if (simulation != nullptr)
			simulation->renderGUIOverlay(renderer);

		//glm::vec4 testColor(1.0f, 0.0f, 1.0f, 1.0f);
		//graphics::Triangle testGuiTriangle { {
		//		graphics::ColoredVertex {testColor, glm::vec3(2.0f, 2.0f, 0.0f)},
		//		graphics::ColoredVertex {testColor, glm::vec3(20.0f, 2.0f, 0.0f)},
		//		graphics::ColoredVertex {testColor, glm::vec3(2.0f, 20.0f, 0.0f)},
		//} };
		//renderer.submit(testGuiTriangle);
		renderer.renderAndClearAll();

		//glEnable(GL_DEPTH_TEST);

		// TODO: I think the problem with multiple Renderers is in the bind() calls in Renderer::draw()

	}

	void Window::renderGUI()
	{
		camera.renderGUI();

		if (simulation != nullptr)
			simulation->renderGUI(camera);

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}

	void Window::directRender(Renderable & renderable)
	{
		renderable.render(renderer);
	}

	void Window::directRender(const graphics::Triangle & tri)
	{
		renderer.submit(tri);
	}

	void Window::directRender(const graphics::Quad & quad)
	{
		renderer.submit(quad);
	}

	void Window::directRender(const graphics::CenteredPoly & cp)
	{
		renderer.submit(cp);
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

	void Window::handleKey(GLFWwindow* glfwWindow, int key, int scancode, int action,
		int mods)
	{
		//if (ImGui::GetIO().WantCaptureKeyboard)
		//	return;

		//Window* window = getWindow(glfwWindow);
		//if (window != nullptr && window->initialized)
		//	window->camera.handleKey(key, action);
	}

	void Window::handleMouseMotion(GLFWwindow* glfwWindow, double xpos, double ypos)
	{
		if (ImGui::GetIO().WantCaptureMouse)
			return;

		Window* window = getWindow(glfwWindow);

		if (window != nullptr && window->initialized && window->mouseTracker.dragging && 
			glfwGetMouseButton(glfwWindow, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
		{
			int windowHeight;
			window->getDimensions(nullptr, &windowHeight);

			float x = static_cast<float>(xpos);
			float y = windowHeight - static_cast<float>(ypos);

			float dx = x - window->mouseTracker.prevX;
			float dy = y - window->mouseTracker.prevY;

			//std::cout << "dx: " << dx << ", dy: " << dy << std::endl;

			window->camera.handleMouseMotion(dx, dy);

			window->mouseTracker.prevX = x;
			window->mouseTracker.prevY = y;
		}
	}

	void Window::handleMouseButton(GLFWwindow* glfwWindow, int button, int action,
		int mods)
	{
		if (ImGui::GetIO().WantCaptureMouse)
			return;

		if (button == GLFW_MOUSE_BUTTON_LEFT)
		{
			Window* window = getWindow(glfwWindow);

			if (window == nullptr || !(window->initialized))
				return;

			if (action == GLFW_PRESS)
			{
				double xd, yd;
				glfwGetCursorPos(glfwWindow, &xd, &yd);
				int windowHeight;
				window->getDimensions(nullptr, &windowHeight);
				window->mouseTracker.prevY = windowHeight - static_cast<float>(yd);
				window->mouseTracker.prevX = static_cast<float>(xd);

				//std::cout << "Left mouse click detected" << std::endl;
				window->mouseTracker.dragging = true;
			}
			else // action == GLFW_RELEASE
			{
				window->mouseTracker.dragging = false;
			}
		}
	}

	void Window::handleScroll(GLFWwindow* glfwWindow, double xoffset, double yoffset)
	{
		if (ImGui::GetIO().WantCaptureMouse)
			return;

		Window* window = getWindow(glfwWindow);
		if (window != nullptr && window->initialized)
			window->camera.handleScroll(static_cast<float>(yoffset));
	}

	Window* Window::getWindow(GLFWwindow* glfwWindow)
	{
		return (Window*)glfwGetWindowUserPointer(glfwWindow);
	}

} /* namespace graphics */