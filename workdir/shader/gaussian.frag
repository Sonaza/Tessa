uniform sampler2D u_texture;
uniform vec2 u_textureSize;
uniform vec2 u_direction;

vec4 blur13(sampler2D tex, vec2 uv, vec2 resolution, vec2 direction)
{
	vec4 color = vec4(0.0);
	vec2 off1 = vec2(1.411764705882353) * direction;
	vec2 off2 = vec2(3.2941176470588234) * direction;
	vec2 off3 = vec2(5.176470588235294) * direction;
	color += texture2D(tex, uv) * 0.1964825501511404;
	color += texture2D(tex, uv + (off1 / resolution)) * 0.2969069646728344;
	color += texture2D(tex, uv - (off1 / resolution)) * 0.2969069646728344;
	color += texture2D(tex, uv + (off2 / resolution)) * 0.09447039785044732;
	color += texture2D(tex, uv - (off2 / resolution)) * 0.09447039785044732;
	color += texture2D(tex, uv + (off3 / resolution)) * 0.010381362401148057;
	color += texture2D(tex, uv - (off3 / resolution)) * 0.010381362401148057;
	return color;
}

void main()
{
	vec2 uv = gl_TexCoord[0].xy;
	gl_FragColor = blur13(u_texture, uv, u_textureSize, u_direction) * gl_Color;
	// gl_FragColor.r = pow(gl_FragColor.r, 2.4);
	// gl_FragColor.g = pow(gl_FragColor.g, 2.4);
	// gl_FragColor.b = pow(gl_FragColor.b, 2.4);
}