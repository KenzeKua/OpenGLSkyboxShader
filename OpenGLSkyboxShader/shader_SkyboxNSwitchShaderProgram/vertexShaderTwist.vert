attribute vec4 vPosition;
attribute vec2 vTexCoord;

varying vec2 fTexCoord;
uniform mat4 uMvpMatrix;

uniform float Factor1;

float twistStrength = 0.9;

mat4 rotationMatrix(vec3 axis, float angle)
{
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    
    return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
                0.0,                                0.0,                                0.0,                                1.0);
}

void main()
{
	fTexCoord = vTexCoord;
	gl_Position = uMvpMatrix * rotationMatrix(vec3(0.0, 1.0, 0.0), sin(Factor1) * vPosition.y * twistStrength) * vPosition;
}
