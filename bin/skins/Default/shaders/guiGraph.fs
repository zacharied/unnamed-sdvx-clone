precision mediump float;

varying vec2 fsTex;

uniform sampler2D graphTex;
uniform ivec2 viewport;
uniform vec4 color;
uniform vec4 upperColor;
uniform vec4 lowerColor;
uniform float colorBorder;


void main()
{
	gl_FragColor = vec4(1.0);
    float xStep = 1.0 / float(viewport.x);
    vec3 col = vec3(0.0);
    
    vec2 current = vec2(fsTex.x, texture2D(graphTex, vec2(fsTex.x, 0.5)).x);
    vec2 next = vec2(fsTex.x + xStep, texture2D(graphTex, vec2(clamp(fsTex.x + xStep, 0.0, 1.0) , 0.5)).x);
    vec2 avg = (current + next) / 2.0;
    float dist = abs(distance(vec2(fsTex.x,fsTex.y * -1.0 + 1.0),avg));
    
    if (avg.y >= colorBorder)
        col = upperColor.xyz;
    else
        col = lowerColor.xyz;
    

    gl_FragColor.xyz = vec3(col);
    gl_FragColor.a = smoothstep(abs(next.y - current.y) + 0.010, abs(next.y - current.y),dist);
}