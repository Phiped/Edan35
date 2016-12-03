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

	int nbFrames = 0;
	int totFrames = 0;
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


		// add all of our objects

		//spheres[0] = Sphere(vec3(1.0, 1.0, -6.0) + vec3(0.5 * sin(roll), 0.5 * cos(roll), 0.5), 1.0, vec3(0.4, 0.4, 1.0));
		//planes[0] = Plane(vec3(-4.0, -4.0, -7.0), vec3(0.0, 0.0, 1), vec3(1, 1, 0.5));




		glUniform3f(glGetUniformLocation(computeHandle, "spheres[0].center"), 1.0 + sin(totFrames++*0.001f), 1.0 + cos(totFrames++*0.001f), 0.0);
		glUniform1f(glGetUniformLocation(computeHandle, "spheres[0].radius"), 1.0);
		glUniform1f(glGetUniformLocation(computeHandle, "spheres[0].reflectivity"), 0.4);
		glUniform3f(glGetUniformLocation(computeHandle, "spheres[0].color"), 0.4, 0.4, 1.0);

		glUniform3f(glGetUniformLocation(computeHandle, "planes[0].point"), 0.0, 0.0, -10.0);
		glUniform3f(glGetUniformLocation(computeHandle, "planes[0].normal"), 0.0, 0.0, 1.0);
		glUniform3f(glGetUniformLocation(computeHandle, "planes[0].color"), 0.9, 0.9, 0.5);

		//glUniform3f(glGetUniformLocation(computeHandle, "planes[1].point"), 0.0, 0.0, -10.0);
		//glUniform3f(glGetUniformLocation(computeHandle, "planes[1].normal"), 0.0, 0.0, 1.0);
		//glUniform3f(glGetUniformLocation(computeHandle, "planes[1].color"), 0.9, 0.5, 0.5);




		//glUniform3

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