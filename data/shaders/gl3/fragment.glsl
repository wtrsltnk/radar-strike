#version 150

uniform sampler2D u_texture;

in vec2 f_texcoord;

out vec4 color;

void main()
{
   color = texture(u_texture, f_texcoord);
}
