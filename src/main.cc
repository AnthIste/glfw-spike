////////////////////////////////////////////////////////////////
// Example OpenGL Program
//
// Behavior:
//  - Opens a 640x640 window
//  - Renders a blue square in the center of the screen
//
// Based on the arcsynthesis tutorial introduction available
// at the following URL:
//
//   http://www.arcsynthesis.org/gltut/Basics/Tutorial%2001.html
//
////////////////////////////////////////////////////////////////

#define GLFW_INCLUDE_GL_3
#define GL_GLEXT_PROTOTYPES 1
#include <GLFW/glfw3.h>

#include <cstdlib>
#include <iostream>
#include <vector>
#include <algorithm>

#include "shader_utils.h"

using namespace std;

//--------------------------------------------------------------
// Configuration constants
//--------------------------------------------------------------

const char* WindowTitle = "Hello OpenGL!";
const int WindowWidth = 640;
const int WindowHeight = 640;

//--------------------------------------------------------------
// Shader definitions
//--------------------------------------------------------------

const char* VertexShaderFilename = "shaders/vertex/multiinput.glsl";
const char* FragmentShaderFilename = "shaders/fragment/multiinput.glsl";

//--------------------------------------------------------------
// Program function declarations
//--------------------------------------------------------------

static GLuint initialize_main_shaders();
static GLuint create_shader_program(const std::vector<GLuint> &shaderList);
static GLuint create_shader(GLenum eShaderType, const std::string &strShaderFile);
static GLuint initialize_vertex_buffer();
static void render_scene(GLuint shaderProgram);
static void window_size_callback(GLFWwindow* window, int width, int height);
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
static void error_callback(int error, const char* description);

//==============================================================
// Entry point
//==============================================================

int main()
{
    // Initialize error handler
    glfwSetErrorCallback(error_callback);

    // Initialize GLFW library
    if (!glfwInit())
    {
        exit(EXIT_FAILURE);
    }

    // Set up a windowed OpenGL window
    GLFWwindow* window = glfwCreateWindow(WindowWidth, WindowHeight, WindowTitle, NULL, NULL);

    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    // Configure window hookpoints
    glfwSetWindowSizeCallback(window, window_size_callback);
    glfwSetKeyCallback(window, key_callback);

    // Initialize OpenGL by creating a context
    glfwMakeContextCurrent(window);

    // Initialize OpenGL resources such as shaders
    GLuint mainShader = initialize_main_shaders();

    // Enter main window loop
    while (!glfwWindowShouldClose(window))
    {
        render_scene(mainShader);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

//--------------------------------------------------------------
// Scene composition and pipeline
//--------------------------------------------------------------

static void render_scene(GLuint shaderProgram)
{
    GLuint positionBufferObject;

    // Start from black
    // [These functions clear the current viewable area of the screen.
    // glClearColor sets the color to clear, while glClear with the
    // GL_COLOR_BUFFER_BIT value causes the image to be cleared with that color.]
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // We need to draw with shaders, NOT compatibility layer
    // [This function causes the given program to become the current program.
    // All rendering taking place after this call will use this program for
    // the various shader stages. If the program 0 is given, then no program
    // is current.]
    glUseProgram(shaderProgram);

    // Create a buffer of triangle data that will be rendered
    positionBufferObject = initialize_vertex_buffer();

    // Shove our vertex buffer into the OpenGL pipeline, by
    // telling OpenGL what format our data is in
    //
    // Position:
    // [These functions control vertex attribute arrays. glEnableVertexAttribArray
    // activates the given attribute index, glDisableVertexAttribArray deactivates
    // the given attribute index, and glVertexAttribPointer defines the format and
    // source location (buffer object) of the vertex data.]
    //
    // Colors:
    // [Since we have two pieces of data, we have two vertex attributes. For each
    // attribute, we must call glEnableVertexAttribArray to enable that particular
    // attribute]
    glBindBuffer(GL_ARRAY_BUFFER, positionBufferObject);

    // [The only difference in the two calls are which attribute location to send
    // he data to and the last parameter. The last parameter is the byte offset
    // into the buffer of where the data for this attribute starts]
    // [he array takes its data from bufferObject because this was the buffer
    // object that was bound at the time that glVertexAttribPointer]
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(GLfloat) * 4 * 3));

    // Actually interpret the vertex buffer as triangles
    // [The glDrawArrays function can be used to draw triangles,
    // using particular buffer objects as sources for vertex data
    // and the currently bound program object.]
    // [This function initiates rendering, using the currently active vertex
    // attributes and the current program object (among other state). It causes
    // a number of vertices to be pulled from the attribute arrays in order.]
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // Cleanup
    glDisableVertexAttribArray(0);
    glUseProgram(0);
}

//--------------------------------------------------------------
// Shader creation
//--------------------------------------------------------------

// [GLSL shaders are compiled into shader objects that represent the code to be executed
// for a single shader stage. These shader objects can be linked together to produce a
// program object, which represent all of the shader code to be executed during rendering.]
static GLuint initialize_main_shaders()
{
    GLuint program;
    std::vector<GLuint> shaderList;

    // A shader program is a linked collection of shader objects
    shaderList.push_back(create_shader(GL_VERTEX_SHADER, load_shader_from_file(VertexShaderFilename)));
    shaderList.push_back(create_shader(GL_FRAGMENT_SHADER, load_shader_from_file(FragmentShaderFilename)));

    // Create the "chunk" shader program
    program = create_shader_program(shaderList);

    // Clean up the shader objects used in setup, they are now
    // part of the program in OpenGL land
    std::for_each(shaderList.begin(), shaderList.end(), glDeleteShader);

    return program;
}

