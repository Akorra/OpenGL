#include "Renderer.h"

#include <iostream>

void GLClearError()
{
    while (glGetError() != GL_NO_ERROR); // GL_NO_ERROR is 0 so we could just check result instead of comparing
}

bool GLLogCall(const char* function, const char* file, int line)
{
    while (GLenum err = glGetError())
    {
        std::cout << "[OpenGL_Error] (" << err << "): " << function << " " << file << ": " << line << std::endl;
        return false;

    }
    return true;
}