attribute vec4 vPosition;
varying vec3 fTexCoord;
uniform mat4 uMvpMatrix;

void main()
{
	fTexCoord = vPosition.xyz;
	fTexCoord.y = -fTexCoord.y;
	
    gl_Position = uMvpMatrix * vPosition;
}
