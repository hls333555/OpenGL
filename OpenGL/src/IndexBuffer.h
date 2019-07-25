#pragma once

class IndexBuffer
{
private:
	unsigned int m_RendererID;
	unsigned int m_Count;

public:
	/** Count means element count. */
	IndexBuffer(const unsigned int* data, unsigned int count);
	~IndexBuffer();

	inline unsigned int GetCount() const { return m_Count; };

	/** Bind a named index buffer object. */
	void Bind() const;
	/** Unbind a named index buffer object */
	void Unbind() const;

};
