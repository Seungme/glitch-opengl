#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

std::string load_shader(const std::string &filename) {
  std::ifstream ifs(filename, std::ios::in);
  std::string content;

  if (ifs.is_open()) {
    std::stringstream ss;
    ss << ifs.rdbuf();
    content = ss.str();
    ifs.close();
  } else {
    std::cerr << "FAIL to open file" << std::endl;
    return 0;
  }
  return content;
}

class Shader {
public:
  unsigned int ID;
  const char *vertex_file;
  const char *fragment_file;
  Shader(const char *vertex, const char *fragment)
      : vertex_file(vertex), fragment_file(fragment) {}
  void Init() {

    auto src = load_shader(vertex_file);
    const char *vertex_src = src.c_str();

    auto f_src = load_shader(fragment_file);
    const char *fragment_src = f_src.c_str();

    // vertex shader
    int vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_src, NULL);
    glCompileShader(vertex_shader);
    test_error(vertex_shader, "vertex");

    // fragment shader
    int fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_src, NULL);
    glCompileShader(fragment_shader);
    test_error(fragment_shader, "fragment");

    // link shaders
    ID = glCreateProgram();
    glAttachShader(ID, vertex_shader);
    glAttachShader(ID, fragment_shader);
    glLinkProgram(ID);
    test_error(ID, "program");

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
  }
  void test_error(unsigned int shader, const std::string &type) {
    // check for error
    int success;
    char log[512];
    if (!type.compare("program")) {
      glGetProgramiv(shader, GL_LINK_STATUS, &success);
      if (!success) {
        glGetProgramInfoLog(ID, 512, NULL, log);
        std::cerr << "Linking error: " << type << log << std::endl;
      }
    } else {
      glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
      if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, log);
        std::cerr << "Shader error: " << type << log << std::endl;
      }
    }
  }

  void use() { glUseProgram(ID); }
  void setMat4(const std::string &name, const glm::mat4 &mat) const {
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE,
                       &mat[0][0]);
  }
};
