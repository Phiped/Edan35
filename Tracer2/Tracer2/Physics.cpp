#include "physics.h"
#include <algorithm>
#include <ctime>

Physics::Physics()
{
}


Physics::~Physics()
{
}

float getRandomDir() {
	return static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 10)) - 5;
}

void Physics::init() {

	srand(static_cast <unsigned> (time(0)));

	Sphere *s1 = new Sphere();
	s1->center = glm::vec3(0.0, 4.0, 1.1);
	s1->color = glm::vec3(1.0, 0.4, 0.4);
	s1->radius = 1.0;
	s1->reflectivity = 0.1;
	s1->refractivity = 0.9;
	s1->velocity = glm::vec3(0.0, 0.0, 0.0);
	//s1->velocity = glm::vec3(getRandomDir(), getRandomDir(), getRandomDir());

	Sphere *s2 = new Sphere();
	s2->center = glm::vec3(0.0, 4.0, 3.2);
	s2->color = glm::vec3(0.4, 1.0, 0.4);
	s2->radius = 1.0;
	s2->reflectivity = 0.1;
	s2->refractivity = 0.0;
	s2->velocity = glm::vec3(0.0, 0.0, 0.0);
	//s2->velocity = glm::vec3(getRandomDir(), getRandomDir(), getRandomDir());

	Sphere *s3 = new Sphere();
	s3->center = glm::vec3(0.0, 4.0, -1.0);
	s3->color = glm::vec3(0.4, 1.0, 0.4);
	s3->radius = 1.0;
	s3->reflectivity = 1.0;
	s3->refractivity = 0.0;
	s3->velocity = glm::vec3(0.1f, 0.2f, 0.0);
	//s3->velocity = glm::vec3(getRandomDir(), getRandomDir(), getRandomDir());

	spheres.push_back(s1);
	spheres.push_back(s2);
	spheres.push_back(s3);

	Plane *p1 = new Plane();
	p1->point = glm::vec3(0.0, 0.0, -2.0);
	p1->normal = glm::vec3(0.0, 0.0, -1.0);
	p1->color = glm::vec3(0.9, 0.9, 0.5);
	p1->reflectivity = 0.3;

	Plane *p2 = new Plane();
	p2->point = glm::vec3(0.0, 0.0, 10.0);
	p2->normal = glm::vec3(0.0, 0.0, 1.0);
	p2->color = glm::vec3(0.4, 0.7, 0.8);
	p2->reflectivity = 0.3;

	Plane *p3 = new Plane();
	p3->point = glm::vec3(0.0, 6.0, 0.0);
	p3->normal = glm::vec3(0.0, 1.0, 0.0);
	p3->color = glm::vec3(0.4, 0.7, 0.8);
	p3->reflectivity = 0.3;

	Plane *p4 = new Plane();
	p4->point = glm::vec3(6.0, 0.0, 0.0);
	p4->normal = glm::vec3(1.0, 0.0, 0.0);
	p4->color = glm::vec3(0.4, 0.7, 0.8);
	p4->reflectivity = 0.3;

	Plane *p5 = new Plane();
	p5->point = glm::vec3(-6.0, 0.0, 0.0);
	p5->normal = glm::vec3(-1.0, 0.0, 0.0);
	p5->color = glm::vec3(0.4, 0.7, 0.8);
	p5->reflectivity = 0.3;

	Plane *p6 = new Plane();
	p6->point = glm::vec3(0.0, -4.0, 0.0);
	p6->normal = glm::vec3(0.0, -1.0, 0.0);
	p6->color = glm::vec3(0.4, 0.7, 0.8);
	p6->reflectivity = 0.3;

	planes.push_back(p1);
	planes.push_back(p2);
	planes.push_back(p3);
	planes.push_back(p4);
	planes.push_back(p5);
	planes.push_back(p6);

	Box *b1 = new Box();
	b1->min = glm::vec3(-2.0, 9.0, -2.0);
	b1->max = glm::vec3(-1.0, 8.0, -1.0);
	b1->color = glm::vec3(1.0, 0.6, 0.5);
	b1->reflectivity = 0.0;

	//boxes.push_back(b1);

}

bool Physics::atGround(Sphere *s) {
	return (s->center.z - s->radius - ground) < 0.01f;

}

void Physics::tick(float deltaTime) {
	float frictionFactor = 0.05f;
	float gravityForce = 10.0f;

	for (int i = 0; i < spheres.size(); i++) {
		// determine collision and move along movement vector
		Sphere *s = spheres[i];
		s->center += s->velocity * deltaTime;
		for (int j = i; j < spheres.size(); j++) {
			Sphere *s2 = spheres[j];
			if (s != s2 && glm::length(s->center - s2->center) <= s->radius + s2->radius) {

				// collision detected
				glm::vec3 x = glm::normalize(s2->center - s->center);

				glm::vec3 location = s->center + x * s->radius;

				s2->center = location + x * s2->radius;

				float mass_1 = pow(s->radius, 2);
				float mass_2 = pow(s2->radius, 2);

				float x1 = glm::dot(x, s->velocity);
				glm::vec3 v1x = x1 * x;
				glm::vec3 v1y = s->velocity -v1x;
				float m1 = mass_1;

				float x2 = glm::dot(x, s2->velocity);
				glm::vec3 v2x = x2 * x;
				glm::vec3 v2y = s2->velocity - v2x;
				float m2 = mass_2;

				s->velocity = v1x * (m1 - m2) / (m1 + m2) + v2x * (2 * m2) / (m1 + m2) + v1y;
				s2->velocity = v1x * (2 * m1) / (m1 + m2) + v2x *(m2 - m1) / (m1 + m2) + v2y;
			}
		}
		for (Plane *p : planes) {
			float num = -dot(p->normal, p->point);
			float distance = std::abs(p->normal.x * s->center.x + p->normal.y * s->center.y + p->normal.z * s->center.z + num) / std::sqrt(p->normal.x *p->normal.x + p->normal.y * p->normal.y + p->normal.z * p->normal.z);
			if (distance <= s->radius && glm::dot(p->normal, s->velocity) > 0) {
				// collision detected
				//std::cout << "COLLISION";
				s->velocity = s->velocity - 2 * glm::dot(p->normal, s->velocity) * p->normal;
				s->velocity.z *= 0.8;
			}
		}


		if (atGround(s)) {
			// reduce speeds
			if (s->velocity.x > 0) {
				s->velocity.x = std::max(0.0f, s->velocity.x - deltaTime * frictionFactor);
			}
			else {
				s->velocity.x = std::min(0.0f, s->velocity.x + deltaTime * frictionFactor);
			}
			if (s->velocity.y > 0) {
				s->velocity.y = std::max(0.0f, s->velocity.y - deltaTime * frictionFactor);
			}
			else {
				s->velocity.y = std::min(0.0f, s->velocity.y + deltaTime * frictionFactor);
			}
			if (abs(s->velocity.z) < 0.15f) {
				s->velocity.z = 0;
			}
		}
		else {
			s->velocity.z -= gravityForce * deltaTime;
		}



		//if (s->center.z < ground + s->radius) {
		//	s->center.z = ground + s->radius;
		//	//s->velocity.z = 0;
		//}
		//s->center.z = std::max(s->center.z, ground + s->radius + 0.1f);
		//s->center.z = std::max(s->center.z, ground + s->radius);

	}
}