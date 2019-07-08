#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "camera.hh"
#include "model.hh"
#include "shader.hh"

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw_gl3.h>
#include <iostream>
#include <stdlib.h>
#include <string>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
void make_background(char *file, Shader background, unsigned int *VBO,
                     unsigned int *VAO, unsigned int *EBO,
                     unsigned int *texture1);

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main(int argc, char *argv[]) {
  int obj = false;
  if (argc != 2)
    std::cout << "Please enter a file" << std::endl;
  else if (string(argv[1]).find(".obj") != std::string::npos)
    obj = true;
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window =
      glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
  if (window == NULL) {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);

  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetScrollCallback(window, scroll_callback);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return -1;
  }

  glEnable(GL_DEPTH_TEST);

  Shader scene("src/vertex.shd", "src/fragment.shd");
  if (!obj) {
    scene.vertex_file = "src/background_vertex.shd";
    scene.fragment_file = "src/background_fragment.shd";
  }
  scene.Init();

  unsigned int VBO, VAO, EBO;
  unsigned int texture1;

  Model obj_model = Model();
  if (!obj)
    make_background(argv[1], scene, &VBO, &VAO, &EBO, &texture1);
  else
    obj_model.loadModel(argv[1]);

  ImGui::CreateContext();
  ImGui_ImplGlfwGL3_Init(window, true);
  ImGui::StyleColorsClassic();

  bool wind = false;
  bool noise_to_whole_image = false;
  bool lightning = false;
  bool uv_shift = false;
  bool horizontal_lines = false;
  bool vertical_lines = false;
  bool small_blocks = false;
  bool rgb_shift = false;
  bool full_noise = false;
  bool vertical_shift = false;

  ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

  while (!glfwWindowShouldClose(window)) {
    ImGui_ImplGlfwGL3_NewFrame();

    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    processInput(window);

    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture1);

    scene.use();
    if (!obj)
      glBindVertexArray(VAO);
    else {
      glUniform3fv(glGetUniformLocation(scene.ID, "viewPos"), 1,
                   &camera.Position[0]);
      glm::vec3 lightPos = glm::vec3(0.0f, -0.75f, 0.0f);
      glUniform3fv(glGetUniformLocation(scene.ID, "lightPos"), 1, &lightPos[0]);
      glm::mat4 projection =
          glm::perspective(glm::radians(camera.Zoom),
                           (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
      scene.setMat4("projection", projection);
      glm::mat4 view = camera.GetViewMatrix();
      scene.setMat4("view", view);

      glm::mat4 model = glm::mat4(1.0f);
      model = glm::translate(model, lightPos);
      model = glm::scale(model, glm::vec3(0.35f, 0.35f, 0.35f));
      scene.setMat4("model", model);
      obj_model.Draw(scene);
    }

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    {
      ImGui::Begin("Effects", &wind);
      ImGui::Text("Activate effects");
      ImGui::Checkbox("lightning", &lightning);
      ImGui::Checkbox("uv shift", &uv_shift);
      ImGui::Checkbox("horizontal lines", &horizontal_lines);
      ImGui::Checkbox("vertical lines", &vertical_lines);
      ImGui::Checkbox("small blocks", &small_blocks);
      ImGui::Checkbox("rgb shift", &rgb_shift);
      ImGui::Checkbox("full noise", &full_noise);
      ImGui::Checkbox("vertical shift", &vertical_shift);
      ImGui::End();
    }

    glUniform1f(glGetUniformLocation(scene.ID, "time"), currentFrame);
    glUniform1f(glGetUniformLocation(scene.ID, "rand"), rand() % 100);
    glUniform1f(glGetUniformLocation(scene.ID, "lightning_check"),
                float(lightning));
    glUniform1f(glGetUniformLocation(scene.ID, "uv_shift_check"),
                float(uv_shift));
    glUniform1f(glGetUniformLocation(scene.ID, "horizontal_lines_check"),
                float(horizontal_lines));
    glUniform1f(glGetUniformLocation(scene.ID, "vertical_lines_check"),
                float(vertical_lines));
    glUniform1f(glGetUniformLocation(scene.ID, "small_blocks_check"),
                float(small_blocks));
    glUniform1f(glGetUniformLocation(scene.ID, "rgb_shift_check"),
                float(rgb_shift));
    glUniform1f(glGetUniformLocation(scene.ID, "full_noise_check"),
                float(full_noise));
    glUniform1f(glGetUniformLocation(scene.ID, "vertical_shift_check"),
                float(vertical_shift));

    ImGui::Render();
    ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  ImGui_ImplGlfwGL3_Shutdown();
  ImGui::DestroyContext();
  glfwTerminate();
  return 0;
}

void make_background(char *file, Shader background, unsigned int *VBO,
                     unsigned *VAO, unsigned *EBO, unsigned *texture1) {
  // poistions + texture coordinates
  float vertices[] = {
      1.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top right
      1.0f,  -1.0f, 0.0f, 1.0f, 0.0f, // bottom right
      -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, // bottom left
      -1.0f, 1.0f,  0.0f, 0.0f, 1.0f  // top left
  };
  unsigned int indices[] = {0, 1, 3, 1, 2, 3};
  glGenVertexArrays(1, &*VAO);
  glGenBuffers(1, &*VBO);
  glGenBuffers(1, &*EBO);

  glBindVertexArray(*VAO);

  glBindBuffer(GL_ARRAY_BUFFER, *VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
               GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                        (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  glGenTextures(1, &*texture1);
  glBindTexture(GL_TEXTURE_2D, *texture1);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  int width, height, nrChannels;
  stbi_set_flip_vertically_on_load(true);

  unsigned char *data = stbi_load(file, &width, &height, &nrChannels, 0);

  if (data) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
  } else {
    std::cout << "Failed to load texture" << std::endl;
  }
  stbi_image_free(data);

  background.use();
  glUniform1i(glGetUniformLocation(background.ID, "texture1"), 0);
}

void processInput(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);

  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    camera.ProcessKeyboard(FORWARD, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    camera.ProcessKeyboard(BACKWARD, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    camera.ProcessKeyboard(LEFT, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    camera.ProcessKeyboard(RIGHT, deltaTime);
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
  if (firstMouse) {
    lastX = xpos;
    lastY = ypos;
    firstMouse = false;
  }

  float xoffset = xpos - lastX;
  float yoffset = lastY - ypos;

  lastX = xpos;
  lastY = ypos;

  camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
  camera.ProcessMouseScroll(yoffset);
}
