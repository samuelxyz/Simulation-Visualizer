/*
 * OpenGLTest.cpp
 *
 *  Created on: Jul 26, 2018
 *      Author: Samuel Tan
 *
 */

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "../graphics/Window.h"

void renderBasic()
{
  glBegin(GL_TRIANGLES);

  glVertex2f(-0.7f, -0.2f);
  glVertex2f(-0.7f, -0.7f);
  glVertex2f(-0.2f, -0.7f);

  glVertex2f(0.7f, 0.2f);
  glVertex2f(0.7f, 0.7f);
  glVertex2f(0.2f, 0.7f);

  glEnd();
}

void renderArray()
{
  glDrawArrays(GL_TRIANGLES, 0, 6);
}

void renderElements(int indices)
{
  glDrawElements(GL_TRIANGLES, indices, GL_UNSIGNED_INT, nullptr);
}

void changeColor(float& color, float& increment)
{
  color += increment;
  if (color >= 1.0f)
  {
    color = 1.0f;
    increment *= -1;
  }
  else if (color <= 0.0f)
  {
    color = 0.0f;
    increment *= -1;
  }
}

int vertexColorShaderTest()
{
  graphics::Window window;

  float vertices[] = {

//   |  x      y   |   r     g     b     a  |
      -0.7f, -0.2f,   1.0f, 0.0f, 0.0f, 1.0f, // lower left
      -0.7f, -0.7f,   1.0f, 0.0f, 0.0f, 1.0f,
      -0.2f, -0.7f,   1.0f, 0.0f, 0.0f, 1.0f,

      -0.7f,  0.2f,   0.0f, 1.0f, 0.0f, 1.0f, // upper left
      -0.7f,  0.7f,   0.0f, 1.0f, 0.0f, 1.0f,
      -0.2f,  0.7f,   0.0f, 1.0f, 0.0f, 1.0f,

       0.7f,  0.2f,   0.0f, 0.0f, 1.0f, 1.0f, // upper right
       0.7f,  0.7f,   0.0f, 0.0f, 1.0f, 1.0f,
       0.2f,  0.7f,   0.0f, 0.0f, 1.0f, 1.0f,

       0.7f, -0.2f,   1.0f, 1.0f, 0.0f, 1.0f, // lower right
       0.7f, -0.7f,   1.0f, 1.0f, 0.0f, 1.0f,
       0.2f, -0.7f,   1.0f, 1.0f, 0.0f, 1.0f,

      -0.2f, -0.2f,   1.0f, 0.0f, 0.0f, 1.0f, // center square
      -0.2f,  0.2f,   0.0f, 1.0f, 0.0f, 1.0f,
       0.2f,  0.2f,   0.0f, 0.0f, 1.0f, 1.0f,
       0.2f, -0.2f,   1.0f, 1.0f, 0.0f, 1.0f
  };

  GLuint indices[] = {

      0, 1, 2,

      3, 4, 5,

      6, 7, 8,

      9, 10, 11,

      12, 13, 14,
      12, 14, 15
  };

  graphics::ShaderProgram shaderProgram("resources/shaders/VertexColorShader.glsl");

  graphics::VertexArray vertexArray;
  vertexArray.addAttribute("position", GL_FLOAT, 2);
  vertexArray.addAttribute("color", GL_FLOAT, 4);

  graphics::VertexBuffer vertexBuffer((const void*)vertices, sizeof(vertices), GL_STATIC_DRAW);
  vertexArray.applyAttributesWithBuffer(vertexBuffer, shaderProgram);

  graphics::IndexBuffer indexBuffer(indices, sizeof(indices)/sizeof(GLuint), GL_STATIC_DRAW);

  /* Loop until the user closes the window */
  while (!window.shouldClose())
  {
    /* Render here */
    graphics::Renderer::clearScreen();
    graphics::Renderer::draw(GL_TRIANGLES, vertexArray, indexBuffer, shaderProgram);

    /* Swap front and back buffers */
    window.swapBuffers();

    /* Poll for and process events */
    glfwPollEvents();
  }

  return 0;
}