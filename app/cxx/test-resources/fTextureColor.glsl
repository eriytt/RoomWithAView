#version 120

uniform sampler2D tex;
uniform vec4 pColor;

void main()
{
        vec4 base_color = vec4(1.0, 0.0, 0.0, 1.0);
        vec4 tcolor = texture2D(tex,gl_TexCoord[0].st);
	gl_FragColor = pColor * tcolor.a;
}
