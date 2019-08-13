#include "Test.h"

#include "imgui/imgui.h"

#include "Renderer.h"

namespace test
{
	GLFWwindow* test::Test::s_Window = nullptr;

	Test::~Test()
	{
		// You should disable previously enabled things here!
		GLCALL(glDisable(GL_DEPTH_TEST));
		GLCALL(glDisable(GL_BLEND));
	}

	TestMenu::TestMenu(Test*& currentTest)
		: m_CurrentTest(currentTest)
	{
	}

	void TestMenu::OnImGuiRender()
	{
		for (auto& test : m_Tests)
		{
			// If the button is clicked
			if (ImGui::Button(test.first.c_str()))
			{
				// Call the construct function
				m_CurrentTest = test.second();
			}
		}
	}
	
}
