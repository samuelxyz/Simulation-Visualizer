/*
 * TriangleBatch.h
 *
 *  Created on: Aug 6, 2018
 *      Author: Samuel Tan
 */

#ifndef TRIANGLEBATCH_H_
#define TRIANGLEBATCH_H_

#include "graphics/IndexBuffer.h"
#include "graphics/ShaderProgram.h"
#include "graphics/VertexArray.h"
#include "graphics/VertexBuffer.h"
#include "graphics/Definitions.h"

namespace graphics
{

  class TriangleBatch
  {
    public:
      static constexpr unsigned short FLOATS_PER_VERTEX = 7;

    private:
      const unsigned int maxVertices;
      ShaderProgram& shaderProgram;
      VertexArray vertexArray;
      VertexBuffer vertexBuffer;
      IndexBuffer indexBuffer;

      unsigned int iIndex, vIndex, vertexCount;
      const unsigned int iLength, vLength;
      GLuint* iData;
      float* vData;

    public:
      TriangleBatch(unsigned int maxVertices, ShaderProgram& sp);
      ~TriangleBatch();

      bool submit(const graphics::Triangle&);
      bool submit(const graphics::Quad&);
      bool submit(const graphics::CenteredPoly&);
	  bool submit(const std::vector<graphics::ColoredVertex>&, const std::vector<int>&);

      void renderAndClearAll();
  };

} /* namespace graphics */

#endif /* TRIANGLEBATCH_H_ */
