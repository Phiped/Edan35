#version 430

layout(local_size_x = 16, local_size_y = 16) in;
layout(rgba32f, binding = 0) uniform image2D dest_tex;

uniform float roll;

struct Sphere{
	vec3 center;
	float radius;
	vec3 color;
	float reflectivity;
	float refractivity;
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
	float refractivity;
	bool hit;
};


#define NUM_PLANES 6
#define NUM_SPHERES 3
#define NUM_BOXES 1

#define M_PI 3.1415926535897932384626433832795
#define FOV 1.4

#define NUM_BOUNCES 100

#define BIAS_FACTOR 0.0001f

const vec3 starting_origin = vec3(0.0, -3.0, -1.0);

uniform vec3 sun_location;


uniform Sphere spheres[NUM_SPHERES];
uniform Box boxes[NUM_BOXES];
uniform Plane planes[NUM_PLANES];


vec3 getBias(vec3 origin, vec3 target){
	return (target - origin) * BIAS_FACTOR;
}


hit_info hitSphere(Sphere s1, vec3 origin, vec3 target){
	vec3 dir = normalize(target - origin);
	vec3 dist = origin - s1.center;
	float a = dot(dir, dir);
	float b = 2 * dot(dir, dist);
	float c = dot(dist, dist) - pow(s1.radius,2);
	
	float discriminant = pow(b, 2) - (4.0 * a * c);
	hit_info info;
	
	info.hit = discriminant >= 0;
	if (info.hit == false){
		return info;
	}
	float sqrt_d = sqrt(discriminant);	
	float t = (- b - sqrt_d) / (2 * a);

	if (t < 0){
		info.hit = false;
		return info;
	}

	info.impact_point = origin + (t * dir);
	info.impact_point += getBias(info.impact_point, s1.center);
	info.impact_normal = normalize(info.impact_point - s1.center);

	info.reflectivity = s1.reflectivity;
	info.refractivity = s1.refractivity;

	info.color = s1.color;
	
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
	toReturn.refractivity = 0;


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
	vec3 offset = info.impact_point - ((b.min + b.max) / 2);
	
	/// TODO fix the retarded box reflections that aren't working at all
	
	if (abs(offset.x) > abs(offset.y) && abs(offset.x) > abs(offset.z)){
		info.impact_normal = vec3(normalize(offset.x), 0.0, 0.0);
	} else if (abs(offset.y) > abs(offset.z)) {
		info.impact_normal = vec3(0.0, normalize(offset.y), 0.0);
	} else{
		info.impact_normal = vec3(0.0, 0.0, normalize(offset.z));
	}
	
	info.reflectivity = b.reflectivity;
	info.refractivity = 0;
	
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
	float accumulated_block = 0.0;
	
	float dist_to_sun = length(sun_location - modified);
	while (length(closest.impact_point - modified) < dist_to_sun && accumulated_block < 1.0){
		accumulated_block += (1-closest.refractivity);
		accumulated_block = min(accumulated_block, 1.0);
		closest = closest_hit(closest.impact_point, sun_location);
	}
	
	//determine shadow
	float strength = (1-accumulated_block);
	// if (length(closest.impact_point - modified) < dist_to_sun){
		// strength = 0;
	// }
	return vec4(info.color * strength / pow(dist_to_sun* 0.18f, 2), 0.0);
};

vec4 find_color2(vec3 rayStart,vec3 rayDir, float frac) {
	vec4 finalColor=vec4(0.0);
	for (int raybounce=0;raybounce<NUM_BOUNCES && frac > 0.05;raybounce++) {
		hit_info i = closest_hit(rayStart,rayStart + rayDir); // geometric search
		vec4 local = light_intersection(i); // diffuse + specular
		
		if (i.refractivity > 0){
			finalColor += local*(1.0-i.refractivity-i.reflectivity)*frac;
			frac *= i.refractivity; // <- scale down all subsequent rays
			vec3 dist = refract(rayDir, i.impact_normal, 0.97);
			rayDir = dist;
		} else{
			finalColor += local*(1.0-i.reflectivity)*frac;
			frac *= i.reflectivity; // <- scale down all subsequent rays
			rayDir=reflect(rayDir,i.impact_normal);
		}
		rayStart=i.impact_point;


	}
	return finalColor;
};

vec4 find_color(vec3 rayStart,vec3 rayDir) {
	vec4 finalColor=vec4(0.0);
	float frac=1.0; // fraction of my color to add to finalColor
	for (int raybounce=0;raybounce<NUM_BOUNCES && frac > 0.05;raybounce++) {
		hit_info i = closest_hit(rayStart,rayStart + rayDir); // geometric search
		vec4 local = light_intersection(i); // diffuse + specular
		
		// since recursion is not allowed, we can only allow as many "splits" as new functions we have, this is a workaround (hack)
		if (i.refractivity > 0){
			finalColor += local*(1.0-i.refractivity-i.reflectivity)*frac;
			vec4 reflected = find_color2(i.impact_point, reflect(rayDir,i.impact_normal), i.reflectivity);
			finalColor += reflected;
			frac *= i.refractivity; // <- scale down all subsequent rays
			vec3 dist = refract(rayDir, i.impact_normal, 0.8);
			rayDir = dist;
		} else{
			finalColor += local*(1.0-i.reflectivity)*frac;
			frac *= i.reflectivity; // <- scale down all subsequent rays
			rayDir=reflect(rayDir,i.impact_normal);
		}
		rayStart=i.impact_point;


	}
	return finalColor;
};





void main() {
  // base pixel colour for image	
  vec4 pixel = vec4(0.0, 0.0, 0.0, 0.0);
  // get index in global work group i.e x,y position
  ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
  
	ivec2 dims = imageSize(dest_tex); // fetch image dimensions


	float x = (float((pixel_coords.x)* 2 - dims.x) / dims.x);
	float y = (float((pixel_coords.y)* 2 - dims.y) / dims.y);
	vec3 ray_o = starting_origin;
	vec3 ray_d = (ray_o + vec3(x, 1.0, y));
	pixel = find_color(ray_o, ray_d - ray_o);

	imageStore(dest_tex, pixel_coords, pixel);
}