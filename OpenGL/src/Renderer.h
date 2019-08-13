#pragma once

#include <GL/glew.h>

#include <codecvt>
#include <locale>

class VertexArray;
class IndexBuffer;
class Shader;

#define WINDOW_WIDTH	1280.f
#define WINDOW_HEIGHT	720.f

#define ASSERT(x) if(!(x)) __debugbreak();
// You should ensure "DEBUG" exists in PreprocessorDefinations of Debug configuration
#ifdef DEBUG
#define GLCALL(x) GLClearError();\
	x;\
	ASSERT(GLLogCall(__FILE__, #x, __LINE__))
#else
#define GLCALL(x) x
#endif

void GLClearError();

bool GLLogCall(const char* file, const char* function, int line);

std::string wstring2utf8string(const std::wstring& str);
std::wstring utf8string2wstring(const std::string& str);
std::string wstring2string(const std::wstring& str, const std::string& locale = "zh-CN");
std::wstring string2wstring(const std::string& str, const std::string& locale = "zh-CN");
/** 
 * Helper function for string encoding conversion.
 * Since some libraries like imgui and assimp only supports Chinese characters with UTF-8 whereas our system uses zh-CN,
 * we must convert UTF-8 string back to zh-CN to be handled by common functions.
 */
std::string UTF8ToDefault(const std::string& str);
/**
 * Helper function for string encoding conversion.
 * Since our system uses zh-CN whereas some libraries like imgui and assimp only supports Chinese characters with UTF-8,
 * we must convert zh-CN string to UTF-8 to be handled by those library functions (imgui requires UTF-8 to display Chinese characters).
 */
std::string DefaultToUTF8(const std::string& str);

class Renderer
{
public:
	Renderer();
	~Renderer();

	void Clear() const;
	void Draw(const VertexArray& va, const IndexBuffer& ib, const Shader& shader) const;

private:

};
