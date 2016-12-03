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
	float reflectivity;
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

const vec3 sun_location = vec3 (4.0, 3.0, 3.0);


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
	
	info.hit = discriminant > 0;
	float t = (- b - sqrt(discriminant)) / (2 * a);
	info.impact_point = vec3(origin.x + dir.x * t, origin.y + dir.y * t, origin.z + dir.z * t);
	info.impact_normal = normalize(info.impact_point - s1.center);
	info.color = s1.color;
	info.reflectivity = s1.reflectivity;
	return info;
	
	
};

hit_info hitPlane(Plane p1, vec3 origin, vec3 target) {
	float t = dot(p1.normal, (p1.point - origin)) / dot(p1.normal, (target - origin));
	hit_info toReturn;
	toReturn.impact_point = vec3(origin + (t * (target - origin)));
	toReturn.impact_normal = p1.normal;
	toReturn.color = p1.color;
	toReturn.hit = t > 0;
	toReturn.reflectivity = p1.reflectivity;

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
	float closest_dist = 1000000000.0;
	
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

vec4 light_intersection(hit_info info){
	hit_info closest = closest_hit(info.impact_point, sun_location);
	
	//determine shadow
	
	float strength = 0.8;
	float dist_to_sun = length(sun_location - info.impact_point);
	if (length(closest.impact_point - info.impact_point) < dist_to_sun){
		strength = 0;
	}
	// return vec4(info.color, 0.0);
	return vec4(info.color * (0.2 + (strength / pow(dist_to_sun* 0.2f, 2))), 0.0);;
};

vec4 find_color(vec3 rayStart,vec3 rayDir) {
	vec4 finalColor=vec4(0.0);
	float frac=1.0; // fraction of my color to add to finalColor
	for (int raybounce=0;raybounce<5;raybounce++) {
		hit_info i = closest_hit(rayStart,rayStart + rayDir); // geometric search
		vec4 local = light_intersection(i); // diffuse + specular
		// i.reflectivity = 0.5;
		finalColor += local*(1.0-i.reflectivity)*frac;
		frac *= i.reflectivity; // <- scale down all subsequent rays
		rayStart=i.impact_point; // change ray origin for next bounce
		rayDir=reflect(rayDir,i.impact_normal);
	}
	return finalColor;
};






void main() {
  // base pixel colour for image
  vec4 pixel = vec4(0.0, 0.0, 0.0, 0.0);
  // get index in global work group i.e x,y position
  ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
  
	
	float max_x = 7.0;
	float max_y = 5.0;
	float max_z = 5.0;
	// float max_z = 0;
	ivec2 dims = imageSize(dest_tex); // fetch image dimensions
	float x = (float(pixel_coords.x * 2 - dims.x) / dims.x);
	float y = (float(pixel_coords.y * 2 - dims.y) / dims.y);
	float z = (float(pixel_coords.y * 2 - dims.y) / dims.y);
	
	

	vec3 ray_o = vec3(x * max_x, y * max_y, 3 + z * max_z) + vec3(1.0, 1.0, 1.0);
	vec3 ray_d = ray_o + vec3(0.0, 1.0, -1.0);
	
	
	//hit_info res = ;

	// pixel = vec4(closest_hit(ray_o, ray_d).color, 0.0);
	pixel = find_color(ray_o, ray_d - ray_o);

  
  // output to a specific pixel in the image
  imageStore(dest_tex, pixel_coords, pixel);
}