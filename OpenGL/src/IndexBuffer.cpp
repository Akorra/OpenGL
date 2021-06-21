#include "IndexBuffer.h"

#include "Renderer.h"

IndexBuffer::IndexBuffer(const void* data, unsigned int size)
{
    GLCall(glGenBuffers(1, &m_RendererID)); //end argument saves id of buffer
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_RendererID)); //select buffer for work, since it is a Index buffer its just an array
    GLCall(glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW)); // STATIC since the data will be modified once and used every frame, 6*2 floats (6 vertices with x and y each)
}

IndexBuffer::~IndexBuffer()
{
    GLCall(glDeleteBuffers(1, &m_RendererID));
}

void IndexBuffer::Bind()
{
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_RendererID));
}

void IndexBuffer::Unbind()
{
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
}
