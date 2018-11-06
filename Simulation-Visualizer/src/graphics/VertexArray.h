/*
 * VertexArray.h
 *
 *  Created on: Jul 30, 2018
 *      Author: Samuel Tan
 */

#ifndef VERTEXARRAY_H_
#define VERTEXARRAY_H_

#include <GL/glew.h>
#include <string>
#include <vector>
#include "../graphics/ShaderProgram.h"
#include "../graphics/VertexBuffer.h"

namespace graphics
{

  class VertexArray
  {
    public:

      struct Attribute
      {
        std::string varName;
        GLenum dataType;
        GLuint numComponents;
      };

    private:
      static GLuint currentlyBound;
      std::vector<Attribute> attributes;
      GLuint ID, stride;

    public:
      VertexArray();
      ~VertexArray();

      void addAttribute(const std::string& varName, GLenum dataType, GLuint numComponents);
      void applyAttributesWithBuffer(const VertexBuffer& vb, const ShaderProgram& sp);

      void bind() const;
      void forceBind() const;
      void unbind() const;

    private:
      GLsizei getSizeOfType(GLenum dataType);
  };

} /* namespace render */

#endif /* VERTEXARRAY_H_ */
