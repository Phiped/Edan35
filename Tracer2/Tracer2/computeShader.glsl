#version 430
layout(local_size_x = 16, local_size_y = 16) in;
uniform image2D dest_tex; //layout(rgba32f, binding = 0) 

uniform float roll;

struct hit_info{
	vec3 impact_point;
	bool hit;
};

struct Sphere{
	vec3 center;
	float radius;
	vec3 color;
};

struct Block{
	vec3 point;
	vec3 expansion;
	vec3 color;
};

struct Plane{
	vec3 point;
	vec3 normal;
	vec3 color;
};

hit_info hitSphere(Sphere s1, vec3 origin, vec3 target){
	// vec3 omc = origin - s1.center;
	// float b = dot(target, omc);
	// float c = dot(omc, omc) - s1.radius * s1.radius;
	// float bsqmc = b * b - c;
	// hit one or both sides
	// return bsqmc >= 0.0;
	
	vec3 dir = target - origin;
	float a = pow(dir.x, 2) + pow(dir.y, 2) + pow(dir.z, 2);
	float b = 2 * dir.x * (origin.x - s1.center.x) + 2*dir.y*(origin.y - s1.center.y) + 2*dir.z*(origin.z - s1.center.z);
	float c = pow(s1.center.x, 2) + pow(s1.center.y, 2) + pow(s1.center.z, 2) + pow(origin.x, 2) + pow(origin.y, 2) + pow(origin.z, 2) - 2*(s1.center.x * origin.x + s1.center.y * origin.y + s1.center.z * origin.z) - pow(s1.radius, 2);
	
	float discriminant = pow(b, 2) - (4 * a * c);
	hit_info info;
	
	if (discriminant <= 0){
		info.hit = false;
		return info;
	} else{
		info.hit = true;
		float t = (- b - sqrt(discriminant)) / (2 * a);
		info.impact_point = vec3(origin.x + dir.x * t, origin.y + dir.y * t, origin.z + dir.z * t);
		return info;
	}
	
	
};

hit_info hitPlane(Plane p1, vec3 origin, vec3 target) {
	float t = dot(p1.normal, (p1.point - origin)) / dot(p1.normal, (target - origin));
	hit_info toReturn;
	toReturn.impact_point = vec3(origin + (t * (target - origin)));
	toReturn.hit = t > 0;
	return toReturn;
};

bool hitBlock(Block b1, vec3 origin, vec3 target) {
	return false;
};

void main() {
  // base pixel colour for image
  vec4 pixel = vec4(0.0, 0.0, 0.0, 0.0);
  // get index in global work group i.e x,y position
  ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
  
  
	float max_x = 5.0;
	float max_y = 5.0;
	ivec2 dims = imageSize(dest_tex); // fetch image dimensions
	float x = (float(pixel_coords.x * 2 - dims.x) / dims.x);
	float y = (float(pixel_coords.y * 2 - dims.y) / dims.y);
	
	Sphere s1;
	s1.radius = 1.0;
	s1.center = vec3(1.0, 1.0, -4) + vec3(0.5 * sin(roll), 0.5 * cos(roll), 0.5);
	s1.color = vec3(0.4, 0.4, 1.0); // mostly blue
	
	Plane p1;
	p1.point = vec3(-4.0, -4.0, -5.0);
	p1.normal = vec3(0.0, 0.0, 1);
	p1.color = vec3(1, 1, 0.5);
	
	vec3 light_source = vec3 (4.0, 1.0, 1.0);

	vec3 camera_offset = vec3(0, -5, 0);
	vec3 ray_o = vec3(x * max_x, y * max_y, 0.0) + camera_offset;
	vec3 ray_d = ray_o + vec3(0.0, 1.0, -1.0); // ortho
	
	hit_info t = hitPlane(p1, ray_o, ray_d);

	hit_info s = hitSphere(s1, ray_o, ray_d);
	if (s.hit){
		pixel = vec4(s1.color / pow(length(light_source - s.impact_point)*0.2, 2), 0);
	}
	else if (t.hit){
		vec3 target = light_source;
		float strength = 1;
		if (hitSphere(s1, t.impact_point, target).hit){
			strength = 0.2;
		} else{
			// quadratic light falloff
			strength = 0.2 + (0.8 / pow(length(target - t.impact_point)*0.15, 2));
		}
		pixel = vec4(p1.color * strength, 0);
	}

  
  // output to a specific pixel in the image
  imageStore(dest_tex, pixel_coords, pixel);
}