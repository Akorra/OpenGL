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

void Renderer::SetClearColor(float r, float g, float b, float a)
{
    GLCall(glClearColor(r, g, b, a));
}

void Renderer::Clear() const
{
    GLCall(glClear(GL_COLOR_BUFFER_BIT));
}

void Renderer::Draw(const VertexArray& va, const IndexBuffer& ib, const Shader& shader) const
{
    shader.Bind();
    va.Bind();
    ib.Bind();

    //draw currently bound buffer, 6 indices
    GLCall(glDrawElements(GL_TRIANGLES, ib.GetCount(), GL_UNSIGNED_INT, nullptr));
}
