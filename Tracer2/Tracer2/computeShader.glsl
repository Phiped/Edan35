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
	float diffuse;
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
	float diffuse;
	float gloss;
	int type;
	bool hit;
};


#define NUM_PLANES 6
#define NUM_SPHERES 4	
#define NUM_BOXES 1

#define M_PI 3.1415926535897932384626433832795

#define NUM_BOUNCES 100

#define BIAS_FACTOR 0.0001f

const vec3 starting_origin = vec3(0.0, -3.0, -1.0);

uniform vec3 sun_location;


uniform Sphere spheres[NUM_SPHERES];
uniform Box boxes[NUM_BOXES];
uniform Plane planes[NUM_PLANES];


ivec2 pixel_coords;

vec3 getBias(vec3 origin, vec3 target){
	return (target - origin) * BIAS_FACTOR;
}


hit_info hitSphere(Sphere s1, vec3 origin, vec3 dir){
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
	info.diffuse = s1.diffuse;
	info.type = 0;
	
	
	return info;
	
	
};

hit_info hitPlane(Plane p1, vec3 origin, vec3 dir) {
	float t = dot(p1.normal, (p1.point - origin)) / dot(p1.normal, dir);
	
	hit_info toReturn;
	
	toReturn.hit = t > 0;
	if (!toReturn.hit){
		return toReturn;
	}
	toReturn.impact_point = vec3(origin + (t * dir));
	toReturn.impact_point += getBias(toReturn.impact_point, origin);
	toReturn.impact_normal = -p1.normal;
	toReturn.color = p1.color;
	if (abs(p1.normal.z) - 1.0 == 0.0 && !(mod((toReturn.impact_point.x + toReturn.impact_point.y), 2) > 1.0)){
		toReturn.color = vec3(1.0, 1.0, 1.0);
	}
	
	toReturn.reflectivity = p1.reflectivity;
	toReturn.refractivity = 0;
	toReturn.diffuse = 0.0;
	toReturn.type = 1;


	return toReturn;
};

hit_info hitBox(Box b, vec3 origin, vec3 dir) {
 
	//vec3 dir = target - origin;
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
	info.hit = tmin < tmax && tmax > 0;
	info.color = b.color;
	info.impact_point = origin + dir * tmin;
	info.reflectivity = b.reflectivity;
	info.refractivity = 0.0;
	info.type = 2;
	info.diffuse = 0.05;
	
	
	vec3 impact = info.impact_point;
	if(abs(impact.x - b.min.x) < BIAS_FACTOR){
		info.impact_normal =normalize(vec3((b.min.x - b.max.x), 0.0 ,0.0));
	}else if(abs(impact.x - b.max.x) < BIAS_FACTOR){
		info.impact_normal =normalize(vec3((b.max.x - b.min.x), 0.0 ,0.0));
	}
	else if(abs(impact.y - b.min.y) < 0.1f){
		info.impact_normal =normalize(vec3(0.0,(b.min.y - b.max.y),0.0));
	}
	else if(abs(impact.y - b.max.y) < 0.1f){
		info.impact_normal =normalize(vec3(0.0,(b.max.y - b.min.y),0.0));
	}
	else if(abs(impact.z - b.min.z) < BIAS_FACTOR){
		info.impact_normal =normalize(vec3(0.0,0.0,(b.min.z - b.max.z)));
	}else if(abs(impact.z - b.max.z) < BIAS_FACTOR){
		info.impact_normal =normalize(vec3(0.0,0.0,(b.max.z - b.min.z)));
	}
	
	info.impact_point += info.impact_normal * 0.001f;
	//info.impact_normal = vec3(0.0, 1.0, 0.0);
	
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
	vec3 sunDir = normalize(sun_location - info.impact_point);
	vec3 modified =  info.impact_point + sunDir * 0.01f;

	
	hit_info closest = closest_hit(modified, sun_location - modified);
	float accumulated_block = 0.0;
	
	float dist_to_sun = length(sun_location - modified);
	
	if (length(closest.impact_point - modified) < dist_to_sun){
		accumulated_block = 0.5;
	}
	
	float strength = (1-accumulated_block);
	float factor  = 1.0;
	if (info.type == 0)
		factor = max(info.reflectivity, dot(info.impact_normal, sunDir) + 0.3); //info.diffuse * max(0.0, dot(info.impact_normal, sunDir)) + (1-info.diffuse);
	
	return  factor * vec4(info.color * (0.0 + strength) / pow(dist_to_sun* 0.2f, 2), 0.0);
};

float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}


vec4 find_color2(vec3 rayStart,vec3 rayDir, float frac) {
	vec4 finalColor=vec4(0.0);
	for (int raybounce=0;raybounce<NUM_BOUNCES && frac > 0.05;raybounce++) {
		hit_info i = closest_hit(rayStart,rayDir); // geometric search
		vec4 local = light_intersection(i); // diffuse + specular
		
		if (i.refractivity > 0){
			finalColor += local*(1.0-i.refractivity-i.reflectivity)*frac;
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


vec4 find_color(vec3 rayStart,vec3 rayDir) {
	vec4 finalColor=vec4(0.0);
	float frac=1.0; // fraction of my color to add to finalColor
	for (int raybounce=0;raybounce<NUM_BOUNCES && frac > 0.05;raybounce++) {
		hit_info i = closest_hit(rayStart, rayDir); // geometric search
		vec4 local = light_intersection(i); // diffuse + specular
		// since recursion is not allowed, we can only allow as many "splits" as new functions we have, this is a workaround (hack)
		if (i.refractivity > 0){
			finalColor += local*(1.0-i.refractivity - i.reflectivity)*frac;
			vec4 reflected = find_color2(i.impact_point, reflect(rayDir,i.impact_normal), i.reflectivity);
			finalColor += reflected;
			frac *= i.refractivity; // <- scale down all subsequent rays		
			vec3 dist = refract(rayDir, i.impact_normal, 0.8);
			rayDir = dist;
		} else{		
			rayDir=reflect(rayDir,i.impact_normal);
			finalColor += local*(1.0-i.reflectivity)*frac;
			frac *= i.reflectivity; // <- scale down all subsequent rays

			if (i.diffuse > 0){
				vec4 total = vec4(0.0);
				for (float j = 0; j < 1; j++){
						vec3 newDir = vec3(rayDir.x + mix(0.5, -0.5, rand(pixel_coords.xy + vec2(j,j))) * i.diffuse ,rayDir.y + mix(0.5, -0.5, rand(pixel_coords.xy + rand(vec2(j,-j + 1)) )) * i.diffuse, rayDir.z + mix(0.5, -0.5, rand(pixel_coords.xy + vec2(1 + j,-j))) * i.diffuse);
						total += find_color2(i.impact_point, newDir, frac);
				}
				finalColor += total;
				return finalColor;
			} 
		}
		rayStart=i.impact_point;
	}
	return finalColor;
};





void main() {
  // base pixel colour for image	
  vec4 pixel = vec4(0.0, 0.0, 0.0, 0.0);
  // get index in global work group i.e x,y position
  pixel_coords = ivec2(gl_GlobalInvocationID.xy);
	
	
	
	ivec2 dims = imageSize(dest_tex); // fetch image dimensions

	vec4 total = vec4(0.0);
	float x = (float((pixel_coords.x)* 2 - dims.x) / dims.x);
	float y = (float((pixel_coords.y)* 2 - dims.y) / dims.y);
	vec3 ray_o = starting_origin;
	vec3 ray_d = normalize(vec3(x, 1.0, y));
	pixel = find_color(ray_o, ray_d);
	
	imageStore(dest_tex, pixel_coords, pixel);
}