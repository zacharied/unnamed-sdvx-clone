precision mediump float;

varying vec2 fsTex;

uniform sampler2D mainTex;

void main()
{	
	vec4 mainColor = texture2D(mainTex, fsTex.xy);
	gl_FragColor = mainColor;
}