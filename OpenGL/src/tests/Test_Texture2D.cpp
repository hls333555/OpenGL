#include "Test_Texture2D.h"

#include "Renderer.h"
#include "imgui/imgui.h"

#include "VertexBufferLayout.h"

namespace test
{
	Test_Texture2D::Test_Texture2D()
		: m_Proj(glm::ortho(0.f, WINDOW_WIDTH, 0.f, WINDOW_HEIGHT, -1.f, 1.f))
		, m_View(glm::translate(glm::mat4(1.f), glm::vec3(0.f, 0.f, 0.f)))
		, m_TranslationA{ 200.f, 200.f, 0.f }
		, m_TranslationB{ 400.f, 200.f, 0.f }
	{
		// Two floats for vertex position and two floats for texture coordinate
		// For texture coordinate system, the bottom-left is (0,0), the top-right is (1,1)
		float positions[] = {
			-50.f, -50.f, 0.f, 0.f, // 0
			 50.f, -50.f, 1.f, 0.f, // 1
			 50.f,  50.f, 1.f, 1.f, // 2
			-50.f,  50.f, 0.f, 1.f  // 3
		};

		unsigned int indices[] = {
			0, 1, 2,
			2, 3, 0
		};

		GLCALL(glEnable(GL_BLEND));
		// Set this to blend transparency properly
		GLCALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

		m_VAO = std::make_unique<VertexArray>();

		m_VBO = std::make_unique<VertexBuffer>(positions, 4 * 4 * sizeof(float));

		VertexBufferLayout layout;
		// Vertex position
		layout.Push<float>(2);
		// Texture coordinate
		layout.Push<float>(2);
		m_VAO->AddBuffer(*m_VBO, layout);

		m_IBO = std::make_unique<IndexBuffer>(indices, 6);

		m_Shader = std::make_unique<Shader>("res/shaders/Basic.shader");
		m_Shader->Bind();
		// Send color to the shader
		m_Shader->SetUniform4f("u_Color", 0.f, 1.f, 1.f, 1.f);

		m_Texture = std::make_unique<Texture>("res/textures/Logo_Trans.png");
		// Tell the shader which texture slot to sample from
		m_Shader->SetUniform1i("u_Texture", 0);
	}

	void Test_Texture2D::OnRender()
	{
		GLCALL(glClearColor(0.f, 0.f, 0.f, 1.f));
		GLCALL(glClear(GL_COLOR_BUFFER_BIT));

		Renderer renderer;

		m_Texture->Bind();

		{
			// Model matrix
			glm::mat4 model = glm::translate(glm::mat4(1.f), m_TranslationA);
			// Model view projection matrices (note the reverse multiplication order)
			glm::mat4 mvp = m_Proj * m_View * model;

			m_Shader->Bind();
			// Send MVP matrices to the shader
			m_Shader->SetUniformMat4f("u_MVP", mvp);
			renderer.Draw(*m_VAO, *m_IBO, *m_Shader);
		}

		{
			// Model matrix
			glm::mat4 model = glm::translate(glm::mat4(1.f), m_TranslationB);
			// Model view projection matrices (note the reverse multiplication order)
			glm::mat4 mvp = m_Proj * m_View * model;

			m_Shader->Bind();
			// Send MVP matrices to the shader
			m_Shader->SetUniformMat4f("u_MVP", mvp);
			renderer.Draw(*m_VAO, *m_IBO, *m_Shader);
		}
	}

	void Test_Texture2D::OnImGuiRender()
	{
		ImGui::SliderFloat3("TranslationA", &m_TranslationA.x, 0.f, WINDOW_WIDTH);
		ImGui::SliderFloat3("TranslationB", &m_TranslationB.x, 0.f, WINDOW_WIDTH);
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	}
}
