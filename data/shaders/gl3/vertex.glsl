#version 150

in vec3 vertex;
in vec3 normal;
in vec2 texcoord;

uniform mat4 u_projection;
uniform mat4 u_view;
uniform mat4 u_model;

out vec2 f_texcoord;

void main()
{
    gl_Position = u_projection * u_view * u_model * vec4(vertex.xyz, 1.0);
    f_texcoord = texcoord;
}
