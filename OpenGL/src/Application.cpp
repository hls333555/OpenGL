#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>

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

	// This MUST be called after a valid OpenGL rendering context being created
	if (glewInit() != GLEW_OK)
	{
		std::cout << "Failed to init GLEW!" << std::endl;
	}

	// Print OpenGL version in current graphics driver
	std::cout << glGetString(GL_VERSION) << std::endl;

	float positions[6] = {
		-0.5f, -0.5f,
		 0.f,   0.5f,
		 0.5f, -0.5f
	};

	unsigned int buffer;
	// Generate buffer object names
	glGenBuffers(1, &buffer);
	// Bind a named buffer object
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	// Creates and initializes a buffer object's data store.
	glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(float), positions, GL_STATIC_DRAW);

	// Enable the position vertex attribute data
	glEnableVertexAttribArray(0);
	// Define a position vertex attribute data
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (const void*)0);

	// Use C++11 raw string literals
	std::string vertexShaderSource = R"(
		#version 330 core

		// The location of the position vertex attribute data is 0
		layout(location = 0) in vec4 position;
		void main()
		{
			gl_Position = position;
		}	
	)";

	std::string fragmentShaderSource = R"(
		#version 330 core

		layout(location = 0) out vec4 color;
		void main()
		{
			color = vec4(0.f, 1.f, 1.f, 1.f);
		}	
	)";
	
	unsigned int program = CreateShader(vertexShaderSource, fragmentShaderSource);
	// Install the program object as part of current rendering state
	glUseProgram(program);

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		/* Render here */
		glClear(GL_COLOR_BUFFER_BIT);

		// Issue a drawcall
		glDrawArrays(GL_TRIANGLES, 0, 3);

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
