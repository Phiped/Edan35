// Include standard headers
#include <stdio.h>
#include <stdlib.h>

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

#include "shader.hpp"
#include "Physics.h"
#include "utils.h"
#include <math.h>

int main(void)
{
	// Initialise GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(1024, 1024, "Simple Ray Tracer", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders("VertexShader.glsl", "FragShader.glsl");

	glUniform1i(glGetUniformLocation(programID, "srcTex"), 0);


	static const GLfloat g_vertex_buffer_data[] = {
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		1.0f,  1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,
		1.0f,  1.0f, 0.0f,
		-1.0f,  -1.0f, 0.0f,
	};

	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

	GLint computeHandle = genComputeProg();
	double lastTime = glfwGetTime();
	double lastFrame = glfwGetTime();

	int controlled = 0;

	int nbFrames = 0;
	int totFrames = 0;

	Physics p;
	p.init();

	do {
		double currentTime = glfwGetTime();
		nbFrames++;
		totFrames++;
		if (currentTime - lastTime >= 1.0) { // If last prinf() was more than 1 sec ago
											 // printf and reset timer
			printf("%f ms/frame, %f FPS\n", 1000.0 / double(nbFrames), double(nbFrames));
			nbFrames = 0;
			lastTime += 1.0;
		}


		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT);

		// Use our shader
		glUseProgram(computeHandle);
		glUniform1f(glGetUniformLocation(computeHandle, "roll"), (float)totFrames++*0.001f);

		float deltaTime = currentTime - lastFrame;
		p.tick(deltaTime);


		if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
			controlled = 0;
		if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
			controlled = 1;
		if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
			controlled = 2;
		if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
			controlled = 3;
		if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS)
			controlled = 4;

		float push_strength = 5.0;

		if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
			p.spheres[controlled]->velocity += glm::vec3(0.0, push_strength, 0.0) * deltaTime;
		if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
			p.spheres[controlled]->velocity += glm::vec3(0.0, -push_strength, 0.0) * deltaTime;

		if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
			p.spheres[controlled]->velocity += glm::vec3(-push_strength, 0.0, 0.0) * deltaTime;
		if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
			p.spheres[controlled]->velocity += glm::vec3(push_strength, 0.0, 0.0) * deltaTime;

		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
			p.spheres[controlled]->velocity += glm::vec3(0.0, 0.0, push_strength * 2 * deltaTime);

		// add all of our objects


		for (unsigned int i = 0; i < p.spheres.size(); i++) {
			glUniform3f(glGetUniformLocation(computeHandle, ("spheres[" + std::to_string(i) + "].center").c_str()), p.spheres[i]->center.x, p.spheres[i]->center.y, p.spheres[i]->center.z);
			glUniform1f(glGetUniformLocation(computeHandle, ("spheres[" + std::to_string(i) + "].radius").c_str()), p.spheres[i]->radius);
			glUniform3f(glGetUniformLocation(computeHandle, ("spheres[" + std::to_string(i) + "].color").c_str()), p.spheres[i]->color.x, p.spheres[i]->color.y, p.spheres[i]->color.z);
			glUniform1f(glGetUniformLocation(computeHandle, ("spheres[" + std::to_string(i) + "].reflectivity").c_str()), p.spheres[i]->reflectivity);
			glUniform1f(glGetUniformLocation(computeHandle, ("spheres[" + std::to_string(i) + "].refractivity").c_str()), p.spheres[i]->refractivity);
			glUniform1f(glGetUniformLocation(computeHandle, ("spheres[" + std::to_string(i) + "].diffuse").c_str()), p.spheres[i]->diffuse);
		
		}

		for (unsigned int i = 0; i < p.planes.size(); i++) {
			glUniform3f(glGetUniformLocation(computeHandle, ("planes[" + std::to_string(i) +  "].point").c_str()), p.planes[i]->point.x, p.planes[i]->point.y, p.planes[i]->point.z);
			glUniform3f(glGetUniformLocation(computeHandle, ("planes[" + std::to_string(i) + "].normal").c_str()), p.planes[i]->normal.x, p.planes[i]->normal.y, p.planes[i]->normal.z);
			glUniform3f(glGetUniformLocation(computeHandle, ("planes[" + std::to_string(i) + "].color").c_str()), p.planes[i]->color.x, p.planes[i]->color.y, p.planes[i]->color.z);
			glUniform1f(glGetUniformLocation(computeHandle, ("planes[" + std::to_string(i) + "].reflectivity").c_str()), p.planes[i]->reflectivity);
		}

		for (unsigned int i = 0; i < p.boxes.size(); i++) {
			glUniform3f(glGetUniformLocation(computeHandle, ("boxes[" + std::to_string(i) + "].min").c_str()), p.boxes[i]->min.x, p.boxes[i]->min.y, p.boxes[i]->min.z);
			glUniform3f(glGetUniformLocation(computeHandle, ("boxes[" + std::to_string(i) + "].max").c_str()), p.boxes[i]->max.x, p.boxes[i]->max.y, p.boxes[i]->max.z);
			glUniform3f(glGetUniformLocation(computeHandle, ("boxes[" + std::to_string(i) + "].color").c_str()), p.boxes[i]->color.x, p.boxes[i]->color.y, p.boxes[i]->color.z);
			glUniform1f(glGetUniformLocation(computeHandle, ("boxes[" + std::to_string(i) + "].reflectivity").c_str()), p.boxes[i]->reflectivity);

		}


		glUniform3f(glGetUniformLocation(computeHandle, "sun_location"), 4.0, 1.0, 3.0);
		glDispatchCompute(1024 / 16, 1024 / 16, 1); // 512^2 threads in blocks of 16^2

		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);


		glUseProgram(programID);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		//checkErrors("Draw screen");


		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		// Draw the triangle !
		glDrawArrays(GL_TRIANGLES, 0, 3); // 3 indices starting at 0 -> 1 triangle
		glDrawArrays(GL_TRIANGLES, 3, 6);
		//glDisableVertexAttribArray(0);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
		lastFrame = currentTime;
	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0);

	// Cleanup VBO
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteVertexArrays(1, &VertexArrayID);
	glDeleteProgram(programID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();


	return 0;
}