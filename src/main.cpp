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
GLuint uiProgram;

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

void loadShaders(){
    char *vertexShaderCode = load_file("src/shaders/vertex.glsl");
    char *fragShaderCode = load_file("src/shaders/frag.glsl");
    char *uiFragShaderCode = load_file("src/shaders/uifrag.glsl");
    char *uiVertexShaderCode = load_file("src/shaders/uivertex.glsl");

    if(vertexShaderCode == NULL || fragShaderCode == NULL || uiVertexShaderCode == NULL || uiFragShaderCode == NULL){
        if(uiFragShaderCode != NULL)
            free(uiFragShaderCode);
        if(uiVertexShaderCode != NULL)
            free(uiVertexShaderCode);
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

    GLuint uiVertexShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint uiFragShader = glCreateShader(GL_FRAGMENT_SHADER);
    if(uiProgram) glDeleteProgram(uiProgram);
    uiProgram = glCreateProgram();

    glShaderSource(uiVertexShader, 1, &uiVertexShaderCode, NULL);
    glShaderSource(uiFragShader, 1, &uiFragShaderCode, NULL);

    glCompileShader(uiVertexShader);
    {
        char buffer[255] = {};
        GLsizei length;
        glGetShaderInfoLog(uiVertexShader, 254, &length, buffer);
        if(strlen(buffer) > 0)
            printf("%s\n", buffer);

        glCompileShader(uiFragShader);
        glGetShaderInfoLog(uiFragShader, 254, &length, buffer);
        if(strlen(buffer) > 0)
            printf("%s\n", buffer);

        glAttachShader(uiProgram, uiVertexShader);
        glAttachShader(uiProgram, uiFragShader);

        glLinkProgram(uiProgram);
        glGetProgramInfoLog(uiProgram, 254, &length, buffer);
        if(strlen(buffer) > 0)
            printf("%s\n", buffer);

        glDeleteShader(uiVertexShader);
        glDeleteShader(uiFragShader);
        free(uiVertexShaderCode);
        free(uiFragShaderCode);
    }
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
    
    glm::mat4 Projection = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.f);

    Camera camera;
    camera.position = glm::vec3(0,0,-35);
    camera.up = glm::vec3(0,1,0);
    camera.front = glm::vec3(0,0,-1);
    camera.angle = glm::vec2(1.6, 0);
    camera.transform = Projection * glm::lookAt(camera.position, camera.position + camera.front, camera.up);
    camera.speed = 3;
    camera.sensibility = 0.01;

    glm::vec3 lightPos = glm::vec3(0.0, 0.0, 0.0);

    vector<Fish*> fishes;

    Model *aquarioModel = new Model("models/aquario.obj", "models/fish_texture.png");
    Box aquario;
    aquario.position = glm::vec3(0,0,0);
    aquario.size = glm::vec3(5,5,5);

    Model *fishModel = new Model("models/fish2.obj", "models/fish_texture.png");
    for(int i = 0; i < 5; i++){
        Fish *fish = new Fish();

        fish->model = fishModel;
        //fish->position = glm::vec3(rand() % 6 -3, rand() % 6 -3, rand() % 6 -3);
        fish->position = glm::vec3(rand() % 4 -2, rand() % 4 -2, rand() % 4 -2);
        //vec2.x = around x axis, vec2.y = around y axis
        fish->angle = glm::vec2(0,0);
        fish->targetAngle = fish->angle;
        fish->targetPos = glm::vec3(((float)rand()/RAND_MAX * aquario.size.x - aquario.size.x/2) * 0.7,
                                    ((float)rand()/RAND_MAX * aquario.size.y - aquario.size.y/2) * 0.7,
                                    ((float)rand()/RAND_MAX * aquario.size.z - aquario.size.z/2) * 0.7);
        fish->rotationType = rand() % 2;
        fish->scale = 0.2 + ((float)rand())/RAND_MAX * 0.3;
        fish->speed = (rand() % 800 +200)/300.0;

        fishes.push_back(fish);
    }

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
        double dirx = (newxpos - xpos) * camera.sensibility;
        double diry = (newypos - ypos) * camera.sensibility;
        xpos = newxpos;
        ypos = newypos;

        camera.angle.x += dirx;
        camera.angle.y -= diry;

        if(camera.angle.y > 1.5)
            camera.angle.y = 1.5;
        if(camera.angle.y < -1.5)
            camera.angle.y = -1.5;

        if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera.position += (camera.speed * deltaTime) * camera.front;
        if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera.position -= (camera.speed * deltaTime) * camera.front;
        if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera.position -= glm::normalize(glm::cross(camera.front, camera.up)) * (camera.speed * deltaTime);
        if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera.position += glm::normalize(glm::cross(camera.front, camera.up)) * (camera.speed * deltaTime);

        glm::vec3 front;
        front.x = cos(camera.angle.x) * cos(camera.angle.y);
        front.y = sin(camera.angle.y);
        front.z = sin(camera.angle.x) * cos(camera.angle.y);
        camera.front = glm::normalize(front);

        camera.up = glm::normalize(glm::cross(glm::cross(camera.front, glm::vec3(0,1,0)), camera.front));

        camera.transform = Projection * glm::lookAt(camera.position, camera.position + camera.front, camera.up);

        if(glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
            lightPos += glm::vec3(0.1, 0.0, 0.0);

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glEnable(GL_CULL_FACE);

        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        /* Render here */
        glClearColor(0.1, 0.1, 0.1, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(program);

        GLuint uniformLoc = glGetUniformLocation(program, "view");
        glUniformMatrix4fv(uniformLoc, 1, GL_FALSE, glm::value_ptr(camera.transform));
        
        uniformLoc = glGetUniformLocation(program, "lightPos");
        glUniform3fv(uniformLoc, 1, glm::value_ptr(lightPos));

        uniformLoc = glGetUniformLocation(program, "alpha");
        glUniform1f(uniformLoc, 1);
        for(int j = 0; j < fishes.size(); j++){
            Fish *fish = fishes[j];

            glm::vec3 targetDir = glm::normalize(fish->targetPos - fish->position);
            if(targetDir.z == 0) targetDir.z = 0.000000001;
            fish->targetAngle = glm::vec2(acos(targetDir.y) - M_PI/2, atan2(targetDir.x, targetDir.z));

            float turningSpeed = 1 * deltaTime;
            if(abs(fish->targetAngle.x - fish->angle.x) > 0.2 || abs(fish->targetAngle.y - fish->angle.y) > 0.2){
                if(M_PI < fish->targetAngle.y - fish->angle.y && fish->targetAngle.y - fish->angle.y < M_PI){
                    if(fish->rotationType)
                        fish->angle.y += turningSpeed;
                    else
                        fish->angle.y -= turningSpeed;
                }
                else{
                    if(fish->rotationType)
                        fish->angle.y -= turningSpeed;
                    else
                        fish->angle.y += turningSpeed;
                }
                if(fish->targetAngle.x > fish->angle.x)
                    fish->angle.x += turningSpeed;
                if(fish->targetAngle.x < fish->angle.x)
                    fish->angle.x -= turningSpeed;
            }
            if(fish->angle.y > M_PI)
                fish->angle.y -= 2*M_PI;
            if(fish->angle.y < -M_PI)
                fish->angle.y += 2*M_PI;

            glm::mat4 model;
            model = glm::translate(model, fish->position);
            glm::mat4 aux = glm::toMat4(glm::angleAxis(fish->angle.y, glm::vec3(0,1,0)));
            model = model * aux;
            aux = glm::toMat4(glm::angleAxis(fish->angle.x, glm::vec3(1,0,0)));
            model = model * aux;
            model = glm::scale(model, glm::vec3(fish->scale, fish->scale, fish->scale));

            uniformLoc = glGetUniformLocation(program, "model");
            glUniformMatrix4fv(uniformLoc, 1, GL_FALSE, glm::value_ptr(model));
            
            glm::vec3 dist = fish->position - fish->targetPos;
            if(!(abs(dist.x) < 0.1 && abs(dist.y) < 0.1 && abs(dist.z) < 0.1))
            {
                glm::vec3 distToWalk = (deltaTime * fish->speed) * glm::vec3(sin(fish->angle.y)*cos(fish->angle.x), -sin(fish->angle.x), cos(fish->angle.y)*cos(fish->angle.x));
                if (aquario.contains(fish->position + distToWalk))
                {
                    fish->position += distToWalk;
                }
            }
            else{
                fish->targetPos = glm::vec3(((float)rand()/RAND_MAX * aquario.size.x - aquario.size.x/2) * 0.7,
                                            ((float)rand()/RAND_MAX * aquario.size.y - aquario.size.y/2) * 0.7,
                                            ((float)rand()/RAND_MAX * aquario.size.z - aquario.size.z/2) * 0.7);
                fish->rotationType = rand() % 2;
            }

            for(int i = 0; i < fish->model->meshes.size(); i++){
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, fish->model->meshes[i].textures[0]);

                glBindVertexArray(fish->model->meshes[i].VAO);
                glDrawElements(GL_TRIANGLES, fish->model->meshes[i].indices.size(), GL_UNSIGNED_INT, (void*) 0);
                glBindVertexArray(0);
            }
        }

        uniformLoc = glGetUniformLocation(program, "alpha");
        glUniform1f(uniformLoc, 0.6);

        glm::mat4 model;
        model = glm::translate(model, aquario.position);
        model = glm::scale(model, aquario.size * 0.65f);

        uniformLoc = glGetUniformLocation(program, "model");
        glUniformMatrix4fv(uniformLoc, 1, GL_FALSE, glm::value_ptr(model));

        for(int i = 0; i < aquarioModel->meshes.size(); i++){
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, aquarioModel->meshes[i].textures[0]);

            glBindVertexArray(aquarioModel->meshes[i].VAO);
            glDrawElements(GL_TRIANGLES, aquarioModel->meshes[i].indices.size(), GL_UNSIGNED_INT, (void*) 0);
            glBindVertexArray(0);
        }

        glDisable(GL_CULL_FACE);

        glUseProgram(uiProgram);
        glClear(GL_DEPTH_BUFFER_BIT);
        glBindVertexArray(0);

        float triangleVertices[] = {
            -1,1,0,
            -1,0.95,0,
            -0.9,0.95,0,
            -0.9,1,0
        };

        GLuint indices[] = {0,1,2, 2,3,0};
        
        GLuint vao;
        GLuint vbo;
        GLuint ebo;

        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);

        glBindVertexArray(vao);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVertices), triangleVertices, GL_STATIC_DRAW);  

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        // vertex positions
        GLuint attribLoc = glGetAttribLocation(uiProgram, "position");
        glEnableVertexAttribArray(attribLoc);	
        glVertexAttribPointer(attribLoc, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);

        //attribLoc = glGetAttribLocation(uiProgram, "inColor");
        //glEnableVertexAttribArray(attribLoc);	
        //glVertexAttribPointer(attribLoc, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(3*sizeof(float)));

        glBindVertexArray(0);
        glBindVertexArray(vao);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
