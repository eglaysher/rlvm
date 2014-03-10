
uniform sampler2D currentValues, mask;

void main()
{
  vec4 bgColor = texture2D(currentValues, gl_TexCoord[0].st);
  vec4 maskColor = texture2D(mask, gl_TexCoord[1].st);
  gl_FragColor = bgColor - maskColor.a + gl_Color * maskColor.a;
}
