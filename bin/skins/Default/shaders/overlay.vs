attribute vec2 inPos;
attribute vec2 inTex;

varying vec2 fsTex;

uniform mat4 proj;
uniform mat4 camera;
uniform mat4 world;

void main()
{
	fsTex = inTex;
	fsTex.y = -fsTex.y;
	gl_Position = proj * camera * world * vec4(inPos.xy, 0, 1);
}