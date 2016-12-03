#version 430

layout(local_size_x = 16, local_size_y = 16) in;
layout(rgba32f, binding = 0) uniform image2D dest_tex;

uniform float roll;

struct Sphere{
	vec3 center;
	float radius;
	vec3 color;
	float reflectivity;
};

struct Box{
	vec3 min;
	vec3 max;
	vec3 color;
};

struct Plane{
	vec3 point;
	vec3 normal;
	vec3 color;
};

struct hit_info{
	vec3 impact_point;
	vec3 impact_normal;
	vec3 color;
	float reflectivity;
	bool hit;
};


const int NUM_BOXES = 1;
const int NUM_SPHERES = 1;
const int NUM_PLANES = 2;


uniform Sphere spheres[NUM_SPHERES];
uniform Box boxes[NUM_BOXES];
uniform Plane planes[NUM_PLANES];

// vec4 find_color(vec3 rayStart,vec3 rayDir) {
	// vec4 finalColor=vec4(0.0);
	// float frac=1.0; // fraction of my color to add to finalColor
	// for (int raybounce=0;raybounce<max_bounces;raybounce++) {
		// ray_intersection i = closest_hit(rayStart,rayDir); // geometric search
		// vec4 local = light_intersection(i); // diffuse + specular
		// finalColor += local*(1.0-i.mirror)*frac;
		// frac *= i.mirror; // <- scale down all subsequent rays
		// if (frac<0.05) break; // give up--not much impact on final color
		// rayStart=i.hit_location; // change ray origin for next bounce
		// rayDir=reflect(rayDir,i.normal);
	// }x
	// return finalColor;
// }






hit_info hitSphere(Sphere s1, vec3 origin, vec3 target){
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
		info.impact_normal = normalize(info.impact_point - s1.center);
		info.color = s1.color;
		return info;
	}
	
	
};

hit_info hitPlane(Plane p1, vec3 origin, vec3 target) {
	float t = dot(p1.normal, (p1.point - origin)) / dot(p1.normal, (target - origin));
	hit_info toReturn;
	toReturn.impact_point = vec3(origin + (t * (target - origin)));
	toReturn.impact_normal = p1.normal;
	toReturn.color = p1.color;
	toReturn.hit = t > 0;
	return toReturn;
};

hit_info hitBox(Box b, vec3 origin, vec3 target) {
	vec3 dir = target - origin;
	vec3 tMin = (b.min - origin) / dir;
	vec3 tMax = (b.max - origin) / dir;
	vec3 t1 = min(tMin, tMax);
	vec3 t2 = max(tMin, tMax);
	float tNear = max(max(t1.x, t1.y), t1.z);
	float tFar = min(min(t2.x, t2.y), t2.z);
	hit_info toReturn;
	if (tFar < tNear){
		toReturn.hit = false;
		return toReturn;
	} else{
		toReturn.hit = true;
		toReturn.impact_point = origin + tNear * dir;
		return toReturn;
	}
};


hit_info closest_hit(vec3 origin, vec3 target){
	float closest_dist = 100000000000000.0;
	
	hit_info closest;
	hit_info current;
	for (int i = 0; i < NUM_SPHERES; i++){
		current = hitSphere(spheres[i], origin, target);
		if (current.hit){
			float dist = length(current.impact_point - origin);
			if (dist < closest_dist){
				closest = current;
				closest_dist = dist;
			}
		}
	}
	
	// for (int i = 0; i < NUM_BOXES; i++){
		// current = hitBox(boxes[i], origin, target);
		// if (current.hit){
			// float dist = length(current.impact_point - origin);
			// if (dist < closest_dist){
				// closest = current;
				// closest_dist = dist;
			// }
		// }
	// }
	
	for (int i = 0; i < NUM_PLANES; i++){
		current = hitPlane(planes[i], origin, target);
		if (current.hit){
			float dist = length(current.impact_point - origin);
			if (dist < closest_dist){
				closest = current;
				closest_dist = dist;
			}
		}
	}
	return closest;
};


void main() {
  // base pixel colour for image
  vec4 pixel = vec4(0.0, 0.0, 0.0, 0.0);
  // get index in global work group i.e x,y position
  ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
  
  
	float max_x = 5.0;
	float max_y = 5.0;
	//float max_z = 5.0;
	ivec2 dims = imageSize(dest_tex); // fetch image dimensions
	float x = (float(pixel_coords.x * 2 - dims.x) / dims.x);
	float y = (float(pixel_coords.y * 2 - dims.y) / dims.y);
	
	
	
	// spheres[0] = Sphere(vec3(1.0, 1.0, -6.0) + vec3(0.5 * sin(roll), 0.5 * cos(roll), 0.5), 1.0, vec3(0.4, 0.4, 1.0));
	// planes[0] = Plane(vec3(-4.0, -4.0, -7.0), vec3(0.0, 0.0, 1), vec3(1, 1, 0.5));
	// s1.radius = 1.0;
	// s1.center = vec3(1.0, 1.0, -6) + vec3(0.5 * sin(roll), 0.5 * cos(roll), 0.5);
	// s1.color = vec3(0.4, 0.4, 1.0); // mostly blue
	
	// Plane p1;
	// p1.point = vec3(-4.0, -4.0, -7.0);
	// p1.normal = vec3(0.0, 0.0, 1);
	// p1.color = vec3(1, 1, 0.5);
	
	vec3 light_source = vec3 (4.0, 1.0, 1.0);

	vec3 ray_o = vec3(x * max_x, y * max_y, 0.0);
	vec3 ray_d = ray_o + vec3(0.0, 0.0, -1.0); // ortho
	
	
	hit_info res = closest_hit(ray_o, ray_d);
	
	pixel = vec4(res.color, 0.0);

  
  // output to a specific pixel in the image
  imageStore(dest_tex, pixel_coords, pixel);
}