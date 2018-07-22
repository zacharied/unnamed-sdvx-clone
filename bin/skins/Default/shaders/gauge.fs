precision mediump float;

varying vec2 fsTex;

uniform float time;
uniform float rate;
uniform sampler2D mainTex;
uniform sampler2D maskTex;
uniform vec4 barColor;

void main()
{
	vec4 tex = texture2D(mainTex, fsTex);

    float mask = texture2D(maskTex, fsTex).x;
    mask = rate - mask;
    mask *= 100.0;
    mask = clamp(mask, 0.0, 1.0);
	gl_FragColor.rgb = tex.rgb * barColor.rgb;
    gl_FragColor.a = tex.a * mask;
}