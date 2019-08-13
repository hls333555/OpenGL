#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include "Renderer.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "tests/Test.h"
#include "tests/Test_ClearColor.h"
#include "tests/Test_Texture2D.h"
#include "tests/Test_Cube.h"
#include "tests/Test_Lighting.h"
#include "tests/Test_Model.h"

int main(void)
{
	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	// OpenGL 3.3 + GLSL 330
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	// Use core OpenGL profile which will not make VAO object 0 an object
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	const char* glsl_version = "#version 330";

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow((int)WINDOW_WIDTH, (int)WINDOW_HEIGHT, "OpenGL Renderer", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	test::Test::s_Window = window;

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	glfwSwapInterval(1);

	// This MUST be called after a valid OpenGL rendering context being created
	if (glewInit() != GLEW_OK)
	{
		std::cout << "Failed to init GLEW!" << std::endl;
	}

	// Print OpenGL version in current graphics driver
	std::cout << glGetString(GL_VERSION) << std::endl;

	{
		Renderer renderer;

		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		// The last parameter is necessary for outputing chinese characters
		// @see Renderer.h for helper functions
		io.Fonts->AddFontFromFileTTF("res/fonts/wqy-microhei.ttc", 16.0f, NULL, io.Fonts->GetGlyphRangesChineseSimplifiedCommon());
		// Setup Dear ImGui style
		ImGui::StyleColorsDark();
		// Setup Platform/Renderer bindings
		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init(glsl_version);

		test::Test* currentTest = nullptr;
		test::TestMenu* testMenu = new test::TestMenu(currentTest);
		currentTest = testMenu;
		testMenu->RegisterTest<test::Test_ClearColor>("Clear color");
		testMenu->RegisterTest<test::Test_Texture2D>("2D Texture");
		testMenu->RegisterTest<test::Test_Cube>("3D Cube");
		testMenu->RegisterTest<test::Test_Lighting>("Lighting Scene");
		testMenu->RegisterTest<test::Test_Model>("Model Demo");

		/* Loop until the user closes the window */
		while (!glfwWindowShouldClose(window))
		{
			static float m_LastFrameTime = 0.f;
			float time = (float)glfwGetTime();
			float dt = time - m_LastFrameTime;
			m_LastFrameTime = time;

			GLCALL(glClearColor(0.f, 0.f, 0.f, 1.f));
			/* Render here */
			renderer.Clear();

			// Start the Dear ImGui frame
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

			if (currentTest)
			{
				currentTest->OnUpdate(dt);
				currentTest->OnRender();

				ImGui::Begin("Tests");
				if (currentTest != testMenu && ImGui::Button("<-"))
				{
					delete currentTest;
					currentTest = testMenu;
				}
				currentTest->OnImGuiRender();
				ImGui::End();
			}

			// Rendering
			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

			/* Swap front and back buffers */
			glfwSwapBuffers(window);

			/* Poll for and process events */
			glfwPollEvents();
		}

		delete currentTest;
		if (currentTest != testMenu)
		{
			delete testMenu;
		}
	}

	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();
	return 0;
}
