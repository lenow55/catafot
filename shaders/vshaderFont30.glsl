uniform mat4 mvp_matrix;

attribute vec4 v_texcoord;
varying vec2 TexCoords;

void main()
{
    gl_Position = mvp_matrix * vec4(v_texcoord.xy, 1.0, 1.0);
    TexCoords = v_texcoord.zw;
}
