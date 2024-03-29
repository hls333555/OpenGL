#pragma once

class VertexBuffer;
class VertexBufferLayout;

class VertexArray
{
public:
	VertexArray();
	~VertexArray();

	void AddBuffer(const VertexBuffer& vb, const VertexBufferLayout& layout);

	/** Bind a vertex array object. */
	void Bind() const;
	/** Unbind vertex array objects. */
	void Unbind() const;

private:
	unsigned int m_RendererID;
};
