#define STB_IMAGE_IMPLEMENTATION
#include "../include/stb_image.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <iostream>
#include <vector>
#include <string>
#include <map>

#include <stdio.h>
#include <stdlib.h>

using namespace std;

GLuint program;

#include "types.cpp"

char* load_file(char const* path)
{
    char* buffer = 0;
    long length;
    FILE * f = fopen (path, "rb"); //was "rb"

    if (f)
    { 
      fseek (f, 0, SEEK_END);
      length = ftell (f);
      fseek (f, 0, SEEK_SET);
      buffer = (char*)malloc ((length+1)*sizeof(char));
      if (buffer)
      {
        fread (buffer, sizeof(char), length, f);
      }
      fclose (f);
    }
    buffer[length] = '\0';
    // for (int i = 0; i < length; i++) {
    //     printf("buffer[%d] == %c\n", i, buffer[i]);
    // }
    //printf("buffer = %s\n", buffer);

    return buffer;
}

Camera *createCamera(glm::vec3 const & Translate, glm::vec2 const & Rotate, float speed, float sensibility)
{
    Camera *camera = new Camera();

    camera->speed = speed;
    camera->sensibility = sensibility;

    glm::mat4 Projection = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.f);
    glm::mat4 View;
    View = glm::rotate(View, Rotate.y, glm::vec3(-1.0f, 0.0f, 0.0f));
    View = glm::rotate(View, Rotate.x, glm::vec3(0.0f, 1.0f, 0.0f));
    View = glm::translate(View, -Translate);

    camera->transform = Projection * View;
    camera->angle = Rotate;
    camera->translate = Translate;

    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    camera->cameraRight = glm::normalize(glm::cross(up, camera->translate));
    camera->cameraUp = glm::cross(camera->translate, camera->cameraRight);
    return camera;
}

