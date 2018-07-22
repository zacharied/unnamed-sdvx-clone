attribute vec2 inPos;
attribute vec2 inTex;

varying vec2 fsTex;

uniform mat4 proj;
uniform mat4 world;

void main()
{
	fsTex = inTex;
	gl_Position = proj * world * vec4(inPos.xy, 0, 1);
}