/*
 * VertexBuffer.h
 *
 *  Created on: Jul 30, 2018
 *      Author: Samuel Tan
 */

#ifndef VERTEXBUFFER_H_
#define VERTEXBUFFER_H_

#include <GL/glew.h>

namespace graphics
{

  class VertexBuffer
  {
    private:
      static GLuint currentlyBound;
      GLuint ID;
      GLsizeiptr bufferSize;
      GLenum usage;

    public:
      VertexBuffer(const void* data, GLsizeiptr size, GLenum usage);
      ~VertexBuffer();

      void updateData(const void* data, GLsizeiptr size);
      void bind() const;
      void forceBind() const;
      void unbind() const;
  };

} /* namespace render */

#endif /* VERTEXBUFFER_H_ */
