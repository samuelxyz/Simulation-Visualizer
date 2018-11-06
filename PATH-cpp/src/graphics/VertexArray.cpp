/*
 * VertexArray.cpp
 *
 *  Created on: Jul 30, 2018
 *      Author: Samuel Tan
 */

#include "../graphics/VertexArray.h"

#include <cassert>

namespace graphics
{

  GLuint VertexArray::currentlyBound = 0;

  VertexArray::VertexArray()
  : stride(0)
  {
    glGenVertexArrays(1, &ID);
  }

  VertexArray::~VertexArray()
  {
    if (currentlyBound == ID)
      currentlyBound = 0;
    glDeleteVertexArrays(1, &ID);
  }

  void VertexArray::addAttribute(const std::string& varName, GLenum dataType, GLuint numComponents)
  {
    attributes.push_back({ varName, dataType, numComponents });
    stride += (numComponents * getSizeOfType(dataType));
  }

  void VertexArray::applyAttributesWithBuffer(const VertexBuffer& vb, const ShaderProgram& sp)
  {
    vb.bind();
    bind();

    unsigned int offset = 0;
    for (auto& attribute : attributes)
    {
      GLuint varPos = glGetAttribLocation(sp.getID(), attribute.varName.c_str());
      glEnableVertexAttribArray(varPos);
      glVertexAttribPointer(varPos, attribute.numComponents,
          attribute.dataType, GL_FALSE, stride, (const void*)offset);
      offset += attribute.numComponents * getSizeOfType(attribute.dataType);
    }
  }

  void VertexArray::bind() const
  {
    if (currentlyBound != ID)
      forceBind();
  }

  void VertexArray::forceBind() const
  {
    glBindVertexArray(ID);
    currentlyBound = ID;
  }

  void VertexArray::unbind() const
  {
    glBindVertexArray(0);
    currentlyBound = 0;
  }

  GLsizei VertexArray::getSizeOfType(GLenum dataType)
  {
    switch (dataType)
    {
      case GL_FLOAT:
        return sizeof(GLfloat);
      case GL_UNSIGNED_INT:
        return sizeof(GLuint);
      case GL_UNSIGNED_BYTE:
        return sizeof(GLbyte);
    }
    // should be done before here
    assert(false);
    return 0;
  }

} /* namespace render */
