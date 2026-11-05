#version 330 core
in vec2 uv;
out vec4 fragColor;
uniform sampler2D uScene;
void main() {
    vec4 scene = texture(uScene, uv);
    vec3 sky = mix(vec3(0.018, 0.025, 0.055), vec3(0.18, 0.095, 0.08),
                   pow(1.0 - uv.y, 3.0));
    vec3 colour = scene.rgb + sky * (1.0 - scene.a);
    colour = pow(max(colour / (colour + 1.0), vec3(0.0)), vec3(1.0 / 2.2));
    fragColor = vec4(colour, 1.0);
}
