#include "Test.h"

#include "imgui/imgui.h"

namespace test
{
	GLFWwindow* test::Test::s_Window = nullptr;

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
