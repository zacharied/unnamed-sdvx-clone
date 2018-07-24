precision mediump float;

varying vec2 fsTex;

uniform sampler2D mainTex;
uniform ivec2 mapSize; //spritemap size
uniform vec4 color;

void main()
{
	float alpha = texture2D(mainTex, fsTex / vec2(mapSize)).a;
    gl_FragColor = vec4(color.xyz, alpha * color.a);
}