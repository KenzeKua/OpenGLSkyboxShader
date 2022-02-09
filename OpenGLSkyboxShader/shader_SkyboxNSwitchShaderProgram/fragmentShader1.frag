precision mediump float;
varying vec2 fTexCoord;

uniform sampler2D sampler2d;
uniform sampler2D bloomTexture;

uniform float highPassLimit;

uniform int uHighPass;
uniform int uBlurDirection;
uniform int uGray;

uniform float uTextureW;
uniform float uTextureH;

float gaussianFunction(float x)
{
	float variance = 0.25; // x should be 0-1.0 with this variance

	float alpha = -(x*x / (2.0*variance));
	return exp(alpha);
}
float gaussianFunction2D(float x, float y)
{
	float variance = 0.25; // x and y should be 0 - 1.0 with this variance

	float alpha = -( (x*x + y*y) / (2.0*variance));
	return exp(alpha);
}

void HighPassFilter()
{
	vec4 texColour = texture2D(sampler2d, fTexCoord);
	float averageColour = (texColour.x + texColour.y + texColour.z) / 3.0;
	
	if (uHighPass == 0 && averageColour >= highPassLimit) // High pass
	{
		gl_FragColor = texture2D(sampler2d, fTexCoord);
	}
	else if (uHighPass == -1) // Do nothing
	{
		return;
	}
	else
	{
		gl_FragColor = vec4(0.0);
	}
}

void Blur()
{
	float radiusSize = 20.0;
	
	float textureW = uTextureW;
	float textureH = uTextureH;

	vec4 accumulatedColor;
	float totalWeight = 0.0;

	// Horizontal blur
	if(uBlurDirection == 0)
	{
		float x;
		for(x = -radiusSize; x <= radiusSize; x += 1.0)
		{
			float u = fTexCoord.x + x / textureW;
			if(u >= 0.0 && u <= 1.0)
			{
				float weight = gaussianFunction(x / radiusSize);
				accumulatedColor += texture2D(sampler2d, vec2(u, fTexCoord.y)) * weight;
				totalWeight += weight;
			}
		}
		gl_FragColor = accumulatedColor / totalWeight;
	}
	// Vertical blur
	else if (uBlurDirection == 1)
	{
		float y;
		for(y = -radiusSize; y <= radiusSize; y += 1.0)
		{
			float v = fTexCoord.y + y / textureH;
			if(v >= 0.0 && v <= 1.0)
			{
				float weight = gaussianFunction(y / radiusSize);
				accumulatedColor += texture2D(sampler2d, vec2(fTexCoord.x, v)) * weight;
				totalWeight += weight;
			}
		}
		gl_FragColor = accumulatedColor / totalWeight;
	}
	else if (uBlurDirection == 2)
	{
		vec4 texColour = texture2D(sampler2d, fTexCoord);
		vec4 bloomColour = texture2D(bloomTexture, fTexCoord);
		gl_FragColor = texColour + bloomColour;
	}
	else if (uBlurDirection == -1) // No blur and escape
	{
		return;
	}
	else 
	{
		gl_FragColor = texture2D(sampler2d, fTexCoord);
	}
}

void Gray()
{
	if (uGray == 0) // Grayscale
	{
		
		vec4 texColour = texture2D(sampler2d, fTexCoord);
		float average = (texColour.x + texColour.y + texColour.z) / 3.0;
		
		vec4 finalColor = vec4(average, average, average, 1.0);

		gl_FragColor = finalColor;
	}
	else // Do nothing
	{
		return;
	}
}

void main()
{
	HighPassFilter();
	Blur();
	Gray();
}