// Shader link stage
// [These functions create a working program object. glCreateProgram
// creates an empty program object. glAttachShader attaches a shader
// object to that program. Multiple calls attach multiple shader objects.
// glLinkProgram links all of the previously attached shaders into a
// complete program. glDetachShader is used to remove a shader object
// from the program object; this does not affect the behavior of the program.]
static GLuint create_shader_program(const std::vector<GLuint> &shaderList)
{
    // Create OpenGL object
    GLuint program = glCreateProgram();

    // Tell OpenGL about our shader objects
    for(size_t iLoop = 0; iLoop < shaderList.size(); iLoop++)
        glAttachShader(program, shaderList[iLoop]);

    // Link them all into one program
    glLinkProgram(program);

    // Handle errors
    GLint status;
    glGetProgramiv (program, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
    {
        GLint infoLogLength;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);

        GLchar *strInfoLog = new GLchar[infoLogLength + 1];
        glGetProgramInfoLog(program, infoLogLength, NULL, strInfoLog);

        cerr <<  "Linker failure: " << strInfoLog << endl;
        delete[] strInfoLog;
    }

    // The shaders are linked already, we can tell OpenGL to
    // forget about them
    for(size_t iLoop = 0; iLoop < shaderList.size(); iLoop++)
        glDetachShader(program, shaderList[iLoop]);

    return program;
}

// Shader compile stage
// [These functions create a working shader object. glCreateShader simply
// creates an empty shader object of a particular shader stage.
// glShaderSource sets strings into that object; multiple calls to this
// function simply overwrite the previously set strings. glCompileShader
// causes the shader object to be compiled with the previously set strings.
// glDeleteShader causes the shader object to be deleted.]
static GLuint create_shader(GLenum eShaderType, const std::string &strShaderFile)
{
    // Create OpenGL object
    GLuint shader = glCreateShader(eShaderType);

    const char *strFileData = strShaderFile.c_str();
    glShaderSource(shader, 1, &strFileData, NULL);

    // Turn the text shader into a compiled binary object
    glCompileShader(shader);

    // Handle errors
    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        GLint infoLogLength;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);

        GLchar *strInfoLog = new GLchar[infoLogLength + 1];
        glGetShaderInfoLog(shader, infoLogLength, NULL, strInfoLog);

        const char *strShaderType = NULL;
        switch(eShaderType)
        {
            case GL_VERTEX_SHADER: strShaderType = "vertex"; break;
            case GL_GEOMETRY_SHADER: strShaderType = "geometry"; break;
            case GL_FRAGMENT_SHADER: strShaderType = "fragment"; break;
        }

        cerr <<  "Compile failure in " << strShaderType << " shader:" << endl << strInfoLog << "%s" << endl;
        delete[] strInfoLog;
    }

    return shader;
}

//--------------------------------------------------------------
// Triangle data (initialize vertex buffer for rendering)
//--------------------------------------------------------------

// Vertex data specified as (x, y, z, w).
// Z must be constrained between [-1, 1].
// W must be 1.0 for the time being.
// The origin (0, 0, 0) is at the center of the screen,
// and (-1.0, -1.0, 0), (1.0, 1.0, 0) are the opposing
// corners. The y-axis scales bottom-to-top, and the
// x-axis scales left-to-right (like a math graph with
// the origin at the center of a piece of paper)
static GLuint initialize_vertex_buffer()
{
    GLuint bufferObject;
    const float vertexData[] = {
        // Triangle data:
         0.0f,    0.5f, 0.0f, 1.0f, // (Vec4)
         0.5f, -0.366f, 0.0f, 1.0f, // (Vec4)
        -0.5f, -0.366f, 0.0f, 1.0f, // (Vec4)

        // Color data:
         1.0f,    0.0f, 0.0f, 1.0f, // (Vec4)
         0.0f,    1.0f, 0.0f, 1.0f, // (Vec4)
         0.0f,    0.0f, 1.0f, 1.0f, // (Vec4)
    };

    // Tell OpenGL we want an object (identified by a GLuint)
    // [Buffer objects are linear arrays of memory allocated by OpenGL.
    // They can be used to store vertex data.]
    glGenBuffers(1, &bufferObject);

    // Map this object to the GL_ARRAY_BUFFER object in the
    // OpenGL context state. Copy our vertex data into the
    // buffer, then reset the state to how it was before.
    // Now OpenGL knows about our vertex data identified by the object.
    // [These functions are used to create and manipulate buffer objects.
    // glGenBuffers creates one or more buffers, glBindBuffer attaches it
    // to a location in the context, and glBufferData allocates memory and
    // fills this memory with data from the user into the buffer object.]
    glBindBuffer(GL_ARRAY_BUFFER, bufferObject);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return bufferObject;
}

//--------------------------------------------------------------
// GLFW utilities
//--------------------------------------------------------------

static void window_size_callback(GLFWwindow* window, int width, int height)
{
    // [This function defines the current viewport transform. It defines as a
    // region of the window, specified by the bottom-left position and a width/height.]
    glViewport(0, 0, (GLsizei) width, (GLsizei) height);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
}

static void error_callback(int error, const char* description)
{
    cerr << description << endl;
}
