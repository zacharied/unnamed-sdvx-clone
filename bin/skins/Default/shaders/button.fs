precision mediump float;

varying vec2 fsTex;

uniform sampler2D mainTex;
uniform bool hasSample;

void main()
{	
	vec4 mainColor = texture2D(mainTex, fsTex.xy);
    if(hasSample)
    {
        float addition = abs(0.5 - fsTex.x) * - 1.;
        addition += 0.3;
        addition = max(addition,0.);
        addition *= 1.2;
        mainColor.xyz += addition;
    }
	gl_FragColor = mainColor;
}