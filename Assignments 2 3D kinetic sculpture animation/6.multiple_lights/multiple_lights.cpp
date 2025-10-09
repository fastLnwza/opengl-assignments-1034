#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>

#include <iostream>
#include <vector>
#include <cmath>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
unsigned int loadTexture(const char* path);

// settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// camera
Camera camera(glm::vec3(0.0f, 1.0f, 6.5f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// toggles / params
bool animateSculpture = true;
bool animateLights = true;
bool showBackgroundCubes = false; // can toggle to see original cubes

float sculptureSpeed = 0.6f;   // global speed multiplier
float lightOrbitSpeed = 0.7f;  // light movement speed

// lighting
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

// forward declarations for drawing
void drawCube(Shader& shader, unsigned int cubeVAO, const glm::mat4& model);
void drawKineticSculpture(Shader& shader, unsigned int cubeVAO, float t);

int main()
{
    // glfw: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // window
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Kinetic Sculpture — Multiple Lights", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // capture mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // global opengl state
    glEnable(GL_DEPTH_TEST);

    // shaders (use LearnOpenGL provided shader class + files)
    Shader lightingShader("6.multiple_lights.vs", "6.multiple_lights.fs");
    Shader lightCubeShader("6.light_cube.vs", "6.light_cube.fs");

    // cube data
    float vertices[] = {
        // positions          // normals           // texcoords
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.0f + 0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
    };

    // VAOs/VBO
    unsigned int VBO, cubeVAO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindVertexArray(cubeVAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // light cube VAO
    unsigned int lightCubeVAO;
    glGenVertexArrays(1, &lightCubeVAO);
    glBindVertexArray(lightCubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // textures
    stbi_set_flip_vertically_on_load(true);
    unsigned int diffuseMap = loadTexture(FileSystem::getPath("resources/textures/container2.png").c_str());
    unsigned int specularMap = loadTexture(FileSystem::getPath("resources/textures/container2_specular.png").c_str());

    // shader configuration
    lightingShader.use();
    lightingShader.setInt("material.diffuse", 0);
    lightingShader.setInt("material.specular", 1);

    // point lights (base positions, will be animated on orbits)
    glm::vec3 pointLightBase[4] = {
        glm::vec3(0.0f,  1.6f,  0.0f),
        glm::vec3(2.5f,  0.8f, -1.5f),
        glm::vec3(-2.5f,  1.2f, -2.0f),
        glm::vec3(0.0f,  0.5f, -3.0f)
    };

    // background cubes (optional)
    std::vector<glm::vec3> cubePositions = {
        glm::vec3(2.0f, -0.4f, -3.5f),
        glm::vec3(-1.7f,  0.3f, -4.5f),
        glm::vec3(1.3f, -0.8f, -2.5f),
        glm::vec3(1.5f,  0.2f, -1.5f),
        glm::vec3(-1.3f,  0.4f, -1.5f)
    };

    // render loop
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glClearColor(0.06f, 0.06f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // shaders & common uniforms
        lightingShader.use();
        lightingShader.setVec3("viewPos", camera.Position);
        lightingShader.setFloat("material.shininess", 32.0f);

        // directional light — soft top light
        lightingShader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.15f);
        lightingShader.setVec3("dirLight.ambient", 0.03f, 0.03f, 0.04f);
        lightingShader.setVec3("dirLight.diffuse", 0.25f, 0.25f, 0.30f);
        lightingShader.setVec3("dirLight.specular", 0.4f, 0.4f, 0.45f);

        // animate point lights on circular/epicyclic orbits to emphasize 3D
        glm::vec3 pointLightPositions[4];
        for (int i = 0; i < 4; ++i) pointLightPositions[i] = pointLightBase[i];
        if (animateLights) {
            float t = currentFrame * lightOrbitSpeed;
            pointLightPositions[0] += glm::vec3(1.2f * cos(t), 0.3f * sin(2 * t), 1.2f * sin(t));
            pointLightPositions[1] += glm::vec3(0.9f * cos(-1.2f * t), 0.25f * cos(1.8f * t), 0.9f * sin(-1.2f * t));
            pointLightPositions[2] += glm::vec3(1.6f * cos(0.7f * t), 0.35f * sin(1.3f * t), 1.6f * sin(0.7f * t));
            pointLightPositions[3] += glm::vec3(0.7f * cos(2.1f * t), 0.20f * sin(2.3f * t), 0.7f * sin(2.1f * t));
        }

        // send point light uniforms
        for (int i = 0; i < 4; ++i) {
            std::string base = "pointLights[" + std::to_string(i) + "]";
            lightingShader.setVec3((base + ".position").c_str(), pointLightPositions[i]);
            lightingShader.setVec3((base + ".ambient").c_str(), 0.05f, 0.05f, 0.05f);
            lightingShader.setVec3((base + ".diffuse").c_str(), 0.8f, 0.8f, 0.8f);
            lightingShader.setVec3((base + ".specular").c_str(), 1.0f, 1.0f, 1.0f);
            lightingShader.setFloat((base + ".constant").c_str(), 1.0f);
            lightingShader.setFloat((base + ".linear").c_str(), 0.09f);
            lightingShader.setFloat((base + ".quadratic").c_str(), 0.032f);
        }

        // camera-attached spotlight (flashlight)
        lightingShader.setVec3("spotLight.position", camera.Position);
        lightingShader.setVec3("spotLight.direction", camera.Front);
        lightingShader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
        lightingShader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
        lightingShader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
        lightingShader.setFloat("spotLight.constant", 1.0f);
        lightingShader.setFloat("spotLight.linear", 0.09f);
        lightingShader.setFloat("spotLight.quadratic", 0.032f);
        lightingShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
        lightingShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));

        // view/projection
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        lightingShader.setMat4("projection", projection);
        lightingShader.setMat4("view", view);

        // textures
        glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, diffuseMap);
        glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, specularMap);

        // floor/base (scaled cube)
        glm::mat4 model(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -0.55f, 0.0f));
        model = glm::scale(model, glm::vec3(8.0f, 0.1f, 8.0f));
        lightingShader.setMat4("model", model);
        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // kinetic sculpture — hierarchical transforms
        drawKineticSculpture(lightingShader, cubeVAO, animateSculpture ? currentFrame * sculptureSpeed : 0.0f);

        // optional: background reference cubes
        if (showBackgroundCubes) {
            for (size_t i = 0; i < cubePositions.size(); ++i) {
                glm::mat4 m(1.0f);
                m = glm::translate(m, cubePositions[i]);
                m = glm::rotate(m, glm::radians(20.0f * (float)i), glm::vec3(1.0f, 0.3f, 0.5f));
                lightingShader.setMat4("model", m);
                glDrawArrays(GL_TRIANGLES, 0, 36);
            }
        }

        // draw light bulbs
        lightCubeShader.use();
        lightCubeShader.setMat4("projection", projection);
        lightCubeShader.setMat4("view", view);
        glBindVertexArray(lightCubeVAO);
        for (int i = 0; i < 4; ++i) {
            glm::mat4 m(1.0f);
            m = glm::translate(m, pointLightPositions[i]);
            m = glm::scale(m, glm::vec3(0.12f));
            lightCubeShader.setMat4("model", m);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteVertexArrays(1, &lightCubeVAO);
    glDeleteBuffers(1, &VBO);

    glfwTerminate();
    return 0;
}

