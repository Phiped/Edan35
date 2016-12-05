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

#define NUM_BOUNCES 8

#define BIAS_FACTOR 0.0001f

const vec3 starting_origin = vec3(0.0, -3.0, 0.0);

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
	
	//determine shadow
	
	float strength = 1.0;
	float dist_to_sun = length(sun_location - modified);
	if (length(closest.impact_point - modified) < dist_to_sun){
		strength = 0;
	}
	// return vec4(info.color / pow(dist_to_sun* 0.2f, 2), 0.0);
	return vec4(info.color * (0.3 + strength) / pow(dist_to_sun* 0.15f, 2), 0.0);
};

vec4 find_color(vec3 rayStart,vec3 rayDir) {
	vec4 finalColor=vec4(0.0);
	float frac=1.0; // fraction of my color to add to finalColor
	for (int raybounce=0;raybounce<NUM_BOUNCES && frac > 0.05;raybounce++) {
		hit_info i = closest_hit(rayStart,rayStart + rayDir); // geometric search
		vec4 local = light_intersection(i); // diffuse + specular
		
		// ATM we can only refract OR reflect on a surface, not both, not sure if it's possible to do both since recursion is not allowed
		if (i.refractivity > 0){
			// T = (b * (N.I) -/+ sqrt(1 - b^2*(1-(N.I)^2))*N - b*I
			finalColor += local*(1.0-i.refractivity)*frac;
			frac *= i.refractivity; // <- scale down all subsequent rays
			rayDir = refract(rayDir, i.impact_normal, 1.1);
		} else{
			finalColor += local*(1.0-i.reflectivity)*frac;
			frac *= i.reflectivity; // <- scale down all subsequent rays
			rayDir=reflect(rayDir,i.impact_normal);
		}
		rayStart=i.impact_point;


	}
	return finalColor;
};


	// float rindex = prim->GetMaterial()->GetRefrIndex();

// vec3 RefractSlow(vec3 N, vec3 I)
// {
    // float ndoti, two_ndoti, ndoti2, a,b,b2,D2;
    // vec3 T;
    // ndoti = N.x*I.x + N.y*I.y + N.z*I.z;     // 3 mul, 2 add
    // ndoti2 = ndoti*ndoti;                    // 1 mul
    // if (ndoti>=0.0) { b=r; b2=r2;} else {b=invr;b2=invr2;}
    // D2 = 1.0f - b2*(1.0f - ndoti2);

    // if (D2>=0.0f) {
        // if (ndoti >= 0.0f)
            // a = b * ndoti - sqrtf(D2); // 2 mul, 3 add, 1 sqrt
        // else
            // a = b * ndoti + sqrtf(D2);
        // T->x = a*N.x - b*I.x;     // 6 mul, 3 add
        // T->y = a*N.y - b*I.y;     // ----totals---------
        // T->z = a*N.z - b*I.z;     // 12 mul, 8 add, 1 sqrt!
    // } else {
        // total internal reflection
        // this usually doesn't happen, so I don't count it.
        // two_ndoti = ndoti + ndoti;         // +1 add
        // T->x = two_ndoti * N.x - I.x;      // +3 adds, +3 muls
        // T->y = two_ndoti * N.y - I.y;
        // T->z = two_ndoti * N.z - I.z;
    // }
    // return T;
// }




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