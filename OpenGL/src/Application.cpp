#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

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
            ss[(int)type] << line.c_str() << '\n';
        }
    }

    return { ss[0].str(), ss[1].str() };
}

// compile shader function to avoid code duplication
// type is a GLEnum but well use its equivalent to avoid openGl built in types
static unsigned int CompileShader(unsigned int type, const std::string& source)
{
    unsigned int id = glCreateShader(type);
    const char* src = source.c_str();

    glShaderSource(id, 1, &src, nullptr);

    glCompileShader(id);

    int succ;
    glGetShaderiv(id, GL_COMPILE_STATUS, &succ);
    if (succ == GL_FALSE)
    {
        int ln; 
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &ln);
        
        char* message = (char*)alloca(ln * sizeof(char)); //since ln is variable we need alloca to still keep this cariable on the stack dynamicaly
        glGetShaderInfoLog(id, ln, &ln, message);
        std::cout << "Failed to compile " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader!" << std::endl;
        std::cout << message << std::endl;

        glDeleteShader(id);
        return 0;
    }

    return id;
}

// Function to compile shaders (static so it doesnt leak into other translation units (+/- cpp files)
// provide source code so opengl compiles it and links our shader code into a shader and return a unique identifier to sayd shader
static unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader) 
{
    unsigned int program = glCreateProgram();
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

    glAttachShader(program, vs);
    glAttachShader(program, fs);

    glLinkProgram(program);

    glValidateProgram(program);
    
    //we can delete intermediate shaders since they are already linked into a program
    glDeleteShader(vs);
    glDeleteShader(fs);

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

    /* glew needs a valid ogl rendering context */
    if (glewInit() != GLEW_OK)
        std::cout << "Error!" << std::endl;

    std::cout << glGetString(GL_VERSION) << std::endl;

    //Triangle x1, y1, x2, y2, x3, y3
    float positions[6] = {
        -0.5f, -0.5f,
         0.0f,  0.5f,
         0.5f, -0.5f 
    };

    // vertex buffer
    unsigned int buffer;
    glGenBuffers(1, &buffer); //end argument saves id of buffer
    glBindBuffer(GL_ARRAY_BUFFER, buffer); //select buffer for work, since it is a vertex buffer its just an array
    glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(float), positions, GL_STATIC_DRAW); // STATIC since the data will be modified once and used every frame

    //enable first vertex attrib array
    glEnableVertexAttribArray(0);

    // first attribute index 0, num elements in vertex, type of vertex data, already normalized (no need for it),  stride (only pos so 8 bytes), only one attribute so pointer offset is 0
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (const void*) 0);

    ShaderProgramSource source = ParseShader("res/shaders/Basic.shader");

    //location in shader must match with attribute index

    unsigned int shader = CreateShader(source.VertexSource, source.FragmentSource);

    glUseProgram(shader);

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
                              
    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    { 
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        //draw currently bound buffer
        glDrawArrays(GL_TRIANGLES, 0, 3);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glDeleteProgram(shader);

    glfwTerminate();
    return 0;
}