// --- DRAW HELPERS -----------------------------------------------------------

void drawCube(Shader& shader, unsigned int cubeVAO, const glm::mat4& model) {
    shader.setMat4("model", model);
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

// A simple mobile-like kinetic sculpture with 3 arms and hanging cubes.
// Uses only hierarchical 3D transforms of the same cube geometry.
void drawKineticSculpture(Shader& shader, unsigned int cubeVAO, float t) {
    // base pole
    glm::mat4 model(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 0.15f, 0.0f));
    model = glm::scale(model, glm::vec3(0.1f, 1.2f, 0.1f));
    drawCube(shader, cubeVAO, model);

    // hub (center joint)
    glm::mat4 hub(1.0f);
    hub = glm::translate(hub, glm::vec3(0.0f, 0.75f, 0.0f));
    hub = glm::rotate(hub, 0.6f * sin(0.5f * t), glm::vec3(1.0f, 0.0f, 0.0f));
    hub = glm::rotate(hub, 0.6f * cos(0.45f * t), glm::vec3(0.0f, 0.0f, 1.0f));

    // arm 1 (X axis)
    glm::mat4 arm1 = hub;
    arm1 = glm::rotate(arm1, t, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 bar1 = arm1;
    bar1 = glm::scale(bar1, glm::vec3(2.8f, 0.06f, 0.06f));
    drawCube(shader, cubeVAO, bar1);

    glm::mat4 end1 = arm1;
    end1 = glm::translate(end1, glm::vec3(1.4f, -0.45f + 0.15f * sin(1.4f * t), 0.0f));
    end1 = glm::scale(end1, glm::vec3(0.25f));
    drawCube(shader, cubeVAO, end1);

    glm::mat4 end1b = arm1;
    end1b = glm::translate(end1b, glm::vec3(-1.4f, -0.35f + 0.12f * cos(1.1f * t + 1.2f), 0.0f));
    end1b = glm::scale(end1b, glm::vec3(0.20f));
    drawCube(shader, cubeVAO, end1b);

    // arm 2 (Z axis)
    glm::mat4 arm2 = hub;
    arm2 = glm::rotate(arm2, -0.5f * t, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 bar2 = arm2;
    bar2 = glm::rotate(bar2, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    bar2 = glm::scale(bar2, glm::vec3(2.2f, 0.06f, 0.06f));
    drawCube(shader, cubeVAO, bar2);

    glm::mat4 end2 = arm2;
    end2 = glm::translate(end2, glm::vec3(0.0f, -0.40f + 0.10f * sin(1.8f * t + 2.0f), 1.1f));
    end2 = glm::rotate(end2, 1.5f * t, glm::vec3(0.3f, 0.7f, 0.2f));
    end2 = glm::scale(end2, glm::vec3(0.22f));
    drawCube(shader, cubeVAO, end2);

    glm::mat4 end2b = arm2;
    end2b = glm::translate(end2b, glm::vec3(0.0f, -0.36f + 0.13f * cos(1.3f * t + 1.0f), -1.1f));
    end2b = glm::rotate(end2b, -1.2f * t, glm::vec3(0.5f, 0.1f, 0.8f));
    end2b = glm::scale(end2b, glm::vec3(0.18f));
    drawCube(shader, cubeVAO, end2b);

    // arm 3 (diagonal)
    glm::mat4 arm3 = hub;
    arm3 = glm::rotate(arm3, 0.8f * sin(0.7f * t), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 bar3 = arm3;
    bar3 = glm::rotate(bar3, glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    bar3 = glm::scale(bar3, glm::vec3(1.8f, 0.055f, 0.055f));
    drawCube(shader, cubeVAO, bar3);

    glm::mat4 end3 = arm3;
    end3 = glm::translate(end3, glm::vec3(1.0f, -0.30f + 0.10f * sin(2.2f * t + 0.4f), 1.0f));
    end3 = glm::scale(end3, glm::vec3(0.16f));
    drawCube(shader, cubeVAO, end3);

    // hub cube (visual center)
    glm::mat4 hubCube = hub;
    hubCube = glm::scale(hubCube, glm::vec3(0.14f));
    drawCube(shader, cubeVAO, hubCube);
}

// input
void processInput(GLFWwindow* window)
{
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

    // toggles
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) animateSculpture = true;
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_RELEASE && glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) animateSculpture = !animateSculpture;

    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) animateLights = true;
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_RELEASE && glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) animateLights = !animateLights;

    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
        camera = Camera(glm::vec3(0.0f, 1.0f, 6.5f));
    }

    if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS) sculptureSpeed = glm::max(0.1f, sculptureSpeed - 0.5f * deltaTime);
    if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS) sculptureSpeed = glm::min(3.0f, sculptureSpeed + 0.5f * deltaTime);
}

// window resize
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// mouse move
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// scroll
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

// texture loader
unsigned int loadTexture(char const* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format = GL_RGB;
        if (nrComponents == 1) format = GL_RED;
        else if (nrComponents == 3) format = GL_RGB;
        else if (nrComponents == 4) format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}
