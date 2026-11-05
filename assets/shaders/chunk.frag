#version 330 core

out vec4 fragColor;

in vec2 texCoord;
in vec3 normal;
in vec3 worldPosition;

uniform sampler2D uTex;
uniform bool uTextured;
uniform vec3 uTint;
uniform vec3 uEyePosition;
uniform vec3 uMuzzlePosition;
uniform float uMuzzleIntensity;
uniform float uSpecular;
uniform float uShininess;

void main() {
    vec4 base = uTextured ? texture(uTex, texCoord) : vec4(uTint, 1.0);
    vec3 n = normalize(normal);
    vec3 viewDir = normalize(uEyePosition - worldPosition);
    vec3 sunDir = normalize(vec3(-0.4, 1.0, 0.6));
    float diffuse = max(dot(n, sunDir), 0.0);
    float specular = diffuse > 0.0 ? pow(max(dot(viewDir, reflect(-sunDir, n)), 0.0),
                                       uShininess) * uSpecular : 0.0;
    vec3 colour = base.rgb * (0.42 + 0.50 * diffuse) + vec3(specular * 0.25);
    vec3 offset = uMuzzlePosition - worldPosition;
    float distanceSquared = dot(offset, offset);
    vec3 lightDir = offset / sqrt(max(distanceSquared, 0.0001));
    float muzzleDiffuse = max(dot(n, lightDir), 0.0);
    float muzzleSpecular = muzzleDiffuse > 0.0 ?
        pow(max(dot(viewDir, reflect(-lightDir, n)), 0.0), uShininess) * uSpecular : 0.0;
    vec3 pulse = vec3(1.0, 0.66, 0.25) * uMuzzleIntensity * 4.0 /
                 (1.0 + 2.0 * distanceSquared);
    colour += pulse * (base.rgb * muzzleDiffuse + muzzleSpecular);
    fragColor = vec4(colour, base.a);
}
