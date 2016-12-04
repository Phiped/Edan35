#version 430

layout(local_size_x = 16, local_size_y = 16) in;
layout(rgba32f, binding = 0) uniform image2D dest_tex;

uniform float roll;

struct Sphere{
	vec3 center;
	float radius;
	vec3 color;
	float reflectivity;
	bool refracting;
};

struct Box{
	vec3 min;
	vec3 max;
	vec3 color;
	float reflectivity;
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


#define NUM_PLANES 5
#define NUM_SPHERES 1
#define NUM_BOXES 1

#define M_PI 3.1415926535897932384626433832795
#define FOV 1.4

#define NUM_BOUNCES 5

#define BIAS_FACTOR 0.000001f

const vec3 starting_origin = vec3(0.0, -3.0, 0.0);

const vec3 sun_location = vec3 (3.0, 2.0, 3.0);


uniform Sphere spheres[NUM_SPHERES];
uniform Box boxes[NUM_BOXES];
uniform Plane planes[NUM_PLANES];


vec3 getBias(vec3 origin, vec3 target){
	return (target - origin) * BIAS_FACTOR;
}


hit_info hitSphere(Sphere s1, vec3 origin, vec3 target){
	vec3 dir = target - origin;
	float a = pow(dir.x, 2) + pow(dir.y, 2) + pow(dir.z, 2);
	float b = 2 * dir.x * (origin.x - s1.center.x) + 2*dir.y*(origin.y - s1.center.y) + 2*dir.z*(origin.z - s1.center.z);
	float c = pow(s1.center.x, 2) + pow(s1.center.y, 2) + pow(s1.center.z, 2) + pow(origin.x, 2) + pow(origin.y, 2) + pow(origin.z, 2) - 2*(s1.center.x * origin.x + s1.center.y * origin.y + s1.center.z * origin.z) - pow(s1.radius, 2);
	
	float discriminant = pow(b, 2) - (4 * a * c);
	hit_info info;
	
	info.hit = discriminant > 0;
	float t = (- b - sqrt(discriminant)) / (2 * a);
	info.impact_point = vec3(origin.x + (dir.x * t), origin.y +(dir.y * t), origin.z + (dir.z * t));
	info.impact_point += getBias(origin, info.impact_point);
	
	
	info.impact_normal = info.impact_point - s1.center;
	info.color = s1.color;
	info.reflectivity = s1.reflectivity;
	return info;
	
	
};

hit_info hitPlane(Plane p1, vec3 origin, vec3 target) {
	float t = dot(p1.normal, (p1.point - origin)) / dot(p1.normal, (target - origin));
	hit_info toReturn;
	toReturn.impact_point = vec3(origin + (t * (target - origin)));
	toReturn.impact_point += getBias(toReturn.impact_point, origin);
	toReturn.impact_normal = p1.normal;
	toReturn.color = p1.color;
	toReturn.hit = t > 0;
	toReturn.reflectivity = p1.reflectivity;

	return toReturn;
};

hit_info hitBox(Box b, vec3 origin, vec3 target) {

	vec3 dir = target - origin;
	float tx1 = (b.min.x - origin.x) / dir.x;
	float tx2 = (b.max.x - origin.x) / dir.x;
	
	float tmin = min(tx1, tx2);
	float tmax = max(tx1, tx2);
	
	float ty1 = (b.min.y - origin.y) / dir.y;
	float ty2 = (b.max.y - origin.y) / dir.y;
	
	tmin = max(tmin, min(ty1, ty2));
	tmax = min(tmax, max(ty1, ty2));
	
	float tz1 = (b.min.z - origin.z) / dir.z;
	float tz2 = (b.max.z - origin.z) / dir.z;
	
	tmin = max(tmin, min(tz1, tz2));
	tmax = min(tmax, max(tz1, tz2));
	
	hit_info info;
	info.hit = tmin < tmax;
	info.color = b.color;
	info.impact_point = origin + dir * tmin;
	
	if (tmin == min(tz1, tz2)){
		info.impact_normal = vec3(0.0, 0.0, 1.0);
	} else if (tmin == min(tx1, tx2)) {
		info.impact_normal = vec3(1.0, 0.0, 0.0);
	} else{
		info.impact_normal = vec3(0.0, 1.0, 0.0);
	}
	
	info.reflectivity = b.reflectivity;
	
	return info;
};


hit_info closest_hit(vec3 origin, vec3 target){
	float closest_dist = 1000000.0;
	
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
	
	for (int i = 0; i < NUM_BOXES; i++){
		current = hitBox(boxes[i], origin, target);
		if (current.hit){
			float dist = length(current.impact_point - origin);
			if (dist < closest_dist){
				closest = current;
				closest_dist = dist;
			}
		}
	}
	
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
	vec3 modified =  info.impact_point + (sun_location - info.impact_point)*0.01f;

	hit_info closest = closest_hit(modified, sun_location);
	
	//determine shadow
	
	float strength = 0.7;
	float dist_to_sun = length(sun_location - modified);
	if (length(closest.impact_point - modified) < dist_to_sun){
		strength = 0;
	}
	// return vec4(info.color / pow(dist_to_sun* 0.2f, 2), 0.0);
	return vec4(info.color * (0.3 + strength) / pow(dist_to_sun* 0.18f, 2), 0.0);
};

vec4 find_color(vec3 rayStart,vec3 rayDir) {
	vec4 finalColor=vec4(0.0);
	float frac=1.0; // fraction of my color to add to finalColor
	for (int raybounce=0;raybounce<NUM_BOUNCES;raybounce++) {
		hit_info i = closest_hit(rayStart,rayStart + rayDir); // geometric search
		vec4 local = light_intersection(i); // diffuse + specular
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
  
	
	float max_x = 5.0;
	float max_y = 5.0;
	
	// float max_z = 5.0;
	ivec2 dims = imageSize(dest_tex); // fetch image dimensions



	float x = (float((pixel_coords.x)* 2 - dims.x) / dims.x);
	float y = (float((pixel_coords.y)* 2 - dims.y) / dims.y);
	vec3 ray_o = starting_origin;
	vec3 ray_d = (ray_o + vec3(x, 1.0, y));
	pixel = find_color(ray_o, ray_d - ray_o);

		
	// total = 

  	// pixel = vec4(closest_hit(ray_o, ray_d).color, 0.0);

  // output to a specific pixel in the image
  imageStore(dest_tex, pixel_coords, pixel);
}