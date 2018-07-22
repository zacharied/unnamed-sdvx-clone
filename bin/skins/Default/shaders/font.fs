precision mediump float;

varying vec2 fsTex;

uniform sampler2D mainTex;
uniform vec4 color;

void main()
{
	float alpha = texture2D(mainTex, fsTex).a;
	//gl_FragColor = vec4(color.xyz, alpha * color.a);
	gl_FragColor = vec4(255);
}