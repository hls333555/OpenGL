#pragma once

#include <GL/glew.h>

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

class Renderer
{
public:
	Renderer();
	~Renderer();

	void Clear() const;
	void Draw(const VertexArray& va, const IndexBuffer& ib, const Shader& shader) const;

private:

};
