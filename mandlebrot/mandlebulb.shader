#shader vertex
#version 460

layout(location = 0) in vec3 position;

uniform vec3 cam_pos;
uniform vec2 cam_rot; // x = yaw, y = pitch

out vec3 origin, direction;

void main() {
	gl_Position = vec4(position.xy, 0.0f, 1.0f);
	coords = position.xy;
	origin = cam_pos;
	mat3 rot_mat = mat3(
		cos(cursor_rot.x) * cos(cursor_rot.y);
		sin(cursor_rot.y);
		sin(cursor_rot.x) * cos(cursor_rot.y);
	);
	direction = rot_mat * vec3(aspect * position.x, position.y);
};

#shader fragment
#version 460

out vec4 FragColor;
//xoff, yoff, z = aspect ratio, w = scale,
uniform dvec4 _bounds;

uniform int iterations;
uniform vec4 col1, col2, col3, col4;

in vec3 origin, direction;

void main() {
	
};