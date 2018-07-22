precision mediump float;

varying vec2 fsTex;

uniform sampler2D mainTex;
uniform vec4 color;
uniform float objectGlow;

// 20Hz flickering. 0 = Miss, 1 = Inactive, 2 & 3 = Active alternating.
uniform int hitState;

void main()
{	
    float x = fsTex.x;
    float laserSize = 0.8; //0.0 to 1.0
    x -= 0.5;
    x /= laserSize;
    x += 0.5;
	vec4 mainColor = texture2D(mainTex, vec2(x,fsTex.y));
	gl_FragColor = mainColor * color;
	float brightness = (gl_FragColor.x + gl_FragColor.y + gl_FragColor.z) / 3.0;
	gl_FragColor.xyz = gl_FragColor.xyz * (0.5 + objectGlow);
}