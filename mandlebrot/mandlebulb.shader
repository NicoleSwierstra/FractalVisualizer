#shader vertex
#version 460

layout(location = 0) in vec3 position;

uniform vec3 cam_pos;
uniform vec2 cam_rot; // x = yaw, y = pitch
uniform float aspect;

out vec3 origin, direction;

void main() {
	//positions the raycasts that are part of the line
	float fov = 0.75f;
	gl_Position = vec4(position.xy, 0.0f, 1.0f);
	origin = cam_pos;
	mat3 yaw = mat3(
		vec3(cos(cam_rot.x), 0, -sin(cam_rot.x)),
		vec3(0, 1, 0),
		vec3(sin(cam_rot.x), 0, cos(cam_rot.x))
	);
	mat3 pitch = mat3(
		vec3(1, 0, 0),
		vec3(0, cos(cam_rot.y), sin(cam_rot.y)),
		vec3(0, -sin(cam_rot.y), cos(cam_rot.y))
	);
	direction = normalize(yaw * pitch * vec3(fov * aspect * position.x, fov * position.y, 1.0f));
};

#shader fragment
#version 460

out vec4 FragColor;

const float MAXDIST = 30.0f;
//ground and atmosphere colors
uniform vec3 col1, col2;
uniform int iterations = 300;
uniform int MaximumRaySteps = 400;
uniform float minRadius2 = 1;
uniform float fixedRadius2 = 4;
uniform float foldingLimit = 1;
uniform float Scale = -1.5;
uniform float Power = 8;
uniform float water_scat, water_abs;
uniform bool shadows, bwater;

uniform vec3 sun_pos;

uniform int type;

in vec3 origin, direction;

void sphereFold(inout vec3 z, inout float dz) {
	float r2 = dot(z, z);
	if (r2 < minRadius2) {
		// linear inner scaling
		float temp = (fixedRadius2 / minRadius2);
		z *= temp;
		dz *= temp;
	}
	else if (r2 < fixedRadius2) {
		// this is the actual sphere inversion
		float temp = (fixedRadius2 / r2);
		z *= temp;
		dz *= temp;
	}
}

void boxFold(inout vec3 z, inout float dz) {
	z = clamp(z, -foldingLimit, foldingLimit) * 2.0 - z;
}

float DistanceEstimatorCube(vec3 pos) {
	vec3 z = pos;
	vec3 offset = z;
	float dr = 1.0;
	for (int n = 0; n < iterations; n++) {
		boxFold(z, dr);
		sphereFold(z, dr);

		z = Scale * z + offset;
		dr = dr * abs(Scale) + 1.0;
	}
	float r = length(z);
	return r / abs(dr);
}

float DistanceEstimatorBulb(vec3 pos) {
	vec3 z = pos;
	float dr = 1.0;
	float r = 0.0;

	for (int i = 0; i < iterations; i++) {
		r = length(z);
		if (r > 2) break;

		float theta = acos(z.z / r);
		float phi = atan(z.y, z.x);
		dr = pow(r, Power - 1.0) * Power * dr + 1.0;

		float zr = pow(r, Power);
		theta = theta * Power;
		phi = phi * Power;

		z = zr * vec3(sin(theta) * cos(phi), sin(phi) * sin(theta), cos(theta));
		z += pos;
	}
	return 0.5 * log(r) * r / dr;
}

//the struct for the raymarching function
struct marchReturn {
	bool hit; //if hit
	int steps; //# of ray steps to hit
	float steps_normalized; //steps / max steps
	float closest_distance; //closest surface distance
	float total_dist; //distance traveled
	vec3 endpoint; //endpoint
};

//raymarching with seperate distance functions
marchReturn trace(vec3 from, vec3 direction, float MinimumDistance) {
	float totalDistance = 0.0;
	int steps = 0;
	float closestDist = 100;
	vec3 p;
	
	if (type == 0) {
		for (; steps < MaximumRaySteps; steps++) {
			p = from + totalDistance * direction;

			float distance = DistanceEstimatorBulb(p);
			closestDist = distance < closestDist ? distance : closestDist;
			totalDistance += distance;

			if (totalDistance > MAXDIST)
				return marchReturn(false, steps, 1, closestDist, totalDistance, p);
			if (distance < MinimumDistance)
				return marchReturn(true, steps, float(steps) / MaximumRaySteps, 0.0, totalDistance, p);
		}
	}
	else {
		for (; steps < MaximumRaySteps; steps++) {
			p = from + totalDistance * direction;

			float distance = DistanceEstimatorCube(p);
			closestDist = distance < closestDist ? distance : closestDist;
			totalDistance += distance;

			if (totalDistance > MAXDIST)
				return marchReturn(false, steps, 1, closestDist, totalDistance, p);
			if (distance < MinimumDistance)
				return marchReturn(true, steps, float(steps) / MaximumRaySteps, 0.0, totalDistance, p);
		}
	}
	
	return marchReturn(false, steps, 1, closestDist, totalDistance, p);
}

