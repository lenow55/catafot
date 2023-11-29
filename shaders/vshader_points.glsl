attribute vec3 color;
in highp vec2 vertex;

uniform highp mat4 matrix;
uniform float depth;

varying vec3 map;


void main(void)
{
   gl_Position = matrix * vec4(vertex, depth, 1.0);
   map = color;
}
