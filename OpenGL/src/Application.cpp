#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include "Renderer.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexArray.h"
#include "VertexBufferLayout.h"
#include "Shader.h"
#include "Texture.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

int main(void)
{
	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	// Use OpenGL 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	// Use core OpenGL profile which will not make VAO object 0 an object
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(960, 540, "Hello World", NULL, NULL);
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

	{
		// Two floats for vertex position and two floats for texture coordinate
		// For texture coordinate system, the bottom-left is (0,0), the top-right is (1,1)
		float positions[] = {
			100.f, 100.f, 0.f, 0.f, // 0
			200.f, 100.f, 1.f, 0.f, // 1
			200.f, 200.f, 1.f, 1.f, // 2
			100.f, 200.f, 0.f, 1.f  // 3
		};

		unsigned int indices[] = {
			0, 1, 2,
			2, 3, 0
		};

		GLCALL(glEnable(GL_BLEND));
		// Set this to blend transparency properly
		GLCALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

		VertexArray va;

		VertexBuffer vb(positions, 4 * 4 * sizeof(float));

		VertexBufferLayout layout;
		// Vertex position
		layout.Push<float>(2);
		// Texture coordinate
		layout.Push<float>(2);
		va.AddBuffer(vb, layout);

		IndexBuffer ib(indices, 6);

		// Projection matrix
		glm::mat4 proj = glm::ortho(0.f, 960.f, 0.f, 540.f, -1.f, 1.f);
		// View matrix
		glm::mat4 view = glm::translate(glm::mat4(1.f), glm::vec3(-100.f, 0.f, 0.f));
		// Model matrix
		glm::mat4 model = glm::translate(glm::mat4(1.f), glm::vec3(200.f, 200.f, 0.f));
		// Model view projection matrices (note the reverse multiplication order)
		glm::mat4 mvp = proj * view * model;

		Shader shader("res/shaders/Basic.shader");
		shader.Bind();
		// Send color to the shader
		shader.SetUniform4f("u_Color", 0.f, 1.f, 1.f, 1.f);
		// Send MVP matrices to the shader
		shader.SetUniformMat4f("u_MVP", mvp);

		Texture texture("res/textures/Logo_Trans.png");
		int textureSlot = 0;
		texture.Bind(textureSlot);
		// Tell the shader which texture slot to sample from
		shader.SetUniform1i("u_Texture", textureSlot);

		// Unbind all the objects
		va.Unbind();
		shader.Unbind();
		vb.Unbind();
		ib.Unbind();

		Renderer renderer;

		float r = 0.f;
		float increment = 0.05f;
		/* Loop until the user closes the window */
		while (!glfwWindowShouldClose(window))
		{
			/* Render here */
			renderer.Clear();

			shader.Bind();
			shader.SetUniform4f("u_Color", r, 1.f, 1.f, 1.f);

			renderer.Draw(va, ib, shader);

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
	}

	glfwTerminate();
	return 0;
}
