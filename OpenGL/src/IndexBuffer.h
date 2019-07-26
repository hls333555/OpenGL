#pragma once

class IndexBuffer
{
public:
	/** Count means element count. */
	IndexBuffer(const unsigned int* data, unsigned int count);
	~IndexBuffer();

	inline unsigned int GetCount() const { return m_Count; };

	/** Bind a named index buffer object. */
	void Bind() const;
	/** Unbind index buffer objects. */
	void Unbind() const;

private:
	unsigned int m_RendererID;
	unsigned int m_Count;

};
