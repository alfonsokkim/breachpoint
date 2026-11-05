#version 330 core
in vec2 uv;
out vec4 fragColor;
uniform sampler2D uScene;
uniform bool uNightVision;
uniform float uTime;
void main() {
    vec4 scene = texture(uScene, uv);
    vec3 sky = mix(vec3(0.018, 0.025, 0.055), vec3(0.18, 0.095, 0.08),
                   pow(1.0 - uv.y, 3.0));
    vec3 colour = scene.rgb + sky * (1.0 - scene.a);
    colour = pow(max(colour / (colour + 1.0), vec3(0.0)), vec3(1.0 / 2.2));
    if (uNightVision) {
        float luminance = dot(colour, vec3(0.2126, 0.7152, 0.0722));
        float grain = fract(sin(dot(gl_FragCoord.xy + floor(uTime * 24.0), vec2(12.9898, 78.233))) * 43758.5453);
        float vignette = 1.0 - smoothstep(0.35, 0.75, length(uv - 0.5));
        float light = clamp(luminance * 1.7 + (grain - 0.5) * 0.06, 0.0, 1.0);
        colour = vec3(0.18, 0.8, 0.32) * light * (0.35 + 0.65 * vignette);
    }
    fragColor = vec4(colour, 1.0);
}
