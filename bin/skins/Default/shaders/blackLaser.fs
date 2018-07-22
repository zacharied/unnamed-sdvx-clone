precision mediump float;

varying vec2 fsTex;

uniform sampler2D mainTex;

void main()
{	
    float x = fsTex.x;
    float laserSize = 0.8; //0.0 to 1.0
    x -= 0.5;
    x /= laserSize;
    x += 0.5;
	vec4 mainColor = texture2D(mainTex, vec2(x,fsTex.y));
    gl_FragColor = vec4(0.0, 0.0, 0.0, mainColor.a);
}