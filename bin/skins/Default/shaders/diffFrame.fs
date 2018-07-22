precision mediump float;

varying vec2 fsTex;

uniform sampler2D frame;
uniform sampler2D jacket;
uniform float time;
uniform float selected;

void main()
{
	vec4 jacket = texture2D(jacket, fsTex);
	vec4 frame = texture2D(frame, fsTex);
	float a = max(max(frame.x, frame.y), frame.z);
	float pulse = cos(time*2.0) * 0.25 + 0.25;
	a = a * 0.5 + pulse * selected;
	gl_FragColor = jacket * (0.3 + 0.7 * selected) * (1.0 - a) + frame * a;
	gl_FragColor.a = frame.a;
}