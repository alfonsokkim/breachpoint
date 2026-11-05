#version 330 core

in vec4 colour;
flat in float flash;
out vec4 fragColor;

void main() {
    vec2 p = gl_PointCoord * 2.0 - 1.0;
    float radius = length(p);
    float alpha = 1.0 - smoothstep(0.15, 1.0, radius);
    if (flash > 0.5) {
        float arms = min(abs(p.x), abs(p.y));
        alpha *= 1.0 - smoothstep(0.05, 0.32, arms);
    }
    if (alpha < 0.01) discard;
    fragColor = vec4(colour.rgb, colour.a * alpha);
}
