attribute highp vec2 vertex;
uniform highp mat4 matrix;
uniform float depth;

void main(void)
{
   gl_Position = matrix * vec4(vertex, depth, 1.0);
}
