#version 130

uniform vec2 u_resolution;

#define M_PI 3.1415926535897932384626433832795

float PHI = 1.61803398874989484820459 * 00000.1; // Golden Ratio   
float PI  = 3.14159265358979323846264 * 00000.1; // PI
float SQ2 = 1.41421356237309504880169 * 10000.0; // Square Root of Two

float gold_noise(in vec2 coordinate, in float seed)
{
    return fract(tan(distance(coordinate * (seed + PHI), vec2(PHI, PI))) * SQ2);
}

void main()
{
	const vec4 topColor = vec4(0.2, 0.2, 0.2, 1.0);
	const vec4 bottomColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	vec2 uv = gl_FragCoord.xy / u_resolution;
	// uv.y = 1.0 - uv.y;
	
	float topHeight = 0.6 - sin(uv.x * M_PI) * 0.1;
	float bottomHeight = 0.3 + sin(uv.x * M_PI) * 0.1;
	
	float topRatio = (1.0 - (max(0.0, uv.y - (1.0 - topHeight)) / topHeight)) * 0.6 + 0.4;
	float bottomRatio = min(1.0, uv.y / bottomHeight);
	
	float ratio = topRatio * bottomRatio;
	
	vec4 color = uv.y <= bottomHeight ? bottomColor : topColor;
	
	
	vec4 outColor = mix(color, gl_Color, ratio);
	
	// float hratio = min(1.0, (0.5 - abs(uv.x - 0.5)) / 0.5) * 0.4 + 0.6;
	// outColor *= mix(bottomColor, vec4(1.0), hratio);
	
	float noise = gold_noise(uv, SQ2);
	outColor.rgb += mix(vec3(-0.5 / 255.0), vec3(0.5 / 255.0), noise);
	
	// outColor.rgb = vec3(noise);
	
	gl_FragColor = outColor;
	gl_FragColor.r = pow(gl_FragColor.r, 2.4);
	gl_FragColor.g = pow(gl_FragColor.g, 2.4);
	gl_FragColor.b = pow(gl_FragColor.b, 2.4);
}