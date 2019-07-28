#include "Test_ClearColor.h"

#include "Renderer.h"
#include "imgui/imgui.h"

namespace test
{
	Test_ClearColor::Test_ClearColor()
		: m_ClearColor{0.f, 0.f, 0.f, 1.f}
	{
	}

	void Test_ClearColor::OnRender()
	{
		GLCALL(glClearColor(m_ClearColor[0], m_ClearColor[1], m_ClearColor[2], m_ClearColor[3]));
		GLCALL(glClear(GL_COLOR_BUFFER_BIT));
	}

	void Test_ClearColor::OnImGuiRender()
	{
		ImGui::ColorEdit4("Clear Color", m_ClearColor);
	}
}
