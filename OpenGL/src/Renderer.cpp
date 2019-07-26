#include "Renderer.h"

#include <iostream>

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
