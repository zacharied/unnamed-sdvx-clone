precision mediump float;

varying vec2 fsTex;

uniform sampler2D mainTex;
uniform vec4 color;

void main()
{
	gl_FragColor = texture2D(mainTex, fsTex) * color;
}