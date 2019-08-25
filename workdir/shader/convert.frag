#version 130

uniform sampler2D u_texture;

uniform bool u_useAlphaChecker;

#define M_PI 3.1415926535897932384626433832795

vec4 checkerboard(vec4 col1, vec4 col2, vec2 uv, vec2 texturePixelSize, float squareSize)
{
	vec2 puv = texturePixelSize * uv;
	float bit = cos((puv.x / squareSize) * M_PI) * sin((puv.y / squareSize) * M_PI);
	return bit >= 0.f ? col1 : col2;
}

void main()
{
	vec2 uv = gl_TexCoord[0].xy;
	vec4 texColor = texture2D(u_texture, vec2(uv.x, 1.0 - uv.y)).bgra;
	
	if (u_useAlphaChecker)
	{
		vec4 checker = checkerboard(
			vec4(0.7, 0.7, 0.7, 1.0),
			vec4(1.0, 1.0, 1.0, 1.0),
			uv,
			textureSize(u_texture, 0),
			8.f
		);
		
		gl_FragColor = mix(checker, vec4(texColor.rgb, 1.0), texColor.a);
	}
	else
	{
		gl_FragColor = texColor;
	}
}