/*
 * ShaderProgram.h
 *
 *  Created on: Jul 30, 2018
 *      Author: Samuel Tan
 */

#ifndef SHADERPROGRAM_H_
#define SHADERPROGRAM_H_

#include <string>
#include <GL/glew.h>
#include <unordered_map>

#include "glm/glm.hpp"

namespace graphics
{

  class ShaderProgram
  {
    private:
      static GLuint currentlyBound;
      GLuint programID, vertexShaderID, fragmentShaderID;
      std::string vertexShaderSource, fragmentShaderSource;
      std::unordered_map<std::string, GLint> uniformCache;

    public:

      ShaderProgram(const std::string& filepath);
      virtual ~ShaderProgram();

      void bind() const;
      void forceBind() const;
      void setUniform4f(const std::string& uniformName, float, float, float, float);
      void setUniformMat4f(const std::string& uniformName, const glm::mat4& matrix);

      GLuint getID() const {return programID;}

    private:
      void parseShaders(const std::string& filepath);
      void writeShaders();
      static GLuint compileShader(GLenum type, std::string& source);
      GLint getUniformLocation(const std::string& uniformName);
  };

} /* namespace render */

#endif /* SHADERPROGRAM_H_ */
