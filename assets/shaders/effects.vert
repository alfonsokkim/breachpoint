#version 330 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec4 aColour;
layout(location = 2) in float aSize;
layout(location = 3) in float aFlash;

uniform mat4 uViewProjection;
uniform float uPixelScale;
uniform vec2 uSizeRange;

out vec4 colour;
flat out float flash;

void main() {
    gl_Position = uViewProjection * vec4(aPosition, 1.0);
    gl_PointSize = clamp(aSize * uPixelScale / max(gl_Position.w, 0.01),
                        uSizeRange.x, uSizeRange.y);
    colour = aColour;
    flash = aFlash;
}
