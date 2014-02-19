#define GLFW_INCLUDE_GL_3
#define GL_GLEXT_PROTOTYPES 1
#include <GLFW/glfw3.h>

#include <cstdlib>
#include <iostream>
#include <vector>
#include <algorithm>

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

const char* VertexShaderString =
"#version 330\n\
\n\
layout(location = 0) in vec4 position;\n\
void main()\n\
{\n\
    gl_Position = position;\n\
}"
;

const char* FragmentShaderString =
"#version 330\n\
\n\
out vec4 outputColor;\n\
void main()\n\
{\n\
   outputColor = vec4(0.0f, 0.0f, 1.0f, 1.0f);\n\
}"
;

//--------------------------------------------------------------
// Program functions
//--------------------------------------------------------------

static GLuint initialize_main_shaders();
static GLuint create_shader_program(const std::vector<GLuint> &shaderList);
static GLuint create_shader(GLenum eShaderType, const std::string &strShaderFile);
static void initialize_vertex_buffer(GLuint& bufferObject);
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
    shaderList.push_back(create_shader(GL_VERTEX_SHADER, VertexShaderString));
    shaderList.push_back(create_shader(GL_FRAGMENT_SHADER, FragmentShaderString));

    // Create the "chunk" shader program
    program = create_shader_program(shaderList);

    // Clean up the shader objects used in setup, they are now
    // part of the program in OpenGL land
    std::for_each(shaderList.begin(), shaderList.end(), glDeleteShader);

    return program;
}

// Shader link stage
static GLuint create_shader_program(const std::vector<GLuint> &shaderList)
{
    GLuint program = glCreateProgram();

    for(size_t iLoop = 0; iLoop < shaderList.size(); iLoop++)
        glAttachShader(program, shaderList[iLoop]);

    glLinkProgram(program);

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

    for(size_t iLoop = 0; iLoop < shaderList.size(); iLoop++)
        glDetachShader(program, shaderList[iLoop]);

    return program;
}

// Shader compile stage
static GLuint create_shader(GLenum eShaderType, const std::string &strShaderFile)
{
    GLuint shader = glCreateShader(eShaderType);
    const char *strFileData = strShaderFile.c_str();
    glShaderSource(shader, 1, &strFileData, NULL);

    glCompileShader(shader);

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
// Triangle data
//--------------------------------------------------------------

// Vertex data specified as (x, y, z, w).
// Z must be constrained between [-1, 1].
// W must be 1.0 for the time being.
// The origin (0, 0, 0) is at the center of the screen,
// and (-1.0, -1.0, 0), (1.0, 1.0, 0) are the opposing
// corners. The y-axis scales bottom-to-top, and the
// x-axis scales left-to-right (like a math graph with
// the origin at the center of a piece of paper)
static void initialize_vertex_buffer(GLuint& bufferObject)
{
    const float vertexPositions[] = {
         0.00f,  0.50f,  0.0f,  1.0f,
         0.00f,  0.00f,  0.0f,  1.0f,
         0.50f,  0.00f,  0.0f,  1.0f,

         0.00f, -0.50f,  0.0f,  1.0f,

        -0.50f,  0.00f,  0.0f,  1.0f,
    };

    // Tell OpenGL we want an object (identified by a GLuint)
    // [Buffer objects are linear arrays of memory allocated by OpenGL.
    // They can be used to store vertex data.]
    glGenBuffers(1, &bufferObject);

    // Map this object to the GL_ARRAY_BUFFER object in the
    // OpenGL context state. Copy our vertex data into the
    // buffer, then reset the state to how it was before.
    // Now OpenGL knows about our vertex data identified by the object.
    glBindBuffer(GL_ARRAY_BUFFER, bufferObject);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexPositions), vertexPositions, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

//--------------------------------------------------------------
// Scene composition and pipeline
//--------------------------------------------------------------

static void render_scene(GLuint shaderProgram)
{
    GLuint positionBufferObject;
    initialize_vertex_buffer(positionBufferObject);

    // Start from black
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // We need to draw with shaders, NOT compatibility layer
    glUseProgram(shaderProgram);

    // Shove our vertex buffer into the OpenGL pipeline, by
    // telling OpenGL what format our data is in
    glBindBuffer(GL_ARRAY_BUFFER, positionBufferObject);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

    // Actually interpret the vertex buffer as triangles
    // [The glDrawArrays function can be used to draw triangles,
    // using particular buffer objects as sources for vertex data
    // and the currently bound program object.]
    glDrawArrays(GL_TRIANGLE_FAN, 0, 5);

    // Cleanup
    glDisableVertexAttribArray(0);
    glUseProgram(0);
}

//--------------------------------------------------------------
// GLFW utilities
//--------------------------------------------------------------

static void window_size_callback(GLFWwindow* window, int width, int height)
{
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
