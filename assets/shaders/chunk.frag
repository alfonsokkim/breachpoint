#version 330 core

// The rasterizer stores this output color in the framebuffer.
out vec4 fragColor;

in vec2 texCoord;
in vec3 normal;

uniform sampler2D uTex;

void main() {
    vec4 texColour = texture(uTex, texCoord);
    vec3 lightColour = vec3(1.0, 1.0, 1.0);

    // Upward faces are brightest and downward faces darkest.
    float ambientStrength = 0.2;
    vec3 norm = normalize(normal);
    vec3 lighting = lightColour * ((norm.y * 0.3 + 0.5) + ambientStrength);
    fragColor = vec4(texColour.rgb * lighting, texColour.a);
}
