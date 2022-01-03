#shader vertex
#version 460

layout(location = 0) in vec3 position;

uniform vec3 cam_pos;
uniform vec2 cam_rot; // x = yaw, y = pitch
uniform float aspect;

out vec3 origin, direction;

void main() {
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
	direction = normalize(yaw * pitch * vec3(aspect * position.x, position.y, 1.0f));
};



#shader fragment
#version 460

out vec4 FragColor;

const float MAXDIST = 30.0f;

uniform vec3 col1, col2;
uniform int iterations = 300;
uniform int MaximumRaySteps = 400;
uniform float minRadius2 = 1;
uniform float fixedRadius2 = 4;
uniform float foldingLimit = 1;
uniform float Scale = -1.5;
uniform float Power = 8;
uniform bool shadows;

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

struct marchReturn {
	bool hit;
	int steps;
	float steps_normalized;
	float closest_distance;
	float total_dist;
	vec3 endpoint;
};

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

void main() {
	marchReturn marched = trace(origin, direction, 0.001f);
	
	vec3 color = col1 * (1 - marched.steps_normalized);
	
	float dnorm = (marched.closest_distance * 10);
	float haloAmount = ((marched.closest_distance < 1) ? (1 - marched.closest_distance) : 0);
	vec3 halo = mix(vec3(0, 0, 0), col2, haloAmount);
	
	vec3 finalcolor = marched.hit ? mix(color, halo, 0.1f * marched.total_dist) : halo;

	if(shadows) finalcolor *= trace(marched.endpoint + sun_pos * 0.01f, sun_pos, 0.001f).hit ? 0.4f : 1.0f;
	
	FragColor = vec4(finalcolor, 1.0f);
};