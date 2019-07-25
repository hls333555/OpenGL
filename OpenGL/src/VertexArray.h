#pragma once

class VertexBuffer;
class VertexBufferLayout;

class VertexArray
{
private:
	unsigned int m_RendererID;
public:
	VertexArray();
	~VertexArray();

	void AddBuffer(const VertexBuffer& vb, const VertexBufferLayout& layout);

	/** Bind a vertex array object. */
	void Bind() const;
	/** Unbind a vertex array object. */
	void Unbind() const;
};