void loadShaders(){
    char *vertexShaderCode = load_file("src/shaders/vertex.glsl");
    char *fragShaderCode = load_file("src/shaders/frag.glsl");

    if(vertexShaderCode == NULL || fragShaderCode == NULL){
        if(vertexShaderCode != NULL)
            free(vertexShaderCode);
        if(fragShaderCode != NULL)
            free(fragShaderCode);
        return;
    }

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    if(program) glDeleteProgram(program);
    program = glCreateProgram();

    glShaderSource(vertexShader, 1, &vertexShaderCode, NULL);
    glShaderSource(fragShader, 1, &fragShaderCode, NULL);

    glCompileShader(vertexShader);
    char buffer[255] = {};
    GLsizei length;
    glGetShaderInfoLog(vertexShader, 254, &length, buffer);
    if(strlen(buffer) > 0)
        printf("%s\n", buffer);

    glCompileShader(fragShader);
    glGetShaderInfoLog(fragShader, 254, &length, buffer);
    if(strlen(buffer) > 0)
        printf("%s\n", buffer);

    glAttachShader(program, vertexShader);
    glAttachShader(program, fragShader);

    glLinkProgram(program);
    glGetProgramInfoLog(program, 254, &length, buffer);
    if(strlen(buffer) > 0)
        printf("%s\n", buffer);

    glDeleteShader(vertexShader);
    glDeleteShader(fragShader);
    free(vertexShaderCode);
    free(fragShaderCode);
}

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(1920, 1080, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);
    //glfwSwapInterval(0);

    int e = glewInit();
    printf("%s\n", glewGetErrorString(e));

    loadShaders();

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    
    Camera *camera = createCamera(glm::vec3(0,0,10), glm::vec2(0,0), 5, 0.1);
    glm::vec3 lightPos = glm::vec3(10.0, 5.0, 0.0);

    vector<Fish*> fishes;

    Model *fishModel = new Model("models/fish2.obj", "models/fish_texture.png");
    for(int i = 0; i < 1000; i++){
        Fish *fish = new Fish();

        fish->model = fishModel;
        fish->position = glm::vec3(rand() % 20 +10, rand() % 10 -5, 0);
        //vec2.x = around x axis, vec2.y = around y axis
        fish->angle = glm::vec2(0,0);
        fish->targetAngle = fish->angle;
        fish->scale = 0.3;
        fish->speed = (rand() % 800 +200)/300.0;
        fish->timeSinceAction = 0;

        fishes.push_back(fish);
    }

    GLuint vbo;
    GLuint ebo;
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    float deltaTime = 0.0f;	// Time between current frame and last frame
    float lastFrame = 0.0f; // Time of last frame
    float timeElapsed = 0.0f;
    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        timeElapsed += deltaTime;

        if(timeElapsed > 0.3){
            timeElapsed -= 0.3;

            loadShaders();
        }
        //printf("FPS: %f\n", 1/deltaTime);

        double newxpos, newypos;
        glfwGetCursorPos(window, &newxpos, &newypos);
        double difx = newxpos - xpos;
        xpos = newxpos;
        double dify = newypos - ypos;
        ypos = newypos;

        if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera->translate += (-camera->speed * deltaTime) * glm::normalize(camera->translate);
        if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera->translate += (camera->speed * deltaTime) * glm::normalize(camera->translate);
        if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera->translate += (-camera->speed * deltaTime) * glm::normalize(camera->cameraRight);
        if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera->translate += (camera->speed * deltaTime) * glm::normalize(camera->cameraRight);

        Camera *ncamera = createCamera(camera->translate, camera->angle + glm::vec2(camera->sensibility*deltaTime*difx, -camera->sensibility*deltaTime*dify), 5, 0.1);
        delete camera;
        camera = ncamera;

        if(glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
            lightPos += glm::vec3(0.1, 0.0, 0.0);

        glEnable(GL_DEPTH_TEST);
        /* Render here */
        glClearColor(0.1, 0.1, 0.1, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(program);

        GLuint uniformLoc = glGetUniformLocation(program, "view");
        glUniformMatrix4fv(uniformLoc, 1, GL_FALSE, glm::value_ptr(camera->transform));
        
        uniformLoc = glGetUniformLocation(program, "lightPos");
        glUniform3fv(uniformLoc, 1, glm::value_ptr(lightPos));

        for(int j = 0; j < fishes.size(); j++){
            Fish *fish = fishes[j];

            fish->timeSinceAction += deltaTime;
            if(fish->timeSinceAction > 3){
                fish->timeSinceAction -= 3;
                fish->targetAngle = glm::vec2(M_PI * ((float)rand())/RAND_MAX -M_PI/2, 2*M_PI * ((float)rand())/RAND_MAX);
            }

            float turningSpeed = 0.1 * deltaTime;
            if(fish->targetAngle.x != fish->angle.x && fish->targetAngle.y != fish->angle.y){
                if(fish->targetAngle.x > fish->angle.x)
                    fish->angle.x += turningSpeed;
                if(fish->targetAngle.x < fish->angle.x)
                    fish->angle.x -= turningSpeed;
                if(fish->targetAngle.y > fish->angle.y)
                    fish->angle.y += turningSpeed;
                if(fish->targetAngle.y < fish->angle.y)
                    fish->angle.y -= turningSpeed;
            }

            glm::mat4 model;
            model = glm::translate(model, fish->position);
            glm::mat4 aux = glm::toMat4(glm::angleAxis(fish->angle.y, glm::vec3(0,1,0)));
            model = model * aux;
            aux = glm::toMat4(glm::angleAxis(fish->angle.x, glm::vec3(1,0,0)));
            model = model * aux;
            model = glm::scale(model, glm::vec3(fish->scale, fish->scale, fish->scale));

            uniformLoc = glGetUniformLocation(program, "model");
            glUniformMatrix4fv(uniformLoc, 1, GL_FALSE, glm::value_ptr(model));

            fish->position += (deltaTime * fish->speed) * glm::vec3(sin(fish->angle.y)*cos(fish->angle.x), -sin(fish->angle.x), cos(fish->angle.y)*cos(fish->angle.x));

            for(int i = 0; i < fish->model->meshes.size(); i++){
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, fish->model->meshes[i].textures[0]);

                glBindVertexArray(fish->model->meshes[i].VAO);
                glDrawElements(GL_TRIANGLES, fish->model->meshes[i].indices.size(), GL_UNSIGNED_INT, (void*) 0);
                glBindVertexArray(0);
            }
        }

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
