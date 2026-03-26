#version 330 core
out vec4 FragColor;

in vec3 fragPos;

uniform vec3 cameraPos;

float rowGrid(vec2 coord, float spacing) {
  float derivative = fwidth(coord.y);
  float gridLine = abs(fract(coord.y / spacing - 0.5) - 0.5) / (derivative / spacing);
  return 1.0 - min(gridLine, 1.0);
}

void main() {
  float spacing = 0.5;
  float g1 = rowGrid(fragPos.xz, spacing);
  float g2 = rowGrid(fragPos.xz, spacing * 10.0);

  vec3 color = vec3(0.5);
  float alpha = mix(g1, 1.0, g2) * 0.5;

  // Fade out as we get further from camera
  float dist = distance(fragPos.xz, cameraPos.xz);
  alpha *= exp(-dist * 0.05);

  if (alpha < 0.01) discard;

  FragColor = vec4(color, alpha);
}
