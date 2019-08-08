#pragma once

#include "Test.h"

#include <memory>

#include "VertexArray.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Shader.h"

namespace test
{
	class Test_Cube : public Test
	{
	public:
		Test_Cube();
		~Test_Cube() {}

		virtual void OnRender() override;
		virtual void OnImGuiRender() override;

	private:
		std::unique_ptr<VertexArray> m_VAO;
		std::unique_ptr<VertexBuffer> m_VBO;
		std::unique_ptr<IndexBuffer> m_IBO;
		std::unique_ptr<Shader> m_Shader;

		glm::mat4 m_Proj, m_View;

	};

}
