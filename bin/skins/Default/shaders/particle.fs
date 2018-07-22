precision mediump float;

varying vec4 fsColor;
varying vec2 fsTex;

uniform sampler2D mainTex;
		
void main()
{
	gl_FragColor = fsColor * texture2D(mainTex, fsTex);
}