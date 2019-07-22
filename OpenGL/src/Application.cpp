#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>

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
		std::cout << "Glew Init Error!" << std::endl;
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

	glfwTerminate();
	return 0;
}
