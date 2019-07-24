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

	  // Remember that this only takes effect on calling Renderer::renderAndClearAll()
	  static void setDepthWritingEnabled(bool enable);

	  void renderMarkerDot(const glm::vec3 & pos, const glm::vec4 & color = glm::vec4(0.0f));

	  // Remember that this only takes effect on calling Renderer::renderAndClearAll()
	  // Also enables/disables depth writing so that's a thing to keep in mind
	  static void setDepthTestEnabled(bool enable);
      static void draw(GLenum mode, const VertexArray& va,
          const IndexBuffer& ib, const ShaderProgram& sp);

	  void updateCamera(glm::mat4 VPMatrix);

	  void renderAndClearAll(bool enableDepthTest = true, bool enableDepthWriting = true);

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
