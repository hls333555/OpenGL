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

		m_VAO.reset(new VertexArray());

		m_VBO.reset(new VertexBuffer(positions, 4 * 4 * sizeof(float)));

		VertexBufferLayout layout;
		// Vertex position
		layout.Push<float>(2);
		// Texture coordinate
		layout.Push<float>(2);
		m_VAO->AddBuffer(*m_VBO, layout);

		m_IBO.reset(new IndexBuffer(indices, 6));

		m_Shader.reset(new Shader("res/shaders/BasicTexture.shader"));

		m_Texture.reset(new Texture("res/textures/Logo_Trans_D.png"));
		m_Shader->Bind();
		m_Shader->SetUniform1i("u_Texture", 0);
	}

	void Test_Texture2D::OnRender()
	{
		Renderer renderer;
		{
			m_Texture->Bind();

			glm::mat4 modelA = glm::translate(glm::mat4(1.f), m_TranslationA);
			m_Shader->Bind();
			m_Shader->SetUniformMat4f("u_Model", modelA);
			m_Shader->SetUniformMat4f("u_ViewProjection", m_Proj * m_View);

			renderer.Draw(*m_VAO, *m_IBO, *m_Shader);
		}
		{
			m_Texture->Bind();

			glm::mat4 modelB = glm::translate(glm::mat4(1.f), m_TranslationB);
			m_Shader->Bind();
			m_Shader->SetUniformMat4f("u_Model", modelB);
			m_Shader->SetUniformMat4f("u_ViewProjection", m_Proj * m_View);

			renderer.Draw(*m_VAO, *m_IBO, *m_Shader);
		}
	}

	void Test_Texture2D::OnImGuiRender()
	{
		ImGui::SliderFloat3("TranslationA", &m_TranslationA.x, 0.f, WINDOW_WIDTH);
		ImGui::SliderFloat3("TranslationB", &m_TranslationB.x, 0.f, WINDOW_WIDTH);
	}
}
