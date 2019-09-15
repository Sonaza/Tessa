#version 130

uniform sampler2D u_texture;

uniform vec2 u_resolution;
uniform vec2 u_position;
uniform vec2 u_size;

void main()
{
	vec2 base = vec2(gl_FragCoord.x, u_resolution.y - gl_FragCoord.y) - u_position;
	if (base.x > u_size.x || base.y > u_size.y)
		discard;
	
	vec4 col = texture2D(u_texture, gl_TexCoord[0].xy);
	gl_FragColor = col * gl_Color;
}