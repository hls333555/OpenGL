#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#define ASSERT(x) if(!(x)) __debugbreak();
// You should ensure "DEBUG" exists in PreprocessorDefinations of Debug configuration
#ifdef DEBUG
#define GLCALL(x) GLClearError();\
	x;\
	ASSERT(GLLogCall(__FILE__, #x, __LINE__))
#else
#define GLCALL(x) x
#endif

static void GLClearError()
{
	// Loop to clear all previous errors
	while (glGetError() != GL_NO_ERROR);
}

static bool GLLogCall(const char* file, const char* function, int line)
{
	while (unsigned int error = glGetError())
	{
		std::cout << "OpenGL error: " << error << " in " << file << ", " << function << ", " << line << std::endl;
		return false;
	}
	return true;
}

static void ParseShader(const std::string& filePath, std::string& vertexShaderSource, std::string& fragmentShaderSource)
{
	std::ifstream stream(filePath);

	enum class ShaderType
	{
		NONE = -1,
		VERTEX = 0,
		FRAGMENT = 1
	};

	std::string line;
	std::stringstream ss[2];
	ShaderType type = ShaderType::NONE;
	while (getline(stream, line))
	{
		if (line.find("#shader") != std::string::npos)
		{
			if (line.find("vertex") != std::string::npos)
			{
				type = ShaderType::VERTEX;
			}
			else if (line.find("fragment") != std::string::npos)
			{
				type = ShaderType::FRAGMENT;
			}
		}
		else
		{
			ss[(int)type] << line << "\n";
		}
	}
	vertexShaderSource = ss[0].str();
	fragmentShaderSource = ss[1].str();
}

static unsigned int CompileShader(unsigned int type, const std::string& source)
{
	// Create a shader object
	unsigned int id = glCreateShader(type);
	// Convert string to a char pointer
	const char* src = source.c_str();
	// Load the source code into the shader object
	glShaderSource(id, 1, &src, nullptr);
	// Compile the shader object
	glCompileShader(id);

	int result;
	glGetShaderiv(id, GL_COMPILE_STATUS, &result);
	if (result == GL_FALSE)
	{
		int length;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
		// Allocate on the stack dynamically
		char* message = (char*)_malloca(length * sizeof(char));
		// Return the information log for the shader object
		glGetShaderInfoLog(id, length, &length, message);
		std::cout << "Failed to compile " << (type == GL_VERTEX_SHADER ? "vertex" : "fragmemt") << " shader!" << std::endl;
		std::cout << message << std::endl;
		glDeleteShader(id);
		return 0;
	}

	return id;
}

static int CreateShader(const std::string& vertexShaderSource, const std::string& fragmentShaderSource)
{
	// Create the program object
	unsigned int program = glCreateProgram();

	unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShaderSource);
	unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

	// Attach shaders to the program object
	glAttachShader(program, vs);
	glAttachShader(program, fs);
	// Link the program
	glLinkProgram(program);
	// Check to see whether the executables contained in program can execute given the current OpenGL state.
	glValidateProgram(program);

	glDeleteShader(vs);
	glDeleteShader(fs);

	int result;
	glGetProgramiv(program, GL_VALIDATE_STATUS, &result);
	if (result == GL_FALSE)
	{
		std::cout << "Failed to validate program!" << std::endl;
		return 0;
	}

	return program;
}

int main(void)
{
	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	glfwSwapInterval(1);

	// This MUST be called after a valid OpenGL rendering context being created
	if (glewInit() != GLEW_OK)
	{
		std::cout << "Failed to init GLEW!" << std::endl;
	}

	// Print OpenGL version in current graphics driver
	std::cout << glGetString(GL_VERSION) << std::endl;

	float positions[] = {
		-0.5f, -0.5f, // 0
		 0.5f, -0.5f, // 1
		 0.5f,  0.5f, // 2
		-0.5f,  0.5f  // 3
	};

	unsigned int indices[] = {
		0, 1, 2,
		2, 3, 0
	};

	// Vertex buffer object
	unsigned int vbo;
	// Generate vertex buffer object names
	glGenBuffers(1, &vbo);
	// Bind a named vertex buffer object
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	// Create and initialize a vertex buffer object's data store
	glBufferData(GL_ARRAY_BUFFER, 6 * 2 * sizeof(float), positions, GL_STATIC_DRAW);

	// Enable the position vertex attribute data
	glEnableVertexAttribArray(0);
	// Define a position vertex attribute data
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (const void*)0);

	// Index buffer object
	unsigned int ibo;
	// Generate index buffer object names
	glGenBuffers(1, &ibo);
	// Bind a named index buffer object
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	// Create and initialize an index buffer object's data store
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), indices, GL_STATIC_DRAW);

	std::string vsSource, fsSource;
	// Read shaders from shader file
	ParseShader("res/shaders/Basic.shader", vsSource, fsSource);
	// Create shader
	unsigned int program = CreateShader(vsSource, fsSource);
	// Install the program object as part of current rendering state
	glUseProgram(program);

	int location = glGetUniformLocation(program, "u_Color");
	// If uColor is not used in the fragment shader, it will return -1
	ASSERT(location != -1);
	glUniform4f(location, 0.f, 1.f, 1.f, 1.f);

	float r = 0.f;
	float increment = 0.05f;
	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		/* Render here */
		glClear(GL_COLOR_BUFFER_BIT);

		glUniform4f(location, r, 1.f, 1.f, 1.f);
		// Issue a drawcall
		// The count is actually the number of indices rather than vertices
		// Since index buffer is already bound to GL_ELEMENT_ARRAY_BUFFER, we do not need to specify the pointer to indices
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

		if (r > 1.f)
		{
			increment = -0.05f;
		}
		else if (r < 0.f)
		{
			increment = 0.05f;
		}
		r += increment;

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	// Delete the program object
	glDeleteProgram(program);

	glfwTerminate();
	return 0;
}
