#version 330 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in vec3 aNormal;
layout(location = 3) in vec3 aColour;

uniform mat4 uModel;
uniform mat4 uViewProjection;

out vec2 texCoord;
out vec3 normal;
out vec3 worldPosition;
out vec3 vertexColour;

void main() {
    vec4 world = uModel * vec4(aPosition, 1.0);
    gl_Position = uViewProjection * world;
    texCoord = aTexCoord;
    normal = mat3(transpose(inverse(uModel))) * aNormal;
    worldPosition = world.xyz;
    vertexColour = aColour;
}
