/*
 * ShaderProgram.cpp
 *
 *  Created on: Jul 30, 2018
 *      Author: Samuel Tan
 */

#include "../graphics/ShaderProgram.h"

#include <iostream>
#include <cassert>
#include <sstream>
#include <fstream>

namespace graphics
{

  GLuint ShaderProgram::currentlyBound = 0;

  ShaderProgram::ShaderProgram(const std::string& filepath)
  : programID(glCreateProgram())
  {
//    parseShaders(filepath);

    writeShaders();
    vertexShaderID = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    fragmentShaderID = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    assert(vertexShaderID != 0 && fragmentShaderID != 0);

    glAttachShader(programID, vertexShaderID);
    glAttachShader(programID, fragmentShaderID);
    glLinkProgram(programID);
    glValidateProgram(programID);

    glDeleteShader(vertexShaderID);
    glDeleteShader(fragmentShaderID);
  }

  ShaderProgram::~ShaderProgram()
  {
    if (currentlyBound == programID)
      currentlyBound = 0;
    glDeleteProgram(programID);
  }

  void ShaderProgram::parseShaders(const std::string& filepath)
  {
    std::ifstream stream(filepath);
    std::stringstream streams[2];
    std::string line;

    enum class ShaderType {
        NONE = -1, VERTEX, FRAGMENT
    } type(ShaderType::NONE);

    while (getline(stream, line))
    {
      if (line.find("#shader") != std::string::npos)
      {
        if (line.find("vertex") != std::string::npos)
          type = ShaderType::VERTEX;
        else if (line.find("fragment") != std::string::npos)
          type = ShaderType::FRAGMENT;
      }
      else
      {
        if (type != ShaderType::NONE)
          streams[(int)type] << line << '\n';
      }
    }

    vertexShaderSource = streams[0].str();
    fragmentShaderSource = streams[1].str();

  }

  void ShaderProgram::bind() const
  {
    if (currentlyBound != programID)
      forceBind();
  }

  void ShaderProgram::forceBind() const
  {
    glUseProgram(programID);
    currentlyBound = programID;
  }

  GLuint ShaderProgram::compileShader(GLenum type, std::string& source)
  {
    GLuint shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    // error handling
    int compileSuccess;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileSuccess);
    if (compileSuccess == GL_FALSE)
    {
      int length;
      glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);

      char* msg = new char[length];
      glGetShaderInfoLog(shader, length, &length, msg);
      std::cerr << "Failed to compile " << ((type == GL_VERTEX_SHADER) ?
          "vertex" : "fragment") << " shader!" << std::endl;
      std::cerr << msg << std::endl;

      delete msg;

      glDeleteShader(shader);
      return 0;
    }

    return shader;
  }

  void ShaderProgram::writeShaders()
  {
    /////////////////////////////////

    vertexShaderSource = R"rawstring(
#version 330 core

in vec4 position;
in vec4 color;
out vec4 vo_color;

uniform mat4 u_projMatrix;

void main()
{
    gl_Position = u_projMatrix * position;
    vo_color = color;
}
    )rawstring";

    ///////////////////////////////////

    fragmentShaderSource = R"rawstring(
#version 330 core

in vec4 vo_color;
out vec4 color;

void main()
{
    color = vo_color;
}
    )rawstring";
    /////////////////////////////////
  }

  GLint ShaderProgram::getUniformLocation(const std::string& uniformName)
  {
    auto cachedLoc = uniformCache.find(uniformName);

    GLint location;

    if (cachedLoc == uniformCache.end())
    {
      location = glGetUniformLocation(programID, uniformName.c_str());
      uniformCache[uniformName] = location;
    }
    else
    {
      location = uniformCache[uniformName];
    }

    return location;
  }

  void ShaderProgram::setUniform4f(const std::string& uniformName, float float1,
      float float2, float float3, float float4)
  {
    // remember to bind first!
    glUniform4f(getUniformLocation(uniformName), float1, float2, float3, float4);
  }

  void ShaderProgram::setUniformMat4f(const std::string& uniformName,
      const glm::mat4& matrix)
  {
    glUniformMatrix4fv(getUniformLocation(uniformName), 1, GL_FALSE, &matrix[0][0]);

  }

} /* namespace render */

