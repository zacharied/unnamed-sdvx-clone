precision mediump float;

varying vec2 fsTex;

uniform sampler2D mainTex;
uniform vec4 color;

void main()
{	
	vec4 mainColor = texture2D(mainTex, fsTex.xy);
	gl_FragColor = mainColor * color;
}