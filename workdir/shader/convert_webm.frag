#version 130

uniform sampler2D u_texture;

vec3 yuvtorgb(vec3 yuv)
{
	float y = 1.1643 * (yuv.r - 0.0625);
	float u = yuv.g - 0.5;
	float v = yuv.b - 0.5;

	float r = y + 1.5958 * v;
	float g = y - 0.39173 * u - 0.81290 * v;
	float b = y + 2.017 * u;
	
	return vec3(r, g, b);
}

void main()
{
	vec2 uv = gl_TexCoord[0].xy;
	vec4 texColor = texture2D(u_texture, uv);
	gl_FragColor = mix(
		vec4(1.0, 0.0, 1.0, 1.0),
		vec4(yuvtorgb(texColor.rgb), 1.0),
		texColor.a) * gl_Color;
}