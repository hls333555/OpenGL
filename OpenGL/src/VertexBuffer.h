#pragma once

class VertexBuffer
{
private:
	unsigned int m_RendererID;

public:
	/** Size means bytes. */
	VertexBuffer(const void* data, unsigned int size);
	~VertexBuffer();

	/** Bind a named vertex buffer object. */
	void Bind() const;
	/** Unbind a named vertex buffer object. */
	void Unbind() const;
};
