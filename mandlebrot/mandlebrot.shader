#shader vertex
#version 460

layout(location = 0) in vec3 position;

out vec2 coords;

void main() {
	gl_Position = vec4(position.xy, 0.0f, 1.0f);
	coords = position.xy;
};

#shader fragment
#version 460

out vec4 FragColor;
//xoff, yoff, z = aspect ratio, w = scale,
uniform dvec4 _bounds;

uniform int iterations;
uniform int type;
uniform vec2 juliaCoords;
uniform vec4 col1, col2, col3, col4;

in vec2 coords;

dvec2 step_mandlebrot(dvec2 v, double x, double y) {
	return dvec2((v.x * v.x - v.y * v.y) + x, (2 * v.x * v.y) + y);
}

dvec2 step_burningship(dvec2 v, double x, double y) {
	return dvec2((v.x * v.x - v.y * v.y) + x, abs(2 * v.x * v.y) - y);
}

dvec2 step_tricorn(dvec2 v, double x, double y) {
	return dvec2((v.x * v.x - v.y * v.y) + x, (-2 * v.x * v.y) + y);
}

int iterator(double x, double y, int t) {
	dvec2 v = dvec2(x, y);
	int iteration = 0;
	double xtemp = 0;

	switch (t) {
	default:
		while ((v.x * v.x + v.y * v.y) < 4 && iteration < iterations)
		{
			v = step_mandlebrot(v, x, y);
			iteration++;
		}
		break;
	case 1:
		while ((v.x * v.x + v.y * v.y) < 4 && iteration < iterations)
		{
			v = step_mandlebrot(v, juliaCoords.x, juliaCoords.y);
			iteration++;
		}
		break;
	case 2:
		while ((v.x * v.x + v.y * v.y) < 4 && iteration < iterations)
		{
			v = step_burningship(v, x, y);
			iteration++;
		}
		break;
	case 3:
		while ((v.x * v.x + v.y * v.y) < 4 && iteration < iterations)
		{
			v = step_burningship(v, juliaCoords.x, juliaCoords.y);
			iteration++;
		}
		break;
	case 4:
		while ((v.x * v.x + v.y * v.y) < 4 && iteration < iterations)
		{
			v = step_tricorn(v, x, y);
			iteration++;
		}
		break;
	}

	if (iteration == iterations)
		return -1;
	else
		return iteration;
}

void main() {
	dvec2 pos = dvec2(
		(coords.x * _bounds.z * _bounds.w) + _bounds.x,
		(coords.y * _bounds.w) + _bounds.y
	);
	int it = iterator(pos.x, pos.y, type);
	float ivalue = it / float(iterations);
	if (it == -1) { FragColor = col4; return; }
	vec4 mix1 = mix(col1, col2, ivalue);
	vec4 mix2 = mix(col2, col3, ivalue);
	FragColor = mix(mix1, mix2, ivalue);
};