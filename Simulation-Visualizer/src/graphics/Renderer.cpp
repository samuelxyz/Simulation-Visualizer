/*
 * Renderer.cpp
 *
 *  Created on: Jul 30, 2018
 *      Author: Samuel Tan
 */

#include "core/Definitions.h"
#include "graphics/Renderer.h"
#include "graphics/IndexBuffer.h"
#include "graphics/VertexArray.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

namespace graphics
{

  Renderer::Renderer()
  : shaderProgram("resources/shaders/MotLB_generic.glsl"),
    tBatch(50000, shaderProgram)
  {
    //shaderProgram.bind(); // Seems to be unnecessary
  }

  Renderer::~Renderer()
  {
  }

  void Renderer::clearScreen()
  {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }

  void Renderer::clearDepth()
  {
	  glClear(GL_DEPTH_BUFFER_BIT);
  }

  void Renderer::draw(GLenum mode, const VertexArray& va,
      const IndexBuffer& ib, const ShaderProgram& sp)
  {
    va.bind();
    ib.bind();
	//ib.forceBind();
    sp.bind();
    glDrawElements(mode, ib.getNumIndices(), GL_UNSIGNED_INT, nullptr);
  }

  void Renderer::updateCamera(glm::mat4 VPMatrix)
  {
	  shaderProgram.bind();
	  shaderProgram.setUniformMat4f("u_VPMatrix", VPMatrix);
  }

  void Renderer::renderAndClearAll()
  {
    //clearScreen();
    tBatch.renderAndClearAll();
  }

  void Renderer::submit(const graphics::Triangle& tri)
  {
    tBatch.submit(tri);
  }

  void Renderer::submit(const graphics::Quad& quad)
  {
    tBatch.submit(quad);
  }

  void Renderer::submit(const graphics::CenteredPoly& cp)
  {
    tBatch.submit(cp);
  }

  void Renderer::submit(const std::vector<graphics::ColoredVertex>& vertices, const std::vector<int>& indices)
  {
	  tBatch.submit(vertices, indices);
  }

} /* namespace render */
