attribute vec4 vPosition;
attribute vec2 vTexCoord;

varying vec2 fTexCoord;
uniform mat4 uMvpMatrix;

void main()
{
	fTexCoord = vTexCoord;
	gl_Position = uMvpMatrix * vPosition;
}