//quadratic formula that outputs both solutions in the x value
bool solveQuadratic(float a, float b, float c, out vec2 x)
{
	float x0, x1;
	float discr = b * b - 4 * a * c;
	if (discr < 0) return false;
	else if (discr == 0) x0 = x1 = -0.5 * b / a;
	else {
		float q = (b > 0) ?
			-0.5 * (b + sqrt(discr)) :
			-0.5 * (b - sqrt(discr));
		x0 = q / a;
		x1 = c / q;
	}

	if (x0 < 0 && x1 < 0) return false;

	x = (x0 > x1) ? vec2(x1, x0) : vec2(x0, x1);
	return true;
}

//returns dist_to, through + to
vec2 sphereRayDist(vec3 s_origin, float s_rad, vec3 r_origin, vec3 r_dir) {
	vec3 L = r_origin - s_origin;
	float a = dot(r_dir, r_dir);
	float b = 2 * dot(r_dir, L);
	float c = dot(L, L) - s_rad;

	vec2 x = vec2(0,0);
	if (!solveQuadratic(a, b, c, x)) return vec2(0, 0);
	return vec2(x.x, x.y);
}

//reflection for water. alpha channel for blending
vec4 waterReflection(vec3 dir, vec3 s_origin, vec3 p) {
	vec3 reflectdir = dir - (2 * dot(dir, normalize(p - s_origin)) * p);
	bool hit = trace(p, reflectdir, 0.01f).hit;
	float angle = 1 - abs(dot(dir, p));
	vec3 col = mix(vec3(0,0,1), vec3(1,1,1), angle);

	float amount = pow(angle, 2);
	return vec4(col, amount);
}

//subtracts the light that is absorbed by water from the base color
void water(inout vec3 baseCol, float screendepth, vec3 start, vec3 dir) {
	vec3 abco = vec3(0.45, 0.06, 0.01) * water_abs; //absorbtion coefficient

	vec2 distances = sphereRayDist(vec3(0, 0, 0), 1, start, dir);

	float waterfloordist = min(distances.y, screendepth);
	vec3 hitpoint = start + (waterfloordist * dir);
	vec3 surfpoint = start + (distances.x * dir);

	//distance through the water
	float wdist = (distances.x > screendepth) ? 0 :
		waterfloordist - distances.x;

	if (wdist <= 0) return;

	vec3 falloff = vec3(
		exp(-wdist * abco.r),
		exp(-wdist * abco.g),
		exp(-wdist * abco.b)
	);

	baseCol = baseCol * falloff;

	vec3 scattercol = vec3(0, 0.2, 0.7);
	if (shadows) {
		vec2 a = sphereRayDist(vec3(0, 0, 0), 1, hitpoint, sun_pos);
		bool shad = trace(hitpoint * 1.1f, sun_pos, 0.001f).hit; // scales outwards spherically
		scattercol = scattercol * exp(-a.y) * (shad ? 0.8 : 1);
	}
	baseCol = mix(scattercol, baseCol, exp(-water_scat * wdist));
	
	vec4 ref = waterReflection(dir, vec3(0,0,0), surfpoint);
	if(distances.x > 0) baseCol = mix(baseCol, ref.rgb, ref.a * 0.5f);
}

void main() {
	marchReturn marched = trace(origin, direction, 0.001f);
	
	vec3 color = col1 * (1 - marched.steps_normalized);
	
	float dnorm = (marched.closest_distance * 10);
	float haloAmount = ((marched.closest_distance < 1) ? (1 - marched.closest_distance) : 0);
	vec3 halo = mix(vec3(0, 0, 0), col2, haloAmount);
	
	vec3 finalcolor = color;
	
	if (shadows) { 
		bool inshadow = trace(marched.endpoint + sun_pos * 0.01f, sun_pos, 0.001f).hit;
		finalcolor *= inshadow ? 0.4f : 1.0f; 
	}

	finalcolor = marched.hit ? mix(finalcolor, halo, 0.1f * marched.total_dist) : halo;

	if (type == 0 && bwater) 
		water(finalcolor, marched.total_dist, origin, direction);
	
	FragColor = vec4(finalcolor, 1.0f);
};