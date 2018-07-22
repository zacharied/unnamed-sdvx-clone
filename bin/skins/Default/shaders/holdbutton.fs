precision mediump float;

varying vec2 fsTex;

uniform sampler2D mainTex;
uniform float objectGlow;

// 20Hz flickering. 0 = Miss, 1 = Inactive, 2 & 3 = Active alternating.
uniform int hitState;

void main()
{	
	vec4 mainColor = texture2D(mainTex, fsTex.xy);
	gl_FragColor = mainColor;
	gl_FragColor.xyz = gl_FragColor.xyz * (1.0 + objectGlow * 0.3);
	gl_FragColor.a = min(1.0, gl_FragColor.a + gl_FragColor.a * objectGlow * 0.9);
}