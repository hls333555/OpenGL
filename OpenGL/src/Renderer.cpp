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

std::string wstring2utf8string(const std::wstring& str)
{
	static std::wstring_convert<std::codecvt_utf8<wchar_t> > strCnv;
	return strCnv.to_bytes(str);
}

std::wstring utf8string2wstring(const std::string& str)
{
	static std::wstring_convert< std::codecvt_utf8<wchar_t> > strCnv;
	return strCnv.from_bytes(str);
}

std::string wstring2string(const std::wstring& str, const std::string& locale)
{
	typedef std::codecvt_byname<wchar_t, char, std::mbstate_t> F;
	static std::wstring_convert<F> strCnv(new F(locale));

	return strCnv.to_bytes(str);
}

std::wstring string2wstring(const std::string& str, const std::string& locale)
{
	typedef std::codecvt_byname<wchar_t, char, std::mbstate_t> F;
	static std::wstring_convert<F> strCnv(new F(locale));

	return strCnv.from_bytes(str);
}

std::string UTF8ToDefault(const std::string& str)
{
	auto wstr = utf8string2wstring(str);
	return wstring2string(wstr);
}

std::string DefaultToUTF8(const std::string& str)
{
	auto wstr = string2wstring(str);
	return wstring2utf8string(wstr);
}

Renderer::Renderer()
{
}

Renderer::~Renderer()
{
}

void Renderer::Clear() const
{
	GLCALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
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
