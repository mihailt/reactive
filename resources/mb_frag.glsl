#version 120
uniform vec2 size;
uniform int num;
uniform vec2 positions[100];
uniform float radius[100];
void main(void)
{
  // Get coordinates
  vec 2 texCoord = gl_TexCoord[0].st;
  vec4 color = vec4(1.0,1.0,1.0, 0.0);
  float a = 0.0;
  int i;
  for(i = 0; i<num; i++) {
        color.a += (radius[i] / sqrt( ((texCoord.x*size.x)-
           positions[i].x)*((texCoord.x*size.x)-positions[i].x) +
           ((texCoord.y*size.y)-
           positions[i].y)*((texCoord.y*size.y)-positions[i].y) )
           );
  }
    // Set color
    gl_FragColor = color;
}