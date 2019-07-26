#include "Renderer.h"

#include <iostream>

#include "VertexArray.h"
#include "IndexBuffer.h"
#include "Shader.h"

void GLClearError()
{
	// Loop to clear all previous errors
	while (glGetError() != GL_NO_ERROR);
}

bool GLLogCall(const char* file, const char* function, int line)
{
	while (unsigned int error = glGetError())
	{
		std::cout << "OpenGL error: " << error << " in " << file << ", " << function << ", " << line << std::endl;
		return false;
	}
	return true;
}

Renderer::Renderer()
{
}

Renderer::~Renderer()
{
}

void Renderer::Clear() const
{
	GLCALL(glClear(GL_COLOR_BUFFER_BIT));
}

void Renderer::Draw(const VertexArray& va, const IndexBuffer& ib, const Shader& shader) const
{
	shader.Bind();
	va.Bind();
	ib.Bind();
	// Issue a drawcall
	// The count is actually the number of indices rather than vertices
	// Since index buffer is already bound to GL_ELEMENT_ARRAY_BUFFER, we do not need to specify the pointer to indices
	GLCALL(glDrawElements(GL_TRIANGLES, ib.GetCount(), GL_UNSIGNED_INT, nullptr));
}
