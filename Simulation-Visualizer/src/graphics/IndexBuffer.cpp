/*
 * IndexBuffer.cpp
 *
 *  Created on: Jul 30, 2018
 *      Author: Samuel Tan
 */

#include <GL/glew.h>
#include "../graphics/IndexBuffer.h"

namespace graphics
{

  GLuint IndexBuffer::currentlyBound = 0;

  // if supplying data, make sure a VAO is bound first
  IndexBuffer::IndexBuffer(const GLuint* data, GLuint numIndices, GLenum usage)
  : numIndices(numIndices), usage(usage), bufferMade(false)
  {
    glGenBuffers(1, &ID);

    if (data)
    {
      updateData(data, numIndices);
    }
  }

  IndexBuffer::~IndexBuffer()
  {
    if (currentlyBound == ID)
      currentlyBound = 0;
    glDeleteBuffers(1, &ID);
  }

  // make sure a VAO is bound first
  void IndexBuffer::updateData(const GLuint* data, GLuint numIndices)
  {
    //bind();
    forceBind();

    if (numIndices > this->numIndices || !bufferMade)
    {
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices * sizeof(GLuint), data, usage);
      bufferMade = true;
    }
    else
    {
      glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, numIndices * sizeof(GLuint), data);
    }

//    glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices * sizeof(GLuint), nullptr, usage);
//    bufferMade = true;
//    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, numIndices * sizeof(GLuint), data);

  }

  void IndexBuffer::bind() const
  {
    if (currentlyBound != ID)
      forceBind();
  }

  void IndexBuffer::forceBind() const
  {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID);
    currentlyBound = ID;
  }

  void IndexBuffer::unbind() const
  {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    currentlyBound = 0;
  }

} /* namespace render */

