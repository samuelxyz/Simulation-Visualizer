/*
 * Renderer.cpp
 *
 *  Created on: Jul 30, 2018
 *      Author: Samuel Tan
 */

#include "core/stdafx.h"
#include "core/Definitions.h"
#include "graphics/Renderer.h"
#include "graphics/IndexBuffer.h"
#include "graphics/VertexArray.h"
#include "graphics/content/VisualSphere.h"

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

  void Renderer::setDepthWritingEnabled(bool enable)
  {
	  static bool cache = true;

	  if (enable != cache)
	  {
		  cache = enable;
		  glDepthMask(enable ? GL_TRUE : GL_FALSE);
	  }
  }

  // Use zero or omit color for multicolor mode
  void Renderer::renderMarkerDot(const glm::vec3& pos, 
	  const glm::vec4& color)
  {
	  graphics::VisualSphere
	  {
		  pos, core::QUAT_IDENTITY, MARKER_DOT_RADIUS,
		  glm::length2(color) == 0.0f ?
		  VisualEntity::Style::MULTICOLOR : VisualEntity::Style::SOLID_COLOR,
		  color
	  }.render(*this);
  }

  void Renderer::setDepthTestEnabled(bool enable)
  {
	  static bool cache = true;

	  if (enable != cache)
	  {
		  cache = enable;
		  if (enable)
			  glEnable(GL_DEPTH_TEST);
		  else
			  glDisable(GL_DEPTH_TEST);
	  }
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

  void Renderer::renderAndClearAll(bool enableDepthTest, bool enableDepthWriting)
  {
	setDepthTestEnabled(enableDepthTest);
	if (enableDepthTest)
		setDepthWritingEnabled(enableDepthWriting);

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
