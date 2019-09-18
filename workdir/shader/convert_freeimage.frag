#version 130

uniform sampler2D u_texture;
uniform sampler2D u_checkerPatternTexture;

uniform vec2 u_params[2];

#define M_PI 3.1415926535897932384626433832795

vec4 checkerboard(vec2 uv, vec2 texturePixelSize)
{
	vec2 checkerSize = textureSize(u_checkerPatternTexture, 0);
	return texture2D(u_checkerPatternTexture, (texturePixelSize / checkerSize) * (uv - vec2(0.5)));
}

vec4 downsample(sampler2D texture, vec2 uv, vec2 apparentSize, float distance)
{
	vec4 sum = vec4(0.0);
	
    vec2 subpixel = vec2(
    	1.0 / (apparentSize.x * distance),
    	1.0 / (apparentSize.y * distance)
    );
    
    int down = 2;
    for (int i = 0; i < down; i++)
    {
        for (int j = 0; j < down; j++)
        {
            vec2 pure_sample_loc = vec2(
            	uv.x + float(i) * subpixel.x,
            	uv.y + float(j) * subpixel.y
            );
            
            sum += texture2D(texture, pure_sample_loc);
        }
    }
    
    sum = sum / pow(float(down), 2);
    
    return vec4(sum.rgb, texture2D(texture, uv).a);
}




vec4 sharpen(sampler2D texture, vec2 uv, vec2 size, float apparentScale)
{
	const float stepSize = 1.f;
	float step_w = stepSize / size.x;
	float step_h = stepSize / size.y;
	
	vec2 offset[9];
	offset[0] = vec2(-step_w, -step_h);
	offset[1] = vec2( 0.0,    -step_h);
	offset[2] = vec2( step_w, -step_h);
	offset[3] = vec2(-step_w,  0.0);
	offset[4] = vec2( 0.0,     0.0);
	offset[5] = vec2( step_w,  0.0);
	offset[6] = vec2(-step_w,  step_h);
	offset[7] = vec2( 0.0,     step_h);
	offset[8] = vec2( step_w,  step_h);


	/* SHARPEN KERNEL 
	 0 -1  0
	-1  5 -1
	 0 -1  0
	*/
	
	/*
	kernel[0] =  0.0;
	kernel[1] = -1.0;
	kernel[2] =  0.0;
	kernel[3] = -1.0;
	kernel[4] =  5.0;
	kernel[5] = -1.0;
	kernel[6] =  0.0;
	kernel[7] = -1.0;
	kernel[8] =  0.0;*/
	
	float kernel[9];
	kernel[0] =  0.0;
	kernel[1] = -0.5;
	kernel[2] =  0.0;
	kernel[3] = -0.5;
	kernel[4] =  3.0;
	kernel[5] = -0.5;
	kernel[6] =  0.0;
	kernel[7] = -0.5;
	kernel[8] =  0.0;

	vec4 sum = vec4(0.0);
	int i;
	for (i = 0; i < 9; i++)
	{
		// vec4 color = texture2D(texture, uv + offset[i]);
		sum += kernel[i] * downsample(texture, uv + offset[i], size, max(1.0, 1.0 / apparentScale));
		// sum += kernel[i] * texture2D(texture, uv + offset[i]);
	}
	
	return sum;
}

vec4 contrast(vec4 color, float contrast, float brightness)
{
	// color.rgb /= color.a;
	
	// Apply contrast.
	color.rgb = ((color.rgb - 0.5f) * max(contrast, 0.0)) + 0.5f;

	// Apply brightness.
	color.rgb += brightness;
	
	// color.rgb *= color.a;
	
	return vec4(color.rgb, color.a);
}

void main()
{
	vec2 apparentSize = u_params[0].xy;
	float apparentScale = u_params[1].x;
	
	vec2 uv = gl_TexCoord[0].xy;
	uv = vec2(uv.x, 1.0 - uv.y);
	
	vec2 pixelCoordinate = uv * apparentSize;
	if (uv.x * apparentSize.x >= apparentSize.x - 1.5)
		discard;
	
	vec4 direct = texture2D(u_texture, uv);
	vec4 sharpened = sharpen(u_texture, uv, apparentSize, apparentScale);
	
	vec4 texColor = vec4(0.0, 0.0, 0.0, 1.0);
	if (apparentScale < 1.0)
	{
		float scale = (apparentScale - 0.5) / 0.5;
		scale = clamp(scale, 0.0, 1.0);
		texColor = mix(sharpened, direct, smoothstep(0.0, 1.0, scale));
	}
	else
	{
		float scale = (apparentScale - 1.0) / 0.3;
		scale = clamp(scale, 0.0, 1.0);
		texColor = mix(direct, sharpened, smoothstep(0.0, 1.0, scale));
	}
	
	// texColor = contrast(texColor, 1.7, 0.0);
	// texColor = contrast(texColor, 0.75, 0.1);
	
	vec4 checker = checkerboard(uv, apparentSize);
	gl_FragColor = mix(checker, vec4(texColor.rgb, 1.0), texColor.a) * gl_Color;
}