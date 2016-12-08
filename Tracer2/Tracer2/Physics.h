#pragma once

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
// Include GLM
#include <glm/glm.hpp>
#include "shader.hpp"

struct Sphere {
	glm::vec3 center;
	glm::vec3 color;
	glm::vec3 velocity;
	float radius;
	float reflectivity;
	float refractivity;
	float diffuse = 1.0;
};

struct Box {
	glm::vec3 min;
	glm::vec3 max;
	glm::vec3 color;
	float reflectivity;
};

struct Plane {
	glm::vec3 point;
	glm::vec3 normal;
	glm::vec3 color;
	float reflectivity;
};

class Physics
{
public:
	std::vector<Sphere*> spheres;
	std::vector<Box*> boxes;
	std::vector<Plane*> planes;

	float ground = -2.0;

	Physics();
	~Physics();

	void init();
	void tick(float deltaTime);
	bool atGround(Sphere *s);

};

