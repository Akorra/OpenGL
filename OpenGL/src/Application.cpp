#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Renderer.h"

struct ShaderProgramSource
{
    std::string VertexSource;
    std::string FragmentSource;
};

static ShaderProgramSource ParseShader(const std::string& filepath)
{
    std::ifstream stream(filepath);

    enum class ShaderType
    {
        NONE = -1, VERTEX = 0, FRAGMENT = 1
    };

    std::string line;
    std::stringstream ss[2]; //one for vertex other for fragment
    ShaderType type = ShaderType::NONE;
    while (getline(stream, line))
    {
        if (line.find("#shader") != std::string::npos)
        {
            if (line.find("vertex") != std::string::npos)
                type = ShaderType::VERTEX;
            else if (line.find("fragment") != std::string::npos)
                type = ShaderType::FRAGMENT;
        }
        else
        {
            ss[(int)type] << line << '\n';
        }
    }
     
    return { ss[0].str(), ss[1].str() };
}

// compile shader function to avoid code duplication
// type is a GLEnum but well use its equivalent to avoid openGl built in types
static unsigned int CompileShader(unsigned int type, const std::string& source)
{
    GLCall(unsigned int id = glCreateShader(type));
    const char* src = source.c_str();

    GLCall(glShaderSource(id, 1, &src, nullptr));

    GLCall(glCompileShader(id));

    int succ;
    GLCall(glGetShaderiv(id, GL_COMPILE_STATUS, &succ));
    if (succ == GL_FALSE)
    {
        int ln; 
        GLCall(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &ln));
        
        char* message = (char*)alloca(ln * sizeof(char)); //since ln is variable we need alloca to still keep this cariable on the stack dynamicaly
        GLCall(glGetShaderInfoLog(id, ln, &ln, message));
        std::cout << "Failed to compile " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader!" << std::endl;
        std::cout << message << std::endl;

        GLCall(glDeleteShader(id));
        return 0;
    }

    return id;
}

// Function to compile shaders (static so it doesnt leak into other translation units (+/- cpp files)
// provide source code so opengl compiles it and links our shader code into a shader and return a unique identifier to sayd shader
static unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader) 
{
    GLCall(unsigned int program = glCreateProgram());
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

    GLCall(glAttachShader(program, vs));
    GLCall(glAttachShader(program, fs));

    GLCall(glLinkProgram(program));

    GLCall(glValidateProgram(program));
    
    //we can delete intermediate shaders since they are already linked into a program
    GLCall(glDeleteShader(vs));
    GLCall(glDeleteShader(fs));

    return program;
}

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

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

    /* glew needs a valid ogl rendering context */
    if (glewInit() != GLEW_OK)
        std::cout << "Error!" << std::endl;

    std::cout << glGetString(GL_VERSION) << std::endl;

    //Triangle x1, y1, x2, y2, x3, y3
    float positions[] = {
        -0.5f, -0.5f, //0
         0.5f, -0.5f, //1
         0.5f,  0.5f, //2
        -0.5f,  0.5f  //3
    };

    unsigned int indices[] = {
        0, 1, 2, //triangle 1
        2, 3, 0  //triangle 2
    };

    unsigned int vao;
    GLCall(glGenVertexArrays(1, &vao));
    GLCall(glBindVertexArray(vao));

    // vertex buffer
    unsigned int buffer;
    GLCall(glGenBuffers(1, &buffer)); //end argument saves id of buffer
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, buffer)); //select buffer for work, since it is a vertex buffer its just an array
    GLCall(glBufferData(GL_ARRAY_BUFFER, 4 * 2 * sizeof(float), positions, GL_STATIC_DRAW)); // STATIC since the data will be modified once and used every frame, 6*2 floats (6 vertices with x and y each)

    //enable first vertex attrib array
    GLCall(glEnableVertexAttribArray(0));

    // first attribute index 0, num elements in vertex, type of vertex data, already normalized (no need for it),  stride (only pos so 8 bytes), only one attribute so pointer offset is 0
    GLCall(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (const void*) 0));

    // index buffer
    unsigned int ibo;
    GLCall(glGenBuffers(1, &ibo)); //end argument saves id of ibo
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo)); //select buffer for work, since it is a vertex buffer its just an array
    GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), indices, GL_STATIC_DRAW)); // STATIC since the data will be modified once and used every frame, 6 indices

    ShaderProgramSource source = ParseShader("res/shaders/Basic.shader");

    //location in shader must match with attribute index

    unsigned int shader = CreateShader(source.VertexSource, source.FragmentSource);

    GLCall(glUseProgram(shader));

    GLCall(int location = glGetUniformLocation(shader, "u_Color")); //get uniform location
    ASSERT(location != -1);

    //Unbind everything
    GLCall(glBindVertexArray(0));
    GLCall(glUseProgram(0));
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));

    float r = 0.0f;
    float increment = 0.05f;

    GLCall(glClearColor(0.13f, 0.13f, 0.13f, 1.0f));
                              
    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    { 
        GLCall(glUseProgram(shader));
        GLCall(glUniform4f(location, r, 0.3f, 0.8f, 1.0f));

        GLCall(glBindVertexArray(vao));
        GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo));

        /* Render here */
        GLCall(glClear(GL_COLOR_BUFFER_BIT));

        //draw currently bound buffer, 6 indices
        GLCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));

        if (r > 1.0f)
            increment = -0.05f;
        else if (r < 0.0f)
            increment = 0.05f;

        r += increment;

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    GLCall(glDeleteProgram(shader));

    glfwTerminate();
    return 0;
}