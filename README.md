# OpenGL

*Special thanks to TheCherno who brings me into the magical OpenGL world!*

## What is OpenGL

OpenGL is neither a library, a engine, nor a framework, it is just a **specification**, there is no implementation in it. Who actually implements those code are GPU manufacturers. The drivers for the GPU such as Nvidia drivers are what contain implementation of OpenGL. Nvidia, AMD, Intel etc., they have their own implementations. Everyone's implementation for OpenGL is slightly different which is why something may work on Nvidia GPU, while the same game may look a little bit different on AMD GPU. As a matter of fact, OpenGL can not be regarded as open source since you cannot see the source code for OpenGL.

The biggest distinction between legacy OpenGL and modern OpenGL is shaders. A shader is a program(code) that runs on the GPU.

## Setting up OpenGL and Creating a Window

**GLFW** is a library which provides appropriate platform layers - **the implementation of the window creation and management** for Windows, MacOS and Linux. All it's going to do for us is to *create a window*, *create a OpenGL context* and *give us access to some basic things like input*.

Here are the main steps:

* Download GLFW precompiled binaries from [GLFW](https://www.glfw.org/) (If your application is being built for Win64/x64, choose 64-bit Windows Binaries, otherwise choose 32-bit one)
* Create a vs project, add a "Dependencies" folder into your solution directory, inside which create a "GLFW" folder and copy include folder and lib folder from the downloaded content here
* Create a cpp file named "Application", copy and paste the example code from [GLFW Documentation](https://www.glfw.org/documentation.html)
* Open the project property window
  * Make sure *Configuration* is set to All Configurations and check the *Platform* setting next to it
  * Add include path to *Additional Include Directories* (the path should look like this `$(SolutionDir)Dependencies\GLFW\include`)
  * Add lib path to *Additional Library Directories* (the path should look like this `$(SolutionDir)Dependencies\GLFW\lib-vc2019`)
  * Remove all contents in *Additional Dependencies* and Add `glfw3.lib` to it (we use static linking here and below)

* When we compile this file, no error occurs; however if we build(compile and link) the project, a bunch of link errors appear

  * Let's address the issue related to glClear first. **Basically, what linker is trying to do is to find a definition for that glClear function which is inside OpenGL library files.** We have to link that - add `opengl32.lib` to *Additional Dependencies*, now the glClear issue should go away

  * For the remaining platform specific issues, you can copy the function signature name for short and search it online, for example, "RegisterDeviceNotificationW", you will find library requirements for this function in MSDN pages like this:

    ![image](https://github.com/hls333555/OpenGL/tree/master/images/MSDN.png)

    Add that to *Additional Dependencies* and the related issues should go away

* After fixing all other issues using the above method, you will see a black window like below when you run the program:

  ![image](https://github.com/hls333555/OpenGL/tree/master/images/OpenGL_window.png)

## Using modern OpenGL

If we want to call OpenGL functions inside our c++ code, we need to get them from somewhere. Since OpenGL functions are implemented inside graphics drivers, if we want to use any kind of functionality that is newer than OpenGL 1.1, we need to get into those drivers, pull out the functions and call them - basically is to get the function declarations and then link against the functions as well, so we need to access the driver dll files and retrieve function pointers to the functions inside those libraries.

**GLEW** is a library which provides an OpenGL API specification - function declarations, constants etc. The behind-the-scene implementation of the library - those c/cpp files will *identify what graphics driver you are using*, *find the appropriate dll file* and *load all the function pointers*. **The library does not implement those OpenGL functions, it just access the functions that are already on the computer in binary form.**

Here are the main steps:

* Download GLEW precompiled binaries from [GLEW](http://glew.sourceforge.net/index.html). *(If you want to create a game engine or a series of project utilizing OpenGL, you can download the source code instead of the binaries)*
* Copy and paste GLEW files into our "Dependencies" folder
* Add include path, lib path and lib file to the project configuration like above
* Navigate to [GLEW Documentation](http://glew.sourceforge.net/basic.html) to see how to use it

The followings are the issues you may encounter:

* If you put `#include <GL/glew.h>` below `#include <GLFW/glfw3.h>` and compile, an error, saying *gl.h included before glew.h*, will occur.

  To fix this, simply exchange the include order

* If you try to call `glewInit();`, you will get a link error.

  You can dig into this by navigating to its definition in glew.h where you will find a `GLEWAPI` macro defining the return type, after jumping to its definition, you will find the reason that you did not define the `GLEW_STATIC` macro.

  ![image](https://github.com/hls333555/OpenGL/tree/master/images/GLEW_STATIC.png)

  To fix this, just add `GLEW_STATIC` to *Preprocessor Definitions*

* If you put `glewInit()` above `glfwCreateWindow()`, and replace it with the following code to check whether GLEW has initialized successfully, you will see the error illustrated below:

  ```cpp
  	if (glewInit() != GLEW_OK)
  	{
  		std::cout << "Glew Init Error!" << std::endl;
  	}
  ```

  If you take a look at the [GLEW Documentation](http://glew.sourceforge.net/basic.html), you will find the answer:

  > First you need to create a valid OpenGL rendering context and call `glewInit()` to initialize the extension entry points.

  To fix this, just move the above code below `glfwMakeContextCurrent()`.

## Vertex Buffers and Drawing a Triangle in OpenGL

To draw a triangle in legacy OpenGL, the code is as little as below:

```cpp
// Put these below glClear()
glBegin(GL_TRIANGLES);
glVertex2f(-0.5f, -0.5f);
glVertex2f(0.f, 0.5f);
glVertex2f(0.5f, -0.5f);
glEnd();
```

However, for modern OpenGL, there are a lot more setups you need to do to draw a triangle. Primarily speaking, you need to:

* Create a vertex buffer - array of bytes of GPU memory(VRAM)
* Create a shader

**The basic concept is that we define a bunch of data which represents the triangle and put it into the GPU's VRAM and then we want to issue a drawcall which is basically a draw command - read those data from VRAM and draw it on the screen, after that we need to tell the GPU how to interpret the data(interpret it as like a bunch of positions on the screen and draw them up into a triangle), that's what shader is.**

OpenGL specifically operates like a **state machine**. You set a series of states and then if you want to draw a triangle, which is very contextual, you do not need to pass all OpenGL needs to draw a triangle, in fact, OpenGL knows what it needs to draw a triangle because it is part of the state. **Basically, you just need to tell OpenGL to select a buffer, then select a shader and finally draw the triangle.** Based on which buffer and which shader you've selected, that's going to determine what triangle gets drawn and where etc.

Since our triangle will not morph and the data will not change in game, we just put the buffer outside the game loop, here is the code to *give OpenGL the triangle data*, you can go to [OpenGL API Documentation](docs.gl) for details of these function usage:

```cpp
	float positions[6] = {
		-0.5f, -0.5f,
		 0.f,   0.5f,
		 0.5f, -0.5f
	};

	unsigned int buffer;
	// Generate buffer object names(id)
	glGenBuffers(1, &buffer);
	// Bind(select) a named buffer object
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	// Creates and initializes a buffer object's data store
	glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(float), positions, GL_STATIC_DRAW);
```

Next is to *issue a drawcall to actually draw the triangle* by calling the following code inside the game loop:

```cpp
// You can call glDrawElements() instead for index buffers
glDrawArrays(GL_TRIANGLES, 0, 3);
```

Due to the state machine mechanism, if you bind no buffer by calling `glBindBuffer(GL_ARRAY_BUFFER, 0);`, it will not draw the triangle because you select something else(no buffer). Just imagine the layers in PS, if you select a layer and then draw something on that layer using a paint brush, it is going to affect that layer; however if you have nothing selected, obviously it is not going to affect the layer you are going to draw on.

The triangle will not show up when you run the program because we have not created a shader yet.

