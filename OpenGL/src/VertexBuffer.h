#pragma once

class VertexBuffer
{
public:
	/** Size means bytes. */
	VertexBuffer(const void* data, unsigned int size);
	~VertexBuffer();

	/** Bind a named vertex buffer object. */
	void Bind() const;
	/** Unbind vertex buffer objects. */
	void Unbind() const;

private:
	unsigned int m_RendererID;
};
