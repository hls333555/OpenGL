#include "VertexArray.h"

#include "Renderer.h"
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"

VertexArray::VertexArray()
{
	// Generate vertex array object names
	GLCALL(glCreateVertexArrays(1, &m_RendererID));
}

VertexArray::~VertexArray()
{
	// Delete vertex array objects
	GLCALL(glDeleteVertexArrays(1, &m_RendererID));
}

void VertexArray::AddBuffer(const VertexBuffer& vb, const VertexBufferLayout& layout)
{
	// Bind
	GLCALL(glBindVertexArray(m_RendererID));
	vb.Bind();
	const auto& elements = layout.GetElements();
	unsigned int offset = 0;
	for (unsigned int i = 0; i < elements.size(); ++i)
	{
		const auto& element = elements[i];
		// Enable the specified vertex attribute data
		GLCALL(glEnableVertexAttribArray(i));
		// Define the specified vertex attribute data
		GLCALL(glVertexAttribPointer(i, element.count, element.type, element.normalized, layout.GetStride(), (const void*)offset));
		offset += element.count * VertexBufferElement::GetSizeOfType(element.type);
	}
}

void VertexArray::Bind() const
{
	GLCALL(glBindVertexArray(m_RendererID));
}

void VertexArray::Unbind() const
{
	GLCALL(glBindVertexArray(0));
}
