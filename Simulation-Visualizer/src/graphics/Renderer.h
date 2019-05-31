/*
 * Renderer.h
 *
 *  Created on: Jul 30, 2018
 *      Author: Samuel Tan
 */

#ifndef RENDERER_H_
#define RENDERER_H_

#include "graphics/ShaderProgram.h"
#include "graphics/TriangleBatch.h"

namespace graphics
{
  class IndexBuffer;
  class VertexArray;
} /* namespace graphics */

namespace graphics
{

  class Renderer
  {
    public:
      Renderer();
      ~Renderer();

      static void clearScreen();
	  static void clearDepth();
      static void draw(GLenum mode, const VertexArray& va,
          const IndexBuffer& ib, const ShaderProgram& sp);

	  void updateCamera(glm::mat4 VPMatrix);

      void renderAndClearAll();

      void submit(const graphics::Triangle&);
      void submit(const graphics::Quad&);
      void submit(const graphics::CenteredPoly&);
	  void submit(const std::vector<graphics::ColoredVertex>&, const std::vector<int>&);

    private:
      ShaderProgram shaderProgram;
      TriangleBatch tBatch;
  };

} /* namespace graphics */

#endif /* RENDERER_H_ */
