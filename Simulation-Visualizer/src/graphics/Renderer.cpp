/*
 * Renderer.cpp
 *
 *  Created on: Jul 30, 2018
 *      Author: Samuel Tan
 */

#include "graphics/Definitions.h"
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
    //glm::mat4 projMatrix = glm::ortho(
    //    0.0f, static_cast<float>(graphics::WINDOW_WIDTH),
    //    0.0f, static_cast<float>(graphics::WINDOW_HEIGHT),
    //   -1.0f, 1.0f);

	  //glm::mat4 projMatrix = glm::ortho(
		 // -3.0f, 3.0f,
		 // -2.0f, 2.0f,
		 // -2.0f, 2.0f);

    shaderProgram.bind();
  }

  Renderer::~Renderer()
  {
  }

  void Renderer::clearScreen()
  {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }

  void Renderer::draw(GLenum mode, const VertexArray& va,
      const IndexBuffer& ib, const ShaderProgram& sp)
  {
    va.bind();
    ib.bind();
    sp.bind();
    glDrawElements(mode, ib.getNumIndices(), GL_UNSIGNED_INT, nullptr);
  }

  void Renderer::updateCamera(glm::mat4 VPMatrix)
  {
	  shaderProgram.setUniformMat4f("u_VPMatrix", VPMatrix);
  }

  void Renderer::renderAndClearAll()
  {
    clearScreen();
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

} /* namespace render */
