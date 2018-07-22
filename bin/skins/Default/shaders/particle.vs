attribute in vec3 inPos;
attribute in vec4 inColor;
attribute in vec4 inParams;

varying vec4 fsColor;
varying vec4 fsParams;

void main()
{
	fsColor = inColor;
	fsParams = inParams;
	gl_Position = vec4(inPos,1);
}