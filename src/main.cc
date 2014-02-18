#define GLFW_INCLUDE_GL_3
#define GL_GLEXT_PROTOTYPES 1
#include <GLFW/glfw3.h>

#include <cstdlib>
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

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
   outputColor = vec4(0.0f, 1.0f, 0.0f, 1.0f);\n\
}"
;

static GLuint initialize_program();
static GLuint create_program(const std::vector<GLuint> &shaderList);
static GLuint create_shader(GLenum eShaderType, const std::string &strShaderFile);
static void initialize_vertex_buffer(GLuint& bufferObject);
static void render(GLuint shaderProgram);
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
static void error_callback(int error, const char* description);

int main()
{
    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
    {
        exit(EXIT_FAILURE);
    }

    GLFWwindow* window = glfwCreateWindow(640, 480, "Hello OpenGL!", NULL, NULL);

    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, key_callback);

    glfwMakeContextCurrent(window);

    GLuint mainShader = initialize_program();

    while (!glfwWindowShouldClose(window))
    {
        render(mainShader);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

static GLuint initialize_program()
{
    GLuint program;
    std::vector<GLuint> shaderList;

    shaderList.push_back(create_shader(GL_VERTEX_SHADER, VertexShaderString));
    shaderList.push_back(create_shader(GL_FRAGMENT_SHADER, FragmentShaderString));

    program = create_program(shaderList);

    std::for_each(shaderList.begin(), shaderList.end(), glDeleteShader);

    return program;
}

static GLuint create_program(const std::vector<GLuint> &shaderList)
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

static void initialize_vertex_buffer(GLuint& bufferObject)
{
    const float vertexPositions[] = {
        0.75f, 0.75f, 0.0f, 1.0f,
        0.75f, -0.75f, 0.0f, 1.0f,
        -0.75f, -0.75f, 0.0f, 1.0f,
    };

    glGenBuffers(1, &bufferObject);

    glBindBuffer(GL_ARRAY_BUFFER, bufferObject);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexPositions), vertexPositions, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

static void render(GLuint shaderProgram)
{
    GLuint positionBufferObject;
    initialize_vertex_buffer(positionBufferObject);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shaderProgram);

    glBindBuffer(GL_ARRAY_BUFFER, positionBufferObject);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

    glDrawArrays(GL_TRIANGLES, 0, 3);

    glDisableVertexAttribArray(0);
    glUseProgram(0);
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
