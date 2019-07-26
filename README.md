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

    ![image](https://github.com/hls333555/OpenGL/blob/master/images/MSDN.png)

    Add that to *Additional Dependencies* and the related issues should go away

* After fixing all other issues using the above method, you will see a black window like below when you run the program:

  ![image](https://github.com/hls333555/OpenGL/blob/master/images/OpenGL_window.png)

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

  ![image](https://github.com/hls333555/OpenGL/blob/master/images/GLEW_STATIC.png)

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

**The basic concept is that we define a bunch of data which represents the triangle and put it into the GPU's VRAM and then we want to issue a draw call which is basically a draw command - read those data from VRAM and draw it on the screen, after that we need to tell the GPU how to interpret the data(interpret it as like a bunch of positions on the screen and draw them up into a triangle), that's what shader is.**

OpenGL specifically operates like a **state machine**. You set a series of states and then if you want to draw a triangle, which is very contextual, you do not need to pass all OpenGL needs to draw a triangle, in fact, OpenGL knows what it needs to draw a triangle because it is part of the state. **Basically, you just need to tell OpenGL to select a buffer, then select a shader and finally draw the triangle.** Based on which buffer and which shader you've selected, that's going to determine what triangle gets drawn and where etc.

Since our triangle will not morph and the data will not change in game, we just put the buffer outside the game loop, here is the code to *give OpenGL the triangle data*, you can go to [OpenGL API Documentation](docs.gl) for those detail usages:

```cpp
float positions[6] = {
    -0.5f, -0.5f,
     0.f,   0.5f,
     0.5f, -0.5f
};

// Vertex buffer object
unsigned int vbo;
// Generate vertex buffer object names
glGenBuffers(1, &vbo);
// Bind a named vertex buffer object
glBindBuffer(GL_ARRAY_BUFFER, vbo);
// Create and initialize a vertex buffer object's data store
glBufferData(GL_ARRAY_BUFFER, 3 * 2 * sizeof(float), positions, GL_STATIC_DRAW);
```

Next is to *issue a draw call to actually draw the triangle* by calling the following code inside the game loop:

```cpp
// Issue a draw call
glDrawArrays(GL_TRIANGLES, 0, 3);
```

Due to the state machine mechanism, if you bind no buffer by calling `glBindBuffer(GL_ARRAY_BUFFER, 0);`, it will not draw the triangle because you select something else(no buffer). Just imagine the layers in PS, if you select a layer and then draw something on that layer using a paint brush, it is going to affect that layer; however if you have nothing selected, obviously it is not going to affect the layer you are going to draw on.

The triangle will not show up when you run the program because we have not created a shader yet.

## Vertex Attributes and Layouts in OpenGL

A vertex is just more than a position, it contains many other data including texture coordinates, normals, colors, tangents etc., those data are called attributes. In order to tell OpenGL what the layout of our buffer actually is, we can call `glVertexAttribPointer()`, you can go to [glVertexAttribPointer](http://docs.gl/gl4/glVertexAttribPointer) for its detail usage.

Parameter notes:

* Stride - It is **the amount of bytes between each vertex**. Imagine that we have a pointer at the beginning to the buffer, and we go 32 bytes into it and we should be at the very start of the next vertex.

  ![image](https://github.com/hls333555/OpenGL/blob/master/images/Stride.jpg)

* Pointer - It is **the offset of current attribute in bytes in a vertex**. In the case illustrated above, the offset of Position attribute would be 0, the offset of TextureCoordinate attribute would 12, while the offset of Normal attribute would be 20.

Below are the code to enable and define a vertex attribute data:

```cpp
// Enable the position vertex attribute data
glEnableVertexAttribArray(0); // Do not forget this!
// Define a position vertex attribute data
glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (const void*)0);
```

## How Shaders Work in OpenGL

The most commonly used shaders are **vertex shaders** and **fragment shaders**(aka **pixel shaders**), there are also other types of shaders including tessellation shaders, geometry shaders etc. 

For simplicity, when a draw call is issued, the vertex shader will get called **once for each vertex that were trying to render**, then the fragment shader will get called **once for each pixel in the triangle that needs to get filled in**, then you will see the result on the screen.

**The primary purpose of the vertex shader is to tell OpenGL where the vertex to be in the screen space.** It has nothing to do with color or lighting etc.

**The primary purpose of the fragment shader is to determine which color the pixel is supposed to be.**

## Writing a Shader in OpenGL

Here are the main steps on how to create a vertex shader:

### Shader Part

* Construct a shader source, here we use C++11 raw string literals:

  ```cpp
  std::string vertexShaderSource = R"(
  	#version 330 core
  	
  	// The location of the position vertex attribute data is 0
  	layout(location = 0) in vec4 position;
  	void main()
  	{
  		gl_Position = position;
  	}
  )";
  ```

* Create a shader object:

  ```cpp
  unsigned int vsId = glCreateShader(GL_VERTEX_SHADER);
  ```

* Load the shader source into the shader object:

  ```cpp
  const char* src = vertexShaderSource.c_str();
  glShaderSource(vsId, 1, &src, nullptr);
  ```

* Compile the shader object:

  ```cpp
  glCompileShader(vsId);
  
  int result;
  glGetShaderiv(vsId, GL_COMPILE_STATUS, &result);
  if(result == GL_FALSE)
  {
      int length;
      glGetShaderiv(vsId, GL_INFO_LOG_LENGTH, &length);
      // Allocate on the stack dynamically
      char* message = (char*)_malloca(length * sizeof(char));
      // Return the information log for the shader object
      glGetShaderInfoLog(vsId,length, &length, message);
      std::cout << "Failed to compile vertex shader!" << std::endl;
      std::cout << message << std::endl;
      glDeleteShader(vsId);
      return 0;
  }
  ```

### Program Part

* Create the program object:

  ```cpp
  unsigned int program = glCreateProgram();
  ```

* Attach shaders to the program object:

  ```cpp
  glAttachShader(program, vsId);
  ```

* Link the program:

  ```cpp
  glLinkProgram(program);
  ```

* Validate the program object:

  ```cpp
  // Check to see whether the executables contained in program can execute given the current OpenGL state
  glValidateProgram(program);
  
  int result;
  glGetProgramiv(program, GL_VALIDATE_STATUS, &result);
  if(result == GL_FALSE)
  {
      std::cout << "Failed to validate program!" << std::endl;
      return 0;
  }
  ```

* Use the program object:

  ```cpp
  // Install the program object as part of current rendering state
  glUseProgram(program); // Do not forget this!
  ```

* Delete the program object at last:

  ```cpp
  glDeleteProgram(program);
  ```


## Index Buffers in OpenGL

Consider drawing a square, you need to draw two triangles which make up a square. If we draw it like below, you will notice that there are two vertices actually being drawn twice which is a waste of memory.

![image](https://github.com/hls333555/OpenGL/blob/master/images/Square.png)

We can use index buffer to solve this. An index buffer is something that **allows to reuse existing vertices**.

Here are the main steps:

* Remove any duplicate vertices from the vertex array, now the array should look something like this:

  ```cpp
  float positions[] = {
      -0.5f, -0.5f, // 0
       0.5f, -0.5f, // 1
       0.5f,  0.5f, // 2
      -0.5f,  0.5f  // 3
  };
  ```

* Add an index array which contains **index of vertex** according to the above vertex array to draw each triangle:

  ```cpp
  unsigned int indices[] = {
      0, 1, 2,
      2, 3, 0
  };
  ```

* Replace

  ```cpp
  glBufferData(GL_ARRAY_BUFFER, 3 * 2 * sizeof(float), positions, GL_STATIC_DRAW);
  ```

  with:

  ```cpp
  glBufferData(GL_ARRAY_BUFFER, 4 * 2 * sizeof(float), positions, GL_STATIC_DRAW);
  ```

* Send the index buffer to the GPU, note there are several differences from vertex buffer version:

  ```cpp
  // Index buffer object
  unsigned int ibo;
  // Generate index buffer object names
  glGenBuffers(1, &ibo);
  // Bind a named index buffer object
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
  // Create and initialize an index buffer object's data store.
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), indices, GL_STATIC_DRAW);
  ```

* Finally, replace

  ```cpp
  glDrawArrays(GL_TRIANGLES, 0, 3);
  ```

  with:

  ```cpp
  // The count is actually the number of indices rather than vertices
  // Since index buffer is already bound to GL_ELEMENT_ARRAY_BUFFER, we do not need to specify the pointer to indices
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
  ```

That's it! Here is the final result:

![image](https://github.com/hls333555/OpenGL/blob/master/images/Square_window.png)

## Dealing with Errors in OpenGL

There are mainly two ways to deal with errors in OpenGL:

* `glGetError()`: which is **compatible with all OpenGL versions** and works in a very simple way: whenever we call an OpenGL function, if an error were to occur, a flag gets set internally in OpenGL's memory, when we call `glGetError()`, that gives our error flag.

  **The typical workflow with `glGetError()` is, first of all, before each OpenGL function call, clear all pending errors via a while loop calling `glGetError()`, then call the certain OpenGL function you want to debug, and then call `glGetError()` again, this can retrieve any errors that generated in the previous function call.**

  > To allow for distributed implementations, there may be several error flags. If any single error flag has recorded an error, the value of that flag is returned and that flag is reset to `GL_NO_ERROR` when `glGetError` is called. If more than one flag has recorded an error, `glGetError` returns and clears an arbitrary error flag value. Thus, `glGetError` should always be called in a loop, until it returns `GL_NO_ERROR`, if all error flags are to be reset.

* `glDebugMessageCallback()`: which is **only available since OpenGL 4.3** and allows us to specify a function pointer to OpenGL and OpenGL will call that function when an error occurs. This is much better than `glGetError()` obviously.

Here, we use the first method to track our error:

```cpp
// Note that we must put brackets around x!
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
```

To use this for an OpenGL function call, just wrap those functions with `GLCALL()` like below:

```cpp
GLCALL(glDrawElements(GL_TRIANGLES, 6, GL_INT, nullptr));
```

When pressing F5 in debug mode, you will see a breakpoint being triggered if an error occurs in that call.

## Uniforms in OpenGL

**Uniforms are a way to get data from the CPU side(C++ in the case) into the shader.**

Here are the main steps to use uniforms in OpenGL:

* Add a variable into shader:

  ```cpp
  uniform vec4 u_Color;
  ```

* Below `glUseProgram();`, add the following code:

  ```cpp
  int location = glGetUniformLocation(program, "u_Color");
  // If uColor is not used in the fragment shader, it will return -1
  //ASSERT(location != -1);
  glUniform4f(location, 0.f, 1.f, 1.f, 1.f);
  ```

You can see an example in code using uniforms to achieve a flashing effect like below:

![image](https://github.com/hls333555/OpenGL/blob/master/images/Uniforms.gif)

## Vertex Arrays in OpenGL

**Vertex arrays are a way to bind vertex buffers with a certain specification for layout.** Here's what VAO is explained in [Vertex Array Objects](http://ogldev.atspace.co.uk/www/tutorial32/tutorial32.html):

> The Vertex Array Object (a.k.a VAO) is a special type of object that encapsulates all the data that is associated with the vertex processor. Instead of containing the actual data, it holds references to the vertex buffers, the index buffer and the layout specification of the vertex itself. The advantage is that once you set up the VAO for a mesh you can bring in the entire mesh state by simply binding the VAO. After that you can render the mesh object and you don't need to worry about all of its state. The VAO remembers it for you. If your application needs to deal with meshes whose vertex layout slightly differs from one another the VAO takes care of it also. Just make sure to set up the correct layout when you create the VAO and forget about it. From now on it "sticks" to the VAO and becomes active whenever that VAO is used.

For now, what we've done is we bind all the objects(program object, vbo, ibo) we have for one shader, one vertex buffer and one index buffer. If we have multiple objects(multiple vbos etc.), we will have to rebind everything we need for current draw every times(in the game loop):

```cpp
// Bind shader and set up uniforms
glUseProgram(program);
glUniform4f(location, r, 1.f, 1.f, 1.f);

// Bind vertex buffer and set up the layout
glBindBuffer(GL_ARRAY_BUFFER, vbo);
glEnableVertexAttribArray(0);
glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (const void*)0);

// Bind index buffer
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
```

VAOs are actually what contain the state. If you utilize VAOs properly, for example, make a different VAO for each piece of geometry of each draw call, then what you need to do is to bind the VAO, that's it! Because VAO will contain a binding between the vertex buffer(s) and that vertex layout.

Let's utilize VAO to rewrite the above code:

* Add the following code before `glfwCreateWindow()` to use the core OpenGL profile:

  ```cpp
  // Use OpenGL 3.3
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  // Use core OpenGL profile which will not make VAO object 0 an object
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  ```

  If you press F5, an assert will be triggered like below as long as you wrap all gl functions with `GLCALL()`:

  ![image](https://github.com/hls333555/OpenGL/blob/master/images/VAO_Assert.png)

  This is because no VAO objects are found in the core OpenGL profile, which is stated in [VertexArrayObject](https://www.khronos.org/opengl/wiki/Vertex_Specification#Vertex_Array_Object):

  > The compatibility OpenGL profile makes VAO object 0 a default object. The core OpenGL profile makes VAO object 0 not an object at all. So if VAO 0 is bound in the core profile, you should not call any function that modifies VAO state. This includes binding the `GL_ELEMENT_ARRAY_BUFFER` with [glBindBuffer](https://www.khronos.org/opengl/wiki/GLAPI/glBindBuffer).

* To fix that, simply create a VAO:

  ```cpp
  // Vertex array object
  unsigned int vao;
  // Generate vertex array object names
  glGenVertexArrays(1, &vao);
  // Bind a vertex array object
  glBindVertexArray(vao);
  ```

  When you run the program, everything should work fine now!

* In fact, the following lines of code in the game loop are no longer needed since a VAO is created and bound and vertex information is linked to this VAO later:

  ```cpp
  // Bind vertex buffer and set up the layout
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (const void*)0);
  ```

  Therefore, the above code can be replaced with the following line:

  ```cpp
  // Bind vertex array
  glBindVertexArray(vao);
  ```

## Textures in OpenGL

We will use a library called *stb_image* which can be grabbed from [stb library](https://github.com/nothings/stb) to load the png file.

**Basically, what it's going to do is to return a pointer to a buffer of RGBA pixels after giving it a file path. Then, we can take that pixel array and upload it to the GPU via OpenGL as a texture. After that, we can modify the shader to read the texture when it is drawing.**

Here are the main steps:

* Add stb_image.h to the project and create stb_image.cpp containing the following code:

  ```cpp
  // This is required above #include "stb_image.h", see stb_image.h for details
  #define STB_IMAGE_IMPLEMENTATION
  #include "stb_image.h"
  ```

* Load the png file:

  ```cpp
  // You should assign a png file path
  std::string filePath;
  unsigned char* localBuffer;
  int width, height, BPP;
  // OpenGL expects the texture pixels to start at the bottom-left(0,0) instead of the top-left
  // Typically, when png image is being loaded, it is stored in scanlines from the top to the bottom of the image, so we need to flip it on load
  stbi_set_flip_vertically_on_load(1);
  localBuffer = stbi_load(filePath.c_str(), &width, &height, &BPP, 4/*RGBA*/);
  ```

* Create the texture and set its parameters:

  ```cpp
  unsigned int texture;
  // Generate texture names
  glGenTextures(1, &texture);
  // Bind a named texture to a texturing target, you can specify the slot using 	glActiveTexture() in advance
  glBindTexture(GL_TEXTURE_2D, texture);
  
  // Set texture parameters
  // This is the minification filter that how the texture will be resampled down if it needs to be rendered smaller per pixel
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  ```

* Send OpenGL the texture data:

  ```cpp
  // Send OpenGL the texture data
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8/*8-bits per channel*/, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, localBuffer);
  // Unbind textures from a texturing target
  glBindTexture(GL_TEXTURE_2D, 0);
  ```

* Free the local buffer when you don't need it:

  ```cpp
  if (localBuffer)
  {
      // Free the local buffer
      stbi_image_free(localBuffer);
  }
  ```

* Bind texture to a slot and tell the shader:

  ```cpp
  // Select active texture unit(slot)
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture);
  
  int location = glGetUniformLocation(program, "u_Texture");
  // If u_Texture is not used in the fragment shader, it will return -1
  //ASSERT(location != -1);
  // Tell the shader which texture slot to sample from, the slot must be the same as the selected active texture slot
  glUniform1i(location, GL_TEXTURE0);
  ```

* Add **texture coordinates** to **tell the geometry being rendered which part of the texture to sample from**. Furthermore, when it's up to rendering a certain pixel, tell the shader to sample a certain area of the texture to retrieve what color the pixel should be.

  * Modify the vertex data array to accept texture coordinates:

    ```cpp
    // Two floats for vertex position and two floats for texture coordinate
    // For texture coordinate system, the bottom-left is (0,0), the top-right is (1,1)
    float positions[] = {
        -0.5f, -0.5f, 0.f, 0.f, // 0
         0.5f, -0.5f, 1.f, 0.f, // 1
         0.5f,  0.5f, 1.f, 1.f, // 2
        -0.5f,  0.5f, 0.f, 1.f  // 3
    };
    ```

  * Enable and define the texture coordinate vertex attribute data:

    ```cpp
    // Enable the texture coordinate vertex attribute data
    glEnableVertexAttribArray(1); // Do not forget this!
    // Define a texture coordinate vertex attribute data
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (const void*)8);
    ```

  * Replace

    ```cpp
    glBufferData(GL_ARRAY_BUFFER, 4 * 2 * sizeof(float), positions, GL_STATIC_DRAW);
    ```

    with:

    ```cpp
    glBufferData(GL_ARRAY_BUFFER, 4 * 4 * sizeof(float), positions, GL_STATIC_DRAW);
    ```

  * Next is to modify the shader code:

    * For the vertex shader part:
      * Pass in the texture coordinate vertex attribute data:

        ```
        layout(location = 1) in vec2 texCoord;
        ```

      * Define a variable to be passed out to the fragment shader:

        ```
        out vec2 v_texCoord;
        ```

      * Assign that variable in the `main()` function:

        ```
        v_texCoord = texCoord;
        ```

    * For the fragment shader part:

      * Pass in the texture coordinate from the vertex shader:

        ```
        in vec2 v_texCoord;
        ```

      * Define a variable to get texture via uniform:

        ```
        uniform sampler2D u_Texture;
        ```

      * Output the final color in the `main()` function:

        ```
        // Draw the pixel from the texture in the scene by knowing the precise location in the texture to look up
        color = texture(u_Texture, v_texCoord);
        ```

* That's it! You will now see a beautiful texture like below:

  ![image](https://github.com/hls333555/OpenGL/blob/master/images/Texture.png)


## Blending in OpenGL

If you replace the texture with a translucent one, you will see a dirty result something like below:

![image](https://github.com/hls333555/OpenGL/blob/master/images/Texture_Trans_NoBlending.png)

That's because blending is not set properly.

### What is blending?

![image](https://github.com/hls333555/OpenGL/blob/master/images/Blending.jpg)
Blending determines how we **combine** our **output** color with what is already in our **target buffer**.
**Output** = the color we output from our fragment shader (known as **source**)
**Target buffer** = the buffer our fragment shader is drawing to (known as **destination**)

### How do we control blending?

- `glEnable(GL_BLEND)`/ `glDisable(GL_BLEND)`
- `glBlendFunc(src, dest)`
  - src = how the src RGBA factor is computed (default is `GL_ONE`)
  - dest = how the dest RGBA factor is computed (default is `GL_ZERO`)
    The default  `glBlendFunc()` is to throw away the destination and overwrite with the source.
- `glBlendEquation(mode)`
  - mode = how we combine the src and dest colors (default is add)
    So what this means by default is:
  - 1 + 0 = 1
  - Use the source value

### How to draw a translucent texture properly?

- We used:

  - src = `GL_SRC_ALPHA`
  - dest = `GL_ONE_MINUS_SRC_ALPHA`

- So if the pixel we’re rendering from our texture is transparent:

  - src alpha = 0

  - dest = 1 - 0 = 1

  - Which means “Use the destination color” - the color that’s already in the buffer:
    
    ![image](https://latex.codecogs.com/png.latex?R%20%3D%20%28r_%7Bsrc%7D%20*%200%29%20+%20%28r_%7Bdest%7D%20*%20%281%20-%200%29%29%20%3D%20r_%7Bdest%7D%5C%5C%20G%20%3D%20%28g_%7Bsrc%7D%20*%200%29%20+%20%28g_%7Bdest%7D%20*%20%281%20-%200%29%29%20%3D%20g_%7Bdest%7D%5C%5C%20B%20%3D%20%28b_%7Bsrc%7D%20*%200%29%20+%20%28b_%7Bdest%7D%20*%20%281%20-%200%29%29%20%3D%20b_%7Bdest%7D%5C%5C%20A%20%3D%20%28a_%7Bsrc%7D%20*%200%29%20+%20%28a_%7Bdest%7D%20*%20%281%20-%200%29%29%20%3D%20a_%7Bdest%7D)
  
- Another example:

  - Our pixel is partially transparent, let’s say (1.0, 1.0, 1.0, 0.5) (RGBA):

  - So it’s white but translucent

  - Let’s say our destination buffer is cleared to magenta(1.0, 0.0, 1.0, 1.0)

  - So using our blending settings:
    
    ![image](https://latex.codecogs.com/png.latex?R%20%3D%20%281.0%20*%200.5%29%20+%20%281.0%20*%20%281%20-%200.5%29%29%20%3D%201.0%5C%5C%20G%20%3D%20%281.0%20*%200.5%29%20+%20%280.0%20*%20%281%20-%200.5%29%29%20%3D%200.5%5C%5C%20B%20%3D%20%281.0%20*%200.5%29%20+%20%281.0%20*%20%281%20-%200.5%29%29%20%3D%201.0%5C%5C%20A%20%3D%20%280.5%20*%200.5%29%20+%20%281.0%20*%20%281%20-%200.5%29%29%20%3D%200.75)

To fix the issue related to transparent png file, add the following code:

```cpp
glEnable(GL_BLEND);
glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
```

Here's the result with transparency being blended properly:

![image](https://github.com/hls333555/OpenGL/blob/master/images/Texture_Trans_Blending.png)