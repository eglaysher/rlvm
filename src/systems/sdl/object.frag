uniform sampler2D image;
uniform vec4 colour;
uniform vec3 tint;
uniform float light;
uniform float alpha;

void tinter(in float pixel_val, in float tint_val, out float mixed) {
  if (tint_val > 0.0) {
    mixed = pixel_val + tint_val - (pixel_val * tint_val);
  } else if (tint_val < 0.0) {
    mixed = pixel_val * abs(tint_val);
  } else {
    mixed = pixel_val;
  }
}

void main() {
  vec4 pixel = texture2D(image, gl_TexCoord[0].st);
  
  // The colour is blended directly with the incoming pixel value.
  vec3 coloured = mix(pixel.rgb, colour.rgb, colour.a);
  pixel = vec4(coloured.r, coloured.g, coloured.b, pixel.a);

  float out_r, out_g, out_b;
  tinter(pixel.r, tint.r, out_r);
  tinter(pixel.g, tint.g, out_g);
  tinter(pixel.b, tint.b, out_b);
  pixel = vec4(out_r, out_g, out_b, pixel.a);

  tinter(pixel.r, light, out_r);
  tinter(pixel.g, light, out_g);
  tinter(pixel.b, light, out_b);
  pixel = vec4(out_r, out_g, out_b, pixel.a);

  // We're responsible for doing the main alpha blending, too.
  pixel.a = pixel.a * alpha;
  gl_FragColor = pixel;
}
