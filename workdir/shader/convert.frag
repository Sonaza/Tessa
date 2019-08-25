#version 130

uniform sampler2D u_texture;
uniform sampler2D u_checkerPatternTexture;

uniform vec2 u_textureApparentSize;

uniform bool u_useAlphaChecker;

#define M_PI 3.1415926535897932384626433832795

vec4 checkerboard_dyn(vec4 col1, vec4 col2, vec2 uv, vec2 texturePixelSize, float squareSize)
{
	vec2 puv = texturePixelSize * uv;
	float bit = cos(((puv.x + squareSize / 2.f + texturePixelSize.x / 2.f) / squareSize) * M_PI) 
	          * sin((puv.y / squareSize + texturePixelSize.x / 2.f) * M_PI);
	return bit >= 0.f ? col1 : col2;
}

vec4 checkerboard(vec2 uv, vec2 texturePixelSize)
{
	vec2 checkerSize = textureSize(u_checkerPatternTexture, 0);
	return texture2D(u_checkerPatternTexture, (texturePixelSize / checkerSize) * (uv - vec2(0.5)));
}

void main()
{
	vec2 uv = gl_TexCoord[0].xy;
	vec4 texColor = texture2D(u_texture, vec2(uv.x, 1.0 - uv.y)).bgra;
	
	if (u_useAlphaChecker)
	{
		vec4 checker = checkerboard(uv, u_textureApparentSize);
			
		// vec4 checker = checkerboard_dyn(
		// 	vec4(0.7, 0.7, 0.7, 1.0),
		// 	vec4(1.0, 1.0, 1.0, 1.0),
		// 	uv,
		// 	u_textureApparentSize,
		// 	// textureSize(u_texture, 0),
		// 	8.f
		// );
		
		gl_FragColor = mix(checker, vec4(texColor.rgb, 1.0), texColor.a) * gl_Color;
	}
	else
	{
		gl_FragColor = texColor * gl_Color;
	}
}