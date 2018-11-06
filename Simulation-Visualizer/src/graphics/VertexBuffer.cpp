/*
 * VertexBuffer.cpp
 *
 *  Created on: Jul 30, 2018
 *      Author: Samuel Tan
 */

#include "../graphics/VertexBuffer.h"

namespace graphics
{
  GLuint VertexBuffer::currentlyBound = 0;

  VertexBuffer::VertexBuffer(const void* data, GLsizeiptr size, GLenum usage)
  : bufferSize(size), usage(usage)
  {
    glGenBuffers(1, &ID);
    bind();
    glBufferData(GL_ARRAY_BUFFER, size, data, usage);
  }

  VertexBuffer::~VertexBuffer()
  {
    if (currentlyBound == ID)
      currentlyBound = 0;
    glDeleteBuffers(1, &ID);
  }

  void VertexBuffer::bind() const
  {
    if (currentlyBound != ID)
      forceBind();
  }

  void VertexBuffer::forceBind() const
  {
    glBindBuffer(GL_ARRAY_BUFFER, ID);
    currentlyBound = ID;
  }

  void VertexBuffer::unbind() const
  {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    currentlyBound = 0;
  }

  void VertexBuffer::updateData(const void* data, GLsizeiptr size)
  {
    bind();
//    forceBind();
    if (size > bufferSize)
    {
      glBufferData(GL_ARRAY_BUFFER, size, data, usage);
    }
    else
    {
      glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
    }
  }
} /* namespace render */

