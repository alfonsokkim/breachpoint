#version 330 core
out vec4 fragColor;
in vec2 texCoord;
in vec3 normal;
in vec3 worldPosition;
in vec3 vertexColour;
uniform sampler2D uTex;
uniform bool uTextured;
uniform bool uVertexColour;
uniform vec3 uTint;
uniform vec3 uEyePosition;
uniform vec3 uMuzzlePosition;
uniform float uMuzzleIntensity;
uniform float uSpecular;
uniform float uShininess;
uniform bool uPbr;
uniform float uRoughness;
uniform float uMetallic;
uniform float uEmissive;
uniform vec3 uSpotPositions[4];
uniform vec3 uSpotDirections[4];
uniform mat4 uShadowMatrices[4];
uniform sampler2DArray uShadowMap;
const float PI = 3.14159265;

vec3 lighting(vec3 base, vec3 n, vec3 v, vec3 l, vec3 radiance) {
    float nl = max(dot(n, l), 0.0);
    if (nl <= 0.0) return vec3(0.0);
    if (!uPbr) {
        float spec = pow(max(dot(v, reflect(-l, n)), 0.0), uShininess) * uSpecular;
        return (base * nl + spec) * radiance;
    }
    vec3 h = (v + l) / max(length(v + l), 0.0001);
    float nv = max(dot(n, v), 0.001);
    float nh = max(dot(n, h), 0.0);
    float vh = max(dot(v, h), 0.0);
    float a = max(uRoughness * uRoughness, 0.0064);
    float a2 = a * a;
    float denominator = nh * nh * (a2 - 1.0) + 1.0;
    float distribution = a2 / max(PI * denominator * denominator, 0.00001);
    float k = (uRoughness + 1.0) * (uRoughness + 1.0) / 8.0;
    float geometry = nv / (nv * (1.0 - k) + k) * nl / (nl * (1.0 - k) + k);
    vec3 f0 = mix(vec3(0.04), base, uMetallic);
    vec3 fresnel = f0 + (1.0 - f0) * pow(1.0 - vh, 5.0);
    vec3 specular = distribution * geometry * fresnel / max(4.0 * nv * nl, 0.001);
    return ((1.0 - fresnel) * (1.0 - uMetallic) * base / PI + specular) * radiance * nl;
}

float visibility(int light, vec3 n, vec3 l) {
    vec4 clip = uShadowMatrices[light] * vec4(worldPosition, 1.0);
    vec3 p = clip.xyz / clip.w * 0.5 + 0.5;
    if (clip.w <= 0.0 || p.z > 1.0 || p.z < 0.0 || p.x < 0.0 || p.x > 1.0 || p.y < 0.0 || p.y > 1.0) return 1.0;
    float bias = max(0.0015 * (1.0 - dot(n, l)), 0.0004);
    float result = 0.0;
    for (int x = -1; x <= 1; ++x)
        for (int y = -1; y <= 1; ++y) {
            float depth = texture(uShadowMap, vec3(p.xy + vec2(x, y) / 1024.0, float(light))).r;
            result += p.z - bias <= depth ? 1.0 : 0.0;
        }
    return result / 9.0;
}

void main() {
    vec3 base = uVertexColour ? vertexColour : uTextured ? texture(uTex, texCoord).rgb : uTint;
    if (uVertexColour) {
        float grain = fract(sin(dot(floor(worldPosition * 35.0), vec3(12.9, 78.2, 45.1))) * 43758.5);
        base *= 0.90 + 0.10 * grain;
        if (normal.y > 0.9) {
            vec2 joint = abs(fract(worldPosition.xz / 4.0) - 0.5);
            base *= max(joint.x, joint.y) > 0.494 ? 0.6 : 1.0;
        } else if (base.r > base.g * 1.3) {
            float row = floor(worldPosition.y * 3.0);
            vec2 brick = fract(vec2((worldPosition.x + worldPosition.z) * 1.3 + mod(row, 2.0) * 0.5,
                                    worldPosition.y * 3.0));
            if (brick.x < 0.035 || brick.y < 0.07) base *= 0.45;
        }
    }
    if (uEmissive > 0.0) { fragColor = vec4(base * uEmissive, 1.0); return; }
    vec3 n = normalize(normal);
    vec3 v = normalize(uEyePosition - worldPosition);
    vec3 colour = base * vec3(0.025, 0.035, 0.06);
    colour += lighting(base, n, v, normalize(vec3(-0.4, 0.8, 0.6)), vec3(0.03, 0.04, 0.08));
    for (int i = 0; i < 4; ++i) {
        vec3 delta = uSpotPositions[i] - worldPosition;
        float distance = length(delta);
        vec3 l = delta / max(distance, 0.001);
        float cone = smoothstep(0.79, 0.93, dot(-l, normalize(uSpotDirections[i])));
        vec3 radiance = vec3(1.0, 0.83, 0.64) * 20.0 * cone /
                        (1.0 + 0.10 * distance + 0.045 * distance * distance);
        colour += lighting(base, n, v, l, radiance) * visibility(i, n, l);
    }
    vec3 delta = uMuzzlePosition - worldPosition;
    float d2 = dot(delta, delta);
    colour += lighting(base, n, v, delta / sqrt(max(d2, 0.0001)),
                       vec3(1.0, 0.55, 0.16) * uMuzzleIntensity * 8.0 / (1.0 + d2 * 2.0));
    float fog = smoothstep(18.0, 48.0, length(uEyePosition - worldPosition)) * 0.38;
    fragColor = vec4(mix(colour, vec3(0.035, 0.045, 0.075), fog), 1.0);
}
