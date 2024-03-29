#include "VertexBuffer.h"

#include "Renderer.h"

VertexBuffer::VertexBuffer(const void* data, unsigned int size)
{
	// Generate vertex buffer object names
	GLCALL(glCreateBuffers(1, &m_RendererID));
	// Bind
	GLCALL(glBindBuffer(GL_ARRAY_BUFFER, m_RendererID));
	// Create and initialize a vertex buffer object's data store
	GLCALL(glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW));
	// Unbind
	GLCALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

VertexBuffer::~VertexBuffer()
{
	// Delete named buffer objects
	GLCALL(glDeleteBuffers(1, &m_RendererID));
}

void VertexBuffer::Bind() const
{
	GLCALL(glBindBuffer(GL_ARRAY_BUFFER, m_RendererID));
}

void VertexBuffer::Unbind() const
{
	GLCALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
}